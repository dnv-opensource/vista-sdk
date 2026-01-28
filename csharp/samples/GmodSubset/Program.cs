/*
    Asset Model Example: Building a Digital Twin Structure from GmodPaths.

    This example demonstrates one way to build an asset model (digital twin structure)
    from a list of GmodPaths. This is the foundation for:
    - Defining what equipment exists on your vessel
    - Creating hierarchical visualizations
    - Generating LocalIds for sensor data

    The key insight is that a GmodPath like "411.1-1/C101/C101.31-1" implicitly
    contains the entire hierarchy. GMOD validates that the path is structurally
    valid, and we can build a tree by combining multiple paths.
*/
using System.Text.Json;
using Vista.SDK;

// Run all examples
Console.WriteLine(new string('=', 70));
Console.WriteLine("Asset Model Examples: Building Digital Twin Structures from GmodPaths");
Console.WriteLine(new string('=', 70));
Console.WriteLine();
Console.WriteLine("These examples demonstrate how to define what equipment exists");
Console.WriteLine("on your vessel using GmodPaths, then build a tree structure");
Console.WriteLine("for visualization and further processing (like LocalId generation).");

ExampleDualEngineVessel();
ExampleFromLocalIds();
ExampleJsonExport();

Console.WriteLine("\n" + new string('=', 70));
Console.WriteLine("All examples completed!");
Console.WriteLine(new string('=', 70));

// =========================================================================
// Example 1: Dual-engine Vessel Asset Model
// =========================================================================
void ExampleDualEngineVessel()
{
    Console.WriteLine("\n" + new string('=', 70));
    Console.WriteLine("Example 1: Dual-engine Vessel Asset Model");
    Console.WriteLine(new string('=', 70));

    var visVersion = VisVersion.v3_4a;

    // Define the equipment that exists on this vessel
    // Each path is a "leaf" item e.g. ISO19848 Annex C short path - the full hierarchy is implicit
    string[] assetPaths =
    [
        // === Port Main Engine (411.1-P) with 6 cylinders ===
        "411.1-P/C101/C101.31-1", // Port engine, cylinder 1
        "411.1-P/C101/C101.31-2", // Port engine, cylinder 2
        "411.1-P/C101/C101.31-3", // Port engine, cylinder 3
        "411.1-P/C101/C101.31-4", // Port engine, cylinder 4
        "411.1-P/C101/C101.31-5", // Port engine, cylinder 5
        "411.1-P/C101/C101.31-6", // Port engine, cylinder 6
        "411.1-P/C101/C101.63/S206", // Port engine, cooling system
        // === Starboard Main Engine (411.1-S) with 6 cylinders ===
        "411.1-S/C101/C101.31-1", // Starboard engine, cylinder 1
        "411.1-S/C101/C101.31-2", // Starboard engine, cylinder 2
        "411.1-S/C101/C101.31-3", // Starboard engine, cylinder 3
        "411.1-S/C101/C101.31-4", // Starboard engine, cylinder 4
        "411.1-S/C101/C101.31-5", // Starboard engine, cylinder 5
        "411.1-S/C101/C101.31-6", // Starboard engine, cylinder 6
        "411.1-S/C101/C101.63/S206", // Starboard engine, cooling system
        // === Generator Sets ===
        "511.11-1/C101", // Generator 1, diesel engine
        "511.11-2/C101", // Generator 2, diesel engine
        // === Fuel System ===
        "621.21/S90", // Fuel oil transfer system, piping
    ];

    Console.WriteLine($"\n  Defining {assetPaths.Length} equipment paths...");
    Console.WriteLine("  (Each path implicitly includes all parent nodes)");

    // Build the asset model
    var model = AssetModel.FromPathStrings(visVersion, assetPaths);

    Console.WriteLine($"\n  Built model with {model.NodeCount} total nodes");
    Console.WriteLine($"  Maximum depth: {model.MaxDepth}");

    // Demonstrate nodes_by_code lookup - "show me all engines"
    Console.WriteLine("\n  === Lookup: All Engines (C101) ===");
    var engines = model.GetNodesByCode("C101");
    Console.WriteLine($"  Found {engines.Count} instances of C101 (Internal combustion engine):");
    foreach (var engine in engines)
    {
        Console.WriteLine($"    - Path: {engine.Path}");
    }

    // Demonstrate looking up all cylinders
    Console.WriteLine("\n  === Lookup: All Cylinders (C101.31) ===");
    var cylinders = model.GetNodesByCode("C101.31");
    Console.WriteLine($"  Found {cylinders.Count} instances of C101.31 (Cylinder):");
    foreach (var cyl in cylinders.Take(6))
    {
        Console.WriteLine($"    - Path: {cyl.Path}");
    }
    if (cylinders.Count > 6)
    {
        Console.WriteLine($"    ... and {cylinders.Count - 6} more");
    }

    // Show available codes in the model
    Console.WriteLine("\n  === All available codes in model ===");
    var codes = model.NodesByCode.Keys.OrderBy(c => c).ToList();
    Console.WriteLine($"  {codes.Count} unique codes: [{string.Join(", ", codes)}]");

    Console.WriteLine("\n  Asset Model Tree:");
    Console.WriteLine();
    model.PrintTree();
}

// =========================================================================
// Example 2: Build Model from LocalIds (Runtime Flow)
// =========================================================================
void ExampleFromLocalIds()
{
    Console.WriteLine("\n" + new string('=', 70));
    Console.WriteLine("Example 2: Build Model from LocalIds (Runtime Flow)");
    Console.WriteLine(new string('=', 70));

    var visVersion = VisVersion.v3_4a;

    // Simulate LocalIds coming from a DataChannelList or data source
    string[] localIdStrings =
    [
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-3/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-2/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-2/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/511.11-1/C101/meta/qty-revolution",
    ];

    Console.WriteLine($"\n  Received {localIdStrings.Length} LocalIds from data source");

    // Parse LocalIds and extract GmodPaths
    List<GmodPath> paths = [];
    foreach (var lidStr in localIdStrings)
    {
        var localId = LocalId.Parse(lidStr);
        if (localId.PrimaryItem is not null)
        {
            paths.Add(localId.PrimaryItem);
        }
        if (localId.SecondaryItem is not null)
        {
            paths.Add(localId.SecondaryItem);
        }
    }

    Console.WriteLine($"  Extracted {paths.Count} GmodPaths from LocalIds");

    // Build model from extracted paths
    var model = AssetModel.FromPaths(visVersion, paths);

    Console.WriteLine($"\n  Built model with {model.NodeCount} nodes");
    Console.WriteLine($"  Maximum depth: {model.MaxDepth}");

    Console.WriteLine("\n  Asset Model Tree (derived from LocalIds):");
    Console.WriteLine();
    model.PrintTree();
}

// =========================================================================
// Example 3: JSON Export for Visualization
// =========================================================================
void ExampleJsonExport()
{
    Console.WriteLine("\n" + new string('=', 70));
    Console.WriteLine("Example 3: JSON Export for Visualization");
    Console.WriteLine(new string('=', 70));

    var visVersion = VisVersion.v3_4a;

    // Small example for readable JSON
    string[] assetPaths = ["411.1/C101/C101.31-1", "411.1/C101/C101.31-2"];

    var model = AssetModel.FromPathStrings(visVersion, assetPaths);

    Console.WriteLine("\n  JSON output (for D3.js, etc.):");
    Console.WriteLine();
    Console.WriteLine(model.ToJson());
}

// =========================================================================
// Asset Model Classes
// =========================================================================

/// <summary>
/// A node in the asset model tree.
/// Wraps a GmodNode with tree structure for visualization.
/// </summary>
sealed class AssetNode
{
    public GmodNode Node { get; }

    // Optional due to root node. All other nodes should have a path.
    public GmodPath? Path { get; }
    public int Depth { get; }

    private string? _displayName { get; }
    public List<AssetNode> Children { get; } = [];
    public AssetNode? Parent { get; set; }

    public AssetNode(GmodNode node, GmodPath? path, int depth)
    {
        Node = node;
        Path = path;
        Depth = depth;
        // Common names are some time a combination of parent+child, so cache it here
        // Example: Component "C101 - Reciprocating internal combustion engine" becomes "C101 - Propulsion engine" in context of 411.1
        _displayName = path?.GetCommonNames().Last().Name;
    }

    public void AddChild(AssetNode child)
    {
        child.Parent = this;
        Children.Add(child);
    }

    public Dictionary<string, object?> ToDict()
    {
        return new Dictionary<string, object?>
        {
            ["path"] = Path?.ToString(),
            ["code"] = Node.Code,
            ["name"] = Node.Metadata.Name,
            ["commonName"] = Node.Metadata.CommonName,
            ["displayName"] = _displayName ?? Node.Metadata.Name,
            ["category"] = Node.Metadata.Category,
            ["type"] = Node.Metadata.Type,
            ["location"] = Node.Location?.ToString(),
            ["children"] = Children
                .OrderBy(c => c.Path?.ToString())
                .Where(c => c.Path is not null)
                .Select(c => c.ToDict())
                .ToList(),
        };
    }

    public void PrintTree(int indent = 0)
    {
        var prefix = new string(' ', indent * 2);
        var connector = indent > 0 ? "├─" : "";
        var displayName = _displayName;
        var location = Node.Location is not null ? $" [{Node.Location}]" : "";
        Console.WriteLine($"{prefix}{connector} {Node.Code}{location}: {displayName}");
        foreach (var child in Children.OrderBy(c => c.Path?.ToString()).Where(c => c.Path is not null))
        {
            child.PrintTree(indent + 1);
        }
    }
}

///<summary>
///An asset model built from GmodPaths.
///This class builds a tree structure from a list of GmodPaths.
///</summary>
sealed class AssetModel
{
    public VisVersion VisVersion { get; }
    public AssetNode? Root { get; private set; }

    private readonly Dictionary<string, AssetNode> _nodeMap = [];
    private readonly Dictionary<string, List<AssetNode>> _nodesByCode = [];

    private AssetModel(VisVersion visVersion)
    {
        VisVersion = visVersion;
    }

    public static AssetModel FromPaths(VisVersion visVersion, IEnumerable<GmodPath> paths)
    {
        var model = new AssetModel(visVersion);
        foreach (var path in paths)
        {
            model.AddPath(path);
        }
        return model;
    }

    public static AssetModel FromPathStrings(VisVersion visVersion, IEnumerable<string> pathStrings)
    {
        var gmod = VIS.Instance.GetGmod(visVersion);
        var paths = new List<GmodPath>();
        foreach (var pathStr in pathStrings)
        {
            if (gmod.TryParsePath(pathStr, out var path))
            {
                paths.Add(path);
            }
            else
            {
                Console.WriteLine($"  Warning: Could not parse path '{pathStr}'");
            }
        }
        return FromPaths(visVersion, paths);
    }

    private void AddPath(GmodPath path)
    {
        var fullPath = path.GetFullPath().ToList();

        for (var i = 0; i < fullPath.Count; i++)
        {
            var (depth, node) = fullPath[i];

            // Build the full path ID up to this point
            var nodeId = string.Join("/", fullPath.Take(i + 1).Select(p => p.Node.ToString()));

            // Skip if already added
            if (_nodeMap.ContainsKey(nodeId))
                continue;
            // Build the GmodPath for this node
            var parents = fullPath.Take(i).Select(p => p.Node).ToList();
            var nodePath = depth == 0 ? null : new GmodPath(parents, node);

            // Create the asset node
            var assetNode = new AssetNode(node, nodePath, depth);

            // Add to map
            _nodeMap[nodeId] = assetNode;

            // Add to nodes_by_code index
            var code = node.Code;
            if (!_nodesByCode.TryGetValue(code, out var codeList))
            {
                codeList =  [];
                _nodesByCode[code] = codeList;
            }
            codeList.Add(assetNode);

            // Handle root node
            if (depth == 0)
            {
                Root ??= assetNode;
                continue;
            }

            // Find parent and link
            var parentId = string.Join("/", fullPath.Take(i).Select(p => p.Node.ToString()));
            if (_nodeMap.TryGetValue(parentId, out var parentNode))
            {
                parentNode.AddChild(assetNode);
            }
        }
    }

    public AssetNode? GetNode(string nodeId) => _nodeMap.GetValueOrDefault(nodeId);

    public IReadOnlyDictionary<string, List<AssetNode>> NodesByCode => _nodesByCode;

    public List<AssetNode> GetNodesByCode(string code) => _nodesByCode.TryGetValue(code, out var list) ? list : [];

    public int NodeCount => _nodeMap.Count;

    public int MaxDepth => _nodeMap.Count > 0 ? _nodeMap.Values.Max(n => n.Depth) : 0;

    public Dictionary<string, object?> ToDict() => Root?.ToDict() ?? new Dictionary<string, object?>();

    public string ToJson()
    {
        var options = new JsonSerializerOptions { WriteIndented = true };
        return JsonSerializer.Serialize(ToDict(), options);
    }

    public void PrintTree() => Root?.PrintTree();
}
