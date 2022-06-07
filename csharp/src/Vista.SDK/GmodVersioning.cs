using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK;

public sealed class GmodVersioning
{
    private readonly Dictionary<string, GmodVersioningNode> _versioningsMap = new();
    private readonly Func<VisVersion, Gmod> _gmod;

    internal GmodVersioning(GmodVersioningDto dto, Func<VisVersion, Gmod> gmod)
    {
        foreach (var versioningDto in dto.Items)
        {
            var visVersion = versioningDto.Key;
            var gmodVersioningNode = new GmodVersioningNode(versioningDto.Value);
            _versioningsMap.Add(visVersion, gmodVersioningNode);
        }
        _gmod = gmod;
    }

    public GmodNode ConvertNode(
        VisVersion sourceVersion,
        GmodNode sourceNode,
        VisVersion targetVersion
    )
    {
        ValidateSourceAndTargetVersions(sourceVersion, targetVersion);

        if (!TryGetVersioningNode(sourceVersion.ToVersionString(), out var versioningNode))
            throw new ArgumentException(
                "Couldn't get versioning node with VIS version" + sourceVersion.ToVersionString()
            );

        var nextCode = versioningNode.TryGetCodeChanges(sourceNode.Code, out var nodeChanges)
          ? nodeChanges.NextCode
          : sourceNode.Code;

        var gmod = _gmod(targetVersion);

        if (!gmod.TryGetNode(nextCode, out var targetNode))
            throw new ArgumentException("Couldn't get target node with code: " + nextCode);
        return targetNode with { Location = sourceNode.Location };
    }

    public GmodPath ConvertPath(
        VisVersion sourceVersion,
        GmodPath sourcePath,
        VisVersion targetVersion
    )
    {
        var targetEndNode = ConvertNode(sourceVersion, sourcePath.Node, targetVersion);
        if (targetEndNode.IsRoot)
            return new GmodPath(targetEndNode.Parents, targetEndNode);

        var targetGmod = _gmod(targetVersion);

        var qualifyingNodes = sourcePath
            .GetFullPath()
            .Select(
                t =>
                    (
                        SourceNode: t.Node,
                        TargetNode: ConvertNode(sourceVersion, t.Node, targetVersion)
                    )
            )
            .Where(t => t.TargetNode.Code != targetEndNode.Code)
            .ToArray();

        var qualifyingNodesWithCorrectPath = new List<(GmodNode SourceNode, GmodNode TargetNode)>();
        for (int i = 0; i <= qualifyingNodes.Length - 1; i++)
        {
            var qualifyingNode = qualifyingNodes[i];
            qualifyingNodesWithCorrectPath.Add(qualifyingNode);
            if (
                !targetGmod.PathExistsBetween(
                    qualifyingNodesWithCorrectPath.Select(n => n.TargetNode),
                    targetEndNode
                )
            )
                qualifyingNodesWithCorrectPath.RemoveAt(qualifyingNodesWithCorrectPath.Count - 1);
        }

        var locations = qualifyingNodesWithCorrectPath
            .Select(kvp => (Code: kvp.TargetNode.Code, Location: kvp.TargetNode.Location))
            .GroupBy(t => t.Code)
            .Select(grp => grp.First())
            .ToDictionary(kvp => kvp.Code, kvp => kvp.Location);

        var targetBaseNode =
            qualifyingNodesWithCorrectPath.LastOrDefault(
                n => n.TargetNode.IsAssetFunctionNode && !n.TargetNode.IsProductGroupLevel
            ).TargetNode ?? targetGmod.RootNode;

        var possiblePaths = new List<GmodPath>();
        targetGmod.Traverse(
            possiblePaths,
            rootNode: targetBaseNode,
            handler: (possiblePaths, parents, node) =>
            {
                if (node.Code != targetEndNode.Code)
                    return TraversalHandlerResult.Continue;

                var targetParents = new List<GmodNode>(parents.Count);

                targetParents.AddRange(
                    parents
                        .Where(p => p.Code != targetBaseNode.Code && !p.IsProductGroupLevel)
                        .Select(
                            p =>
                                p with
                                {
                                    Location = locations.TryGetValue(p.Code, out var location)
                                      ? location
                                      : null
                                }
                        )
                        .ToList()
                );

                var currentTargetBaseNode = targetBaseNode;
                Debug.Assert(
                    currentTargetBaseNode.Parents.Count == 1,
                    $"More than one path to root found for: {sourcePath}"
                );
                while (currentTargetBaseNode.Parents.Count == 1)
                {
                    // Traversing upwards to get to VE, since we until now have traversed from first leaf node.
                    targetParents.Insert(0, currentTargetBaseNode);
                    currentTargetBaseNode = currentTargetBaseNode.Parents[0];
                }

                targetParents.Insert(0, targetGmod.RootNode);

                var qualifiedParents = qualifyingNodesWithCorrectPath
                    .Where(n => !targetParents.Any(t => t.Code == n.TargetNode.Code))
                    .ToList();

                if (
                    !qualifyingNodesWithCorrectPath.All(
                        cn => targetParents.Any(p => p.Code == cn.TargetNode.Code)
                    )
                )
                    return TraversalHandlerResult.Continue;

                possiblePaths.Add(new GmodPath(targetParents, node));
                return TraversalHandlerResult.Continue;
            }
        );

        Debug.Assert(possiblePaths.Count == 1, $"More than one path found for: {sourcePath}");
        return possiblePaths[0];
    }

    public GmodVersioningNode this[string key] => _versioningsMap[key];

    public bool TryGetVersioningNode(
        string visVersion,
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

        if (!_versioningsMap.ContainsKey(sourceVersion.ToVersionString()))
            throw new ArgumentException(
                "Source VIS Version does not exist in versionings: "
                    + sourceVersion.ToVersionString()
            );

        if (!_versioningsMap.ContainsKey(targetVersion.ToVersionString()))
            throw new ArgumentException(
                "Target VIS Version does not exist in versionings: "
                    + targetVersion.ToVersionString()
            );
    }
}

public readonly record struct GmodVersioningNode
{
    private readonly Dictionary<string, GmodVersioningNodeChanges> _versioningNodeChanges = new();

    internal GmodVersioningNode(IReadOnlyDictionary<string, GmodVersioningNodeChangesDto> dto)
    {
        foreach (var versioningNodeDto in dto)
        {
            var code = versioningNodeDto.Key;
            var versioningNodeChanges = new GmodVersioningNodeChanges(
                versioningNodeDto.Value.NextVisVersion,
                versioningNodeDto.Value.NextCode,
                versioningNodeDto.Value.PreviousVisVersion,
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

public sealed record GmodVersioningNodeChanges(
    string NextVisVersion,
    string NextCode,
    string PreviousVisVersion,
    string PreviousCode
);
