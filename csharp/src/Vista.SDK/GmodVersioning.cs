using System.Diagnostics;
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

    public GmodNode? ConvertNode(
        VisVersion sourceVersion,
        GmodNode sourceNode,
        VisVersion targetVersion
    )
    {
        ValidateSourceAndTargetVersions(sourceVersion, targetVersion);

        if (!TryGetVersioningNode(sourceVersion, out var versioningNode))
            throw new ArgumentException(
                "Couldn't get versioning node with VIS version" + sourceVersion.ToVersionString()
            );

        var nextCode = sourceNode.Code;
        if (versioningNode.TryGetCodeChanges(sourceNode.Code, out var change))
        {
            if (targetVersion == change.NextVisVersion)
                nextCode = change.NextCode!;
            else if (targetVersion == change.PreviousVisVersion)
                nextCode = change.PreviousCode!;
            else
                throw new Exception("Invalid conversion");
        }

        var gmod = VIS.Instance.GetGmod(targetVersion);

        if (!gmod.TryGetNode(nextCode, out var targetNode))
            return null;

        return targetNode with
        {
            Location = sourceNode.Location
        };
    }

    public GmodPath? ConvertPath(
        VisVersion sourceVersion,
        GmodPath sourcePath,
        VisVersion targetVersion
    )
    {
        var targetEndNode = ConvertNode(sourceVersion, sourcePath.Node, targetVersion);
        if (targetEndNode is null)
            return null;

        if (targetEndNode.IsRoot)
            return new GmodPath(targetEndNode.Parents, targetEndNode);

        var targetGmod = VIS.Instance.GetGmod(targetVersion);

        var qualifyingNodes = sourcePath
            .GetFullPath()
            .Select(
                (t, i) =>
                    (
                        SourceNode: t.Node,
                        TargetNode: ConvertNode(sourceVersion, t.Node, targetVersion)!
                    )
            )
            .ToArray();
        if (qualifyingNodes.Any(t => t.TargetNode is null))
            throw new Exception("Could convert node forward");

        var potentialParents = qualifyingNodes
            .Select(n => n.TargetNode)
            .Take(qualifyingNodes.Length - 1)
            .ToArray();
        if (GmodPath.IsValid(potentialParents, targetEndNode))
            return new GmodPath(potentialParents, targetEndNode);

        var qualifyingNodesWithCorrectPath = new List<GmodNode>();
        for (int i = 0; i <= qualifyingNodes.Length - 1; i++)
        {
            var qualifyingNode = qualifyingNodes[i];
            //qualifyingNodesWithCorrectPath.Add(qualifyingNode);
            //if (
            //    !targetGmod.PathExistsBetween(
            //        qualifyingNodesWithCorrectPath.Select(n => n.TargetNode),
            //        targetEndNode
            //    )
            //)
            //    qualifyingNodesWithCorrectPath.RemoveAt(qualifyingNodesWithCorrectPath.Count - 1);

            var codeChanged = qualifyingNode.SourceNode.Code != qualifyingNode.TargetNode.Code;
            var normalAssignmentChanged =
                qualifyingNode.SourceNode.ProductType?.Code
                != qualifyingNode.TargetNode.ProductType?.Code;
            var selectionChanged = false;

            if (codeChanged)
            {
                for (int j = qualifyingNodesWithCorrectPath.Count - 1; j >= 0; j--)
                {
                    var parent = qualifyingNodesWithCorrectPath[j];
                    if (
                        !targetGmod.PathExistsBetween(
                            qualifyingNodesWithCorrectPath.Take(j + 1),
                            qualifyingNode.TargetNode,
                            out var remaining
                        )
                    )
                    {
                        qualifyingNodesWithCorrectPath.RemoveAt(j);
                    }
                    else
                    {
                        qualifyingNodesWithCorrectPath.AddRange(remaining);
                        qualifyingNodesWithCorrectPath.Add(qualifyingNode.TargetNode);
                        break;
                    }
                }
            }
            else
            {
                //if (qualifyingNodesWithCorrectPath.Count > 0)
                //{
                //    var pathExists = targetGmod.PathExistsBetween(
                //        qualifyingNodesWithCorrectPath.Select(n => n.TargetNode),
                //        qualifyingNode.TargetNode,
                //        out var remaining
                //    );
                //    if (pathExists && remaining.Count() > 0)
                //    {
                //        qualifyingNodesWithCorrectPath.AddRange(
                //            remaining.Select(
                //                n =>
                //                    (
                //                        SourceNode: ConvertNode(targetVersion, n, sourceVersion),
                //                        TargetNode: n
                //                    )
                //            )
                //        );
                //    }
                //    else if (!pathExists)
                //    {
                //        var baseNode = qualifyingNodesWithCorrectPath[
                //            qualifyingNodesWithCorrectPath.Count - 1
                //        ];
                //        if (
                //            baseNode.SourceNode?.ProductType?.Code == sourcePath.Node.Code
                //            && baseNode.TargetNode.ProductType is not null
                //            && GmodPath.IsValid(potentialParents, baseNode.TargetNode.ProductType)
                //        )
                //        {
                //            return new GmodPath(potentialParents, baseNode.TargetNode.ProductType);
                //        }
                //    }
                //}
                qualifyingNodesWithCorrectPath.Add(qualifyingNode.TargetNode);
            }

            if (normalAssignmentChanged) // AC || AN || AD
            {
                var wasCreated =
                    qualifyingNode.SourceNode.ProductType is null
                    && qualifyingNode.TargetNode.ProductType is not null;
                var wasDeleted =
                    qualifyingNode.SourceNode.ProductType is not null
                    && qualifyingNode.TargetNode.ProductType is null;
                if (wasCreated)
                {
                    qualifyingNodesWithCorrectPath.Add(qualifyingNode.TargetNode.ProductType!);
                }
                else if (wasDeleted)
                {
                    i++;
                }
                else
                {
                    //qualifyingNodesWithCorrectPath.Add(qualifyingNode.TargetNode.ProductType!);
                    //i++;

                    qualifyingNodes[i + 1] = (
                        qualifyingNode.SourceNode.ProductType!,
                        qualifyingNode.TargetNode.ProductType!
                    );
                }
            }
            if (selectionChanged) // SC || SN || SD
            { }

            if (i == qualifyingNodes.Length - 1) { }
        }

        potentialParents = qualifyingNodesWithCorrectPath
            .Take(qualifyingNodesWithCorrectPath.Count - 1)
            .ToArray();
        targetEndNode = qualifyingNodesWithCorrectPath.Last();

        Debug.Assert(
            GmodPath.IsValid(potentialParents, targetEndNode, out var missinkLinkAt),
            "Should be correct"
        );
        return new GmodPath(potentialParents, targetEndNode);

        //var baseNode = qualifyingNodesWithCorrectPath[missinkLinkAt];
        //var reachedEnd = targetGmod.Traverse(
        //    baseNode.TargetNode,
        //    (parents, node) =>
        //    {
        //        if (node.Code != targetEndNode.Code)
        //            return TraversalHandlerResult.Continue;

        //        var insertAt = missinkLinkAt + 1;
        //        for (int i = 1; i < parents.Count; i++)
        //        {
        //            var parent = parents[i];
        //            var missingNode = (ConvertNode(targetVersion, parent, sourceVersion), parent);
        //            qualifyingNodesWithCorrectPath.Insert(insertAt++, missingNode);
        //        }
        //        return TraversalHandlerResult.Stop;
        //    }
        //);

        //if (reachedEnd)
        //{
        //    if (
        //        baseNode.SourceNode?.ProductType?.Code == sourcePath.Node.Code
        //        && baseNode.TargetNode.ProductType is not null
        //        && GmodPath.IsValid(potentialParents, baseNode.TargetNode.ProductType)
        //    )
        //    {
        //        return new GmodPath(potentialParents, baseNode.TargetNode.ProductType);
        //    }
        //}

        //Debug.Assert(!reachedEnd, "Should find the path and stop traversing");
        //potentialParents = qualifyingNodesWithCorrectPath
        //    .Take(qualifyingNodesWithCorrectPath.Count - 1)
        //    .ToArray();
        //Debug.Assert(
        //    GmodPath.IsValid(potentialParents, targetEndNode, out missinkLinkAt),
        //    "Should have a valid path now"
        //);
        //return new GmodPath(potentialParents, targetEndNode);

        //var locations = qualifyingNodesWithCorrectPath
        //    .Select(kvp => (Code: kvp.TargetNode.Code, Location: kvp.TargetNode.Location))
        //    .GroupBy(t => t.Code)
        //    .Select(grp => grp.First())
        //    .ToDictionary(kvp => kvp.Code, kvp => kvp.Location);

        //var targetBaseNode =
        //    qualifyingNodesWithCorrectPath.LastOrDefault(
        //        n => n.TargetNode.IsAssetFunctionNode && n.TargetNode.Code != targetEndNode.Code
        //    ).TargetNode
        //    ?? qualifyingNodesWithCorrectPath.LastOrDefault().TargetNode
        //    ?? targetGmod.RootNode;

        //var possiblePaths = new List<GmodPath>();
        //targetGmod.Traverse(
        //    possiblePaths,
        //    rootNode: targetBaseNode,
        //    handler: (possiblePaths, parents, node) =>
        //    {
        //        if (node.Code != targetEndNode.Code)
        //            return TraversalHandlerResult.Continue;

        //        var targetParents = new List<GmodNode>(parents.Count);

        //        targetParents.AddRange(
        //            parents
        //                .Where(p => p.Code != targetBaseNode.Code)
        //                .Select(
        //                    p =>
        //                        p with
        //                        {
        //                            Location = locations.TryGetValue(p.Code, out var location)
        //                              ? location
        //                              : null
        //                        }
        //                )
        //                .ToList()
        //        );

        //        var currentTargetBaseNode = targetBaseNode;
        //        Debug.Assert(
        //            currentTargetBaseNode.Parents.Count == 1,
        //            $"More than one path to root found for: {sourcePath}"
        //        );
        //        while (currentTargetBaseNode.Parents.Count == 1)
        //        {
        //            // Traversing upwards to get to VE, since we until now have traversed from first leaf node.
        //            targetParents.Insert(0, currentTargetBaseNode);
        //            currentTargetBaseNode = currentTargetBaseNode.Parents[0];
        //        }

        //        targetParents.Insert(0, targetGmod.RootNode);

        //        var qualifiedParents = qualifyingNodesWithCorrectPath
        //            .Where(n => !targetParents.Any(t => t.Code == n.TargetNode.Code))
        //            .ToList();

        //        if (
        //            !qualifyingNodesWithCorrectPath
        //                .Take(qualifyingNodesWithCorrectPath.Count - 1)
        //                .All(cn => targetParents.Any(p => p.Code == cn.TargetNode.Code))
        //        )
        //            return TraversalHandlerResult.Continue;

        //        possiblePaths.Add(new GmodPath(targetParents, node));
        //        return TraversalHandlerResult.Continue;
        //    }
        //);

        //Debug.Assert(
        //    possiblePaths.Count == 1,
        //    $"Expected exactly one possible target path for: {sourcePath}. Got: {(possiblePaths.Count == 0 ? "0" : string.Join("\n", possiblePaths))}"
        //);
        //return possiblePaths[0];
    }

    private bool TryGetVersioningNode(
        VisVersion visVersion,
        [MaybeNullWhen(false)] out GmodVersioningNode versioningNode
    ) => _versioningsMap.TryGetValue(visVersion, out versioningNode);

    private void ValidateSourceAndTargetVersions(VisVersion sourceVersion, VisVersion targetVersion)
    {
        if (string.IsNullOrWhiteSpace(sourceVersion.ToVersionString()))
            throw new ArgumentException(
                "Invalid source VIS Version: " + sourceVersion.ToVersionString()
            );

        if (string.IsNullOrWhiteSpace(targetVersion.ToVersionString()))
            throw new ArgumentException(
                "Invalid target VISVersion: " + targetVersion.ToVersionString()
            );

        if (!_versioningsMap.ContainsKey(sourceVersion))
            throw new ArgumentException(
                "Source VIS Version does not exist in versionings: "
                    + sourceVersion.ToVersionString()
            );

        if (!_versioningsMap.ContainsKey(targetVersion))
            throw new ArgumentException(
                "Target VIS Version does not exist in versionings: "
                    + targetVersion.ToVersionString()
            );
    }

    private readonly record struct GmodVersioningNode
    {
        private readonly Dictionary<string, GmodVersioningNodeChanges> _versioningNodeChanges =
            new();

        internal GmodVersioningNode(IReadOnlyDictionary<string, GmodVersioningNodeChangesDto> dto)
        {
            foreach (var versioningNodeDto in dto)
            {
                if (
                    versioningNodeDto.Value is null
                    || (
                        versioningNodeDto.Value.PreviousCode is null
                        && versioningNodeDto.Value.NextCode is null
                    )
                )
                    continue;

                var code = versioningNodeDto.Key;
                var versioningNodeChanges = new GmodVersioningNodeChanges(
                    VisVersions.TryParse(
                        versioningNodeDto.Value.NextVisVersion,
                        out var nextVisVersion
                    )
                      ? nextVisVersion
                      : null,
                    versioningNodeDto.Value.NextCode,
                    VisVersions.TryParse(
                        versioningNodeDto.Value.PreviousVisVersion,
                        out var previoiusVisVersion
                    )
                      ? previoiusVisVersion
                      : null,
                    versioningNodeDto.Value.PreviousCode
                );
                _versioningNodeChanges.Add(code, versioningNodeChanges);
            }
        }

        public bool TryGetCodeChanges(
            string code,
            [MaybeNullWhen(false)] out GmodVersioningNodeChanges nodeChanges
        ) => _versioningNodeChanges.TryGetValue(code, out nodeChanges);
    }

    private sealed record GmodVersioningNodeChanges(
        VisVersion? NextVisVersion,
        string? NextCode,
        VisVersion? PreviousVisVersion,
        string? PreviousCode
    );
}
