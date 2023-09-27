using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

internal sealed class GmodVersioning
{
    private readonly Dictionary<VisVersion, GmodVersioningNode> _versioningsMap = new();

    internal GmodVersioning(GmodVersioningDto dto)
    {
        foreach (var versioningDto in dto.Items)
        {
            var visVersion = versioningDto.Key;
            var gmodVersioningNode = new GmodVersioningNode(versioningDto.Value);
            _versioningsMap.Add(VisVersions.Parse(visVersion), gmodVersioningNode);
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

    public GmodNode? ConvertNodeInternal(VisVersion sourceVersion, GmodNode sourceNode, VisVersion targetVersion)
    {
        var nextCode = sourceNode.Code;

        if (TryGetVersioningNode(sourceVersion, out var versioningNode))
        {
            if (versioningNode.TryGetCodeChanges(sourceNode.Code, out var change))
            {
                if (targetVersion == change.NextVisVersion)
                    nextCode = change.NextCode ?? throw new InvalidOperationException("failed to set next code");
                else if (targetVersion == change.PreviousVisVersion)
                    nextCode =
                        change.PreviousCode ?? throw new InvalidOperationException("failed to set previous code");
            }
        }

        var targetGmod = VIS.Instance.GetGmod(targetVersion);

        if (!targetGmod.TryGetNode(nextCode, out var targetNode))
            return null;
        return targetNode.TryWithLocation(sourceNode.Location);
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
        for (int i = 0; i <= qualifyingNodes.Length - 1; i++)
        {
            var qualifyingNode = qualifyingNodes[i];

            if (i > 0 && qualifyingNode.TargetNode.Code == qualifyingNodes[i - 1].TargetNode.Code)
                continue;

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
                                path.AddRange(remaining);
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
                    i++;
                }
                else if (qualifyingNode.TargetNode.Code != targetEndNode.Code)
                {
                    AddToPath(targetGmod, path, targetNormalAssignment!);
                    i++; // Holy moly
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

        if (!GmodPath.IsValid(potentialParents, targetEndNode, out var missinkLinkAt))
            throw new Exception($"Didnt end up with valid path for {sourcePath}");

        return new GmodPath(potentialParents, targetEndNode);
    }

    private bool HasChangesInNextVisVersion(VisVersion source)
    {
        var target = source + 1;

        return TryGetVersioningNode(target, out _);
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
    }

    private readonly record struct GmodVersioningNode
    {
        private readonly Dictionary<string, GmodVersioningNodeChanges> _versioningNodeChanges = new();

        internal GmodVersioningNode(IReadOnlyDictionary<string, GmodVersioningNodeChangesDto> dto)
        {
            foreach (var versioningNodeDto in dto)
            {
                if (
                    versioningNodeDto.Value is null
                    || (versioningNodeDto.Value.PreviousCode is null && versioningNodeDto.Value.NextCode is null)
                )
                    continue;

                var code = versioningNodeDto.Key;
                var versioningNodeChanges = new GmodVersioningNodeChanges(
                    VisVersions.TryParse(versioningNodeDto.Value.NextVisVersion, out var nextVisVersion)
                        ? nextVisVersion
                        : null,
                    versioningNodeDto.Value.NextCode,
                    VisVersions.TryParse(versioningNodeDto.Value.PreviousVisVersion, out var previoiusVisVersion)
                        ? previoiusVisVersion
                        : null,
                    versioningNodeDto.Value.PreviousCode
                );
                _versioningNodeChanges.Add(code, versioningNodeChanges);
            }
        }

        public bool TryGetCodeChanges(string code, [MaybeNullWhen(false)] out GmodVersioningNodeChanges nodeChanges) =>
            _versioningNodeChanges.TryGetValue(code, out nodeChanges);
    }

    private sealed record GmodVersioningNodeChanges(
        VisVersion? NextVisVersion,
        string? NextCode,
        VisVersion? PreviousVisVersion,
        string? PreviousCode
    );
}
