using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

internal sealed class GmodVersioning
{
    private readonly Dictionary<VisVersion, GmodVersioningNode> _versioningsMap = new();

    internal GmodVersioning(Dictionary<string, GmodVersioningDto> dto)
    {
        foreach (var kvp in dto)
        {
            var parsedVersion = VisVersions.Parse(kvp.Key);
            var gmodVersioningNode = new GmodVersioningNode(parsedVersion, kvp.Value.Items);
            _versioningsMap.Add(parsedVersion, gmodVersioningNode);
        }
    }

    public GmodNode? ConvertNode(VisVersion sourceVersion, GmodNode sourceNode, VisVersion targetVersion)
    {
        ValidateSourceAndTargetVersions(sourceVersion, targetVersion);

        GmodNode? node = sourceNode;
        var source = sourceVersion;

        while (source <= targetVersion - 1)
        {
            if (node is null)
                break;

            var target = source + 1;

            node = ConvertNodeInternal(source, node, target);
            source++;
        }

        return node;
    }

    private GmodNode? ConvertNodeInternal(VisVersion sourceVersion, GmodNode sourceNode, VisVersion targetVersion)
    {
        ValidateSourceAndTargetVersionPair(sourceNode.VisVersion, targetVersion);
        var nextCode = sourceNode.Code;

        if (TryGetVersioningNode(targetVersion, out var versioningNode))
        {
            // Naive approach as we dont have any context of the path
            if (versioningNode.TryGetCodeChanges(sourceNode.Code, out var change) && change.Target is not null)
            {
                nextCode = change.Target;
            }
        }

        var targetGmod = VIS.Instance.GetGmod(targetVersion);

        if (!targetGmod.TryGetNode(nextCode, out var targetNode))
            return null;
        var result = targetNode.TryWithLocation(sourceNode.Location);
        if (sourceNode.Location is not null && result.Location != sourceNode.Location)
            throw new Exception("Failed to set location");
        return result;
    }

    public LocalIdBuilder? ConvertLocalId(LocalIdBuilder sourceLocalId, VisVersion targetVersion)
    {
        if (sourceLocalId.VisVersion is null)
            throw new InvalidOperationException("Cant convert local ID without a specific VIS version");

        var targetLocalId = LocalIdBuilder.Create(targetVersion);

        if (sourceLocalId.PrimaryItem is not null)
        {
            var targetPrimaryitem = ConvertPath(
                sourceLocalId.VisVersion.Value,
                sourceLocalId.PrimaryItem,
                targetVersion
            );
            if (targetPrimaryitem is null)
                return null;
            targetLocalId = targetLocalId.WithPrimaryItem(targetPrimaryitem);
        }
        if (sourceLocalId.SecondaryItem is not null)
        {
            var targetSecondaryitem = ConvertPath(
                sourceLocalId.VisVersion.Value,
                sourceLocalId.SecondaryItem,
                targetVersion
            );
            if (targetSecondaryitem is null)
                return null;
            targetLocalId = targetLocalId.WithSecondaryItem(targetSecondaryitem);
        }

        return targetLocalId
            .WithVerboseMode(sourceLocalId.VerboseMode)
            .TryWithMetadataTag(sourceLocalId.Quantity)
            .TryWithMetadataTag(sourceLocalId.Content)
            .TryWithMetadataTag(sourceLocalId.Calculation)
            .TryWithMetadataTag(sourceLocalId.State)
            .TryWithMetadataTag(sourceLocalId.Command)
            .TryWithMetadataTag(sourceLocalId.Type)
            .TryWithMetadataTag(sourceLocalId.Position)
            .TryWithMetadataTag(sourceLocalId.Detail);
    }

    public LocalId? ConvertLocalId(LocalId sourceLocalId, VisVersion targetVersion) =>
        ConvertLocalId(sourceLocalId.Builder, targetVersion)?.Build();

    public GmodPath? ConvertPath(VisVersion sourceVersion, GmodPath sourcePath, VisVersion targetVersion)
    {
        var targetEndNode = ConvertNode(sourceVersion, sourcePath.Node, targetVersion);
        if (targetEndNode is null)
            return null;

        if (targetEndNode.IsRoot)
            return new GmodPath(targetEndNode._parents, targetEndNode, skipVerify: true);

        var targetGmod = VIS.Instance.GetGmod(targetVersion);
        var sourceGmod = VIS.Instance.GetGmod(sourceVersion);

        var qualifyingNodes = sourcePath
            .GetFullPath()
            .Select((t, i) => (SourceNode: t.Node, TargetNode: ConvertNode(sourceVersion, t.Node, targetVersion)!))
            .ToArray();
        if (qualifyingNodes.Any(t => t.TargetNode is null))
            throw new Exception("Could convert node forward");

        var potentialParents = qualifyingNodes.Select(n => n.TargetNode).Take(qualifyingNodes.Length - 1).ToList();
        if (GmodPath.IsValid(potentialParents, targetEndNode))
            return new GmodPath(potentialParents, targetEndNode, skipVerify: true);

        var path = new List<GmodNode>();
        for (int i = 0; i < qualifyingNodes.Length; i++)
        {
            var qualifyingNode = qualifyingNodes[i];
            // Continue if next qualifying node is the same as the previous
            if (i > 0 && qualifyingNode.TargetNode.Code == qualifyingNodes[i - 1].TargetNode.Code)
            {
                /*
                    (Mes) The same as the current qualifying node, assumes same normal assignment [IF NOT THROW EXCEPTION, uncovered case]
                */
                if (qualifyingNode.TargetNode.Code == qualifyingNodes[i - 1].TargetNode.Code)
                {
                    if (
                        qualifyingNode.SourceNode.ProductType is not null
                        && qualifyingNode.SourceNode.ProductType != qualifyingNodes[i - 1].TargetNode.ProductType
                    )
                        throw new InvalidOperationException(
                            $"Failed to convert path at node {qualifyingNode.TargetNode}. Uncovered case of merge where target node also has a new assignment"
                        );
                }
                /*
                    Check if skipped node is individualized
                */
                if (qualifyingNode.TargetNode.Location is not null)
                {
                    // Find node in path
                    var index = path.FindIndex(n => n.Code == qualifyingNode.TargetNode.Code);
                    if (index != -1)
                    {
                        if (
                            path[index].Location is not null
                            && path[index].Location != qualifyingNode.TargetNode.Location
                        )
                            throw new InvalidOperationException(
                                $"Failed to convert path at node {qualifyingNode.TargetNode}. Uncovered case of multiple colliding locations while converting nodes"
                            );
                        if (!path[index].IsIndividualizable(false, false))
                        {
                            throw new InvalidOperationException(
                                $"Failed to convert path at node {path[index]}. Uncovered case of losing individualization information"
                            );
                        }

                        // Dont overwrite existing location
                        if (path[index].Location is null)
                            path[index] = path[index].WithLocation(qualifyingNode.TargetNode.Location);
                    }
                }
                continue;
            }

            var codeChanged = qualifyingNode.SourceNode.Code != qualifyingNode.TargetNode.Code;

            var sourceNormalAssignment = qualifyingNode.SourceNode.ProductType;
            var targetNormalAssignment = qualifyingNode.TargetNode.ProductType;

            var normalAssignmentChanged = sourceNormalAssignment?.Code != targetNormalAssignment?.Code;

            var selectionChanged = false;

            static void AddToPath(Gmod targetGmod, List<GmodNode> path, GmodNode node)
            {
                if (path.Count > 0)
                {
                    var prev = path[path.Count - 1];
                    if (!prev.IsChild(node))
                    {
                        for (int j = path.Count - 1; j >= 0; j--)
                        {
                            var parent = path[j];
                            var currentParents = path.Take(j + 1).ToArray();
                            if (!targetGmod.PathExistsBetween(currentParents, node, out var remaining))
                            {
                                if (!currentParents.Any(n => n.IsAssetFunctionNode && n.Code != parent.Code))
                                    throw new Exception("Tried to remove last asset function node");
                                path.RemoveAt(j);
                            }
                            else
                            {
                                var nodes = new List<GmodNode>();
                                if (node.Location is not null)
                                {
                                    foreach (var n in remaining)
                                    {
                                        if (!n.IsIndividualizable(false, true))
                                        {
                                            nodes.Add(n);
                                        }
                                        else
                                        {
                                            nodes.Add(n.WithLocation(node.Location));
                                        }
                                    }
                                }
                                else
                                {
                                    nodes.AddRange(remaining);
                                }
                                path.AddRange(nodes);
                                break;
                            }
                        }
                    }
                }

                path.Add(node);
            }

            if (codeChanged)
            {
                AddToPath(targetGmod, path, qualifyingNode.TargetNode);
            }
            else if (normalAssignmentChanged) // AC || AN || AD
            {
                var wasDeleted = sourceNormalAssignment is not null && targetNormalAssignment is null;

                if (!codeChanged)
                    AddToPath(targetGmod, path, qualifyingNode.TargetNode);

                if (wasDeleted)
                {
                    if (qualifyingNode.TargetNode.Code == targetEndNode.Code)
                    {
                        var next = qualifyingNodes[i + 1];
                        if (next.TargetNode.Code != qualifyingNode.TargetNode.Code)
                            throw new Exception("Normal assignment end node was deleted");
                    }
                    continue;
                }
                else if (qualifyingNode.TargetNode.Code != targetEndNode.Code)
                {
                    AddToPath(targetGmod, path, targetNormalAssignment!);
                    /*
                        - (AC) The previous normal assignment
                    */

                    if (
                        sourceNormalAssignment is not null
                        && targetNormalAssignment is not null
                        && sourceNormalAssignment.Code != targetNormalAssignment.Code
                    )
                    {
                        // Sanity check that the next node is actually the old assignment
                        if (
                            i + 1 < qualifyingNodes.Length
                            && qualifyingNodes[i + 1].SourceNode.Code != sourceNormalAssignment.Code
                        )
                            throw new InvalidOperationException(
                                $"Failed to convert path at node {qualifyingNode.TargetNode}. Expected next qualifying source node to match target normal assignment"
                            );

                        // Skip next node, since that is the old assignment
                        i++;
                    }
                }
            }
            if (selectionChanged) // SC || SN || SD
            { }

            if (!codeChanged && !normalAssignmentChanged)
            {
                AddToPath(targetGmod, path, qualifyingNode.TargetNode);
            }

            if (path[path.Count - 1].Code == targetEndNode.Code)
                break;
        }

        potentialParents = path.Take(path.Count - 1).ToList();
        targetEndNode = path.Last();

        // Fix individualization
        var visitor = new GmodPath.LocationSetsVisitor();
        for (var i = 0; i < potentialParents.Count + 1; i++)
        {
            var n = i < potentialParents.Count ? potentialParents[i] : targetEndNode;
            var set = visitor.Visit(n, i, potentialParents, targetEndNode);
            if (set is null)
            {
                if (n.Location is not null)
                    break;
                continue;
            }

            var (start, end, location) = set.Value;
            if (start == end)
                continue;

            for (int j = start; j <= end; j++)
            {
                if (j < potentialParents.Count)
                    potentialParents[j] = potentialParents[j] with { Location = location };
                else
                    targetEndNode = targetEndNode with { Location = location };
            }
        }

        if (!GmodPath.IsValid(potentialParents, targetEndNode, out var missingLinkAt))
            throw new Exception($"Didnt end up with valid path for {sourcePath}");

        return new GmodPath(potentialParents, targetEndNode);
    }

    private bool TryGetVersioningNode(
        VisVersion visVersion,
        [MaybeNullWhen(false)] out GmodVersioningNode versioningNode
    ) => _versioningsMap.TryGetValue(visVersion, out versioningNode);

    private void ValidateSourceAndTargetVersions(VisVersion sourceVersion, VisVersion targetVersion)
    {
        if (string.IsNullOrWhiteSpace(sourceVersion.ToVersionString()))
            throw new ArgumentException("Invalid source VIS Version: " + sourceVersion.ToVersionString());

        if (string.IsNullOrWhiteSpace(targetVersion.ToVersionString()))
            throw new ArgumentException("Invalid target VISVersion: " + targetVersion.ToVersionString());
        if (sourceVersion >= targetVersion)
            throw new ArgumentException("Source version must be less than target version");
    }

    private void ValidateSourceAndTargetVersionPair(VisVersion sourceVersion, VisVersion targetVersion)
    {
        if (sourceVersion >= targetVersion)
            throw new ArgumentException("Source version must be less than target version");
        if (targetVersion - sourceVersion != 1)
            throw new ArgumentException("Target version must be exactly one version higher than source version");
    }

    private readonly record struct GmodVersioningNode
    {
        internal VisVersion VisVersion { get; }
        private readonly Dictionary<string, GmodNodeConversion> _versioningNodeChanges = new();

        internal GmodVersioningNode(VisVersion visVersion, IReadOnlyDictionary<string, GmodNodeConversionDto> dto)
        {
            VisVersion = visVersion;
            foreach (var versioningNodeDto in dto)
            {
                var code = versioningNodeDto.Key;
                var versioningNodeChanges = new GmodNodeConversion
                {
                    Operations = new(versioningNodeDto.Value.Operations.Select(ParseConversionType)),
                    Source = versioningNodeDto.Value.Source,
                    Target = versioningNodeDto.Value.Target,
                    OldAssignment = versioningNodeDto.Value.OldAssignment,
                    NewAssignment = versioningNodeDto.Value.NewAssignment,
                    DeleteAssignment = versioningNodeDto.Value.DeleteAssignment,
                };
                _versioningNodeChanges.Add(code, versioningNodeChanges);
            }
        }

        public bool TryGetCodeChanges(string code, [MaybeNullWhen(false)] out GmodNodeConversion nodeChanges) =>
            _versioningNodeChanges.TryGetValue(code, out nodeChanges);
    }

    private sealed record GmodNodeConversion
    {
        public required HashSet<ConversionType> Operations { get; init; }
        public required string Source { get; init; }
        public string? Target { get; init; }
        public string? OldAssignment { get; set; }
        public string? NewAssignment { get; set; }
        public bool? DeleteAssignment { get; set; }
    }

    private enum ConversionType
    {
        ChangeCode,
        Merge,
        Move,
        AssignmentChange = 20,
        AssignmentDelete = 21
    }

    private static ConversionType ParseConversionType(string type) =>
        type switch
        {
            "changeCode" => ConversionType.ChangeCode,
            "merge" => ConversionType.Merge,
            "move" => ConversionType.Move,
            "assignmentChange" => ConversionType.AssignmentChange,
            "assignmentDelete" => ConversionType.AssignmentDelete,
            _ => throw new ArgumentException("Invalid conversion type: " + type)
        };
}
