/*
    GMOD Operations - Vista SDK C#

    This example demonstrates working with GMOD (Generic Product Model):
    - Parsing and traversing GMOD paths
    - Exploring node hierarchies
    - Working with node metadata
    - Version conversion operations
*/

using Vista.SDK;

Console.WriteLine("=== GMOD Operations Example ===\n");

var version = VisVersion.v3_4a;
var gmod = VIS.Instance.GetGmod(version);

// 1. Basic GMOD path operations
Console.WriteLine("1. Basic GMOD Path Operations...");

string[] samplePaths = ["411.1", "411.1/C101.31-2", "612.21-1/C701.13/S93", "1021.1i-6P/H123"];

foreach (var pathStr in samplePaths)
{
    if (gmod.TryParsePath(pathStr, out var path))
    {
        var node = path.Node;
        Console.WriteLine($"   Path: {pathStr}");
        Console.WriteLine($"     → Code: {node.Code}");
        Console.WriteLine($"     → Name: {node.Metadata.Name}");
        Console.WriteLine($"     → Common Name: {node.Metadata.CommonName}");
        Console.WriteLine($"     → Depth: {path.Length}");
        Console.WriteLine();
    }
    else
    {
        Console.WriteLine($"   ✗ Failed to parse {pathStr}");
    }
}

// 2. Exploring node hierarchies
Console.WriteLine("2. Exploring Node Hierarchies...");

var rootPath = gmod.ParsePath("411.1");
var rootNode = rootPath.Node;

Console.WriteLine($"   Root node: {rootNode.Code} - {rootNode.Metadata.CommonName}");
Console.WriteLine("   Child nodes:");

var children = rootNode.Children.Take(5);
foreach (var child in children)
{
    Console.WriteLine($"     - {child.Code}: {child.Metadata.CommonName}");
}

// 3. Path traversal and analysis
Console.WriteLine("\n3. Path Traversal and Analysis...");

var deepPath = gmod.ParsePath("411.1/C101.31-2");

Console.WriteLine($"   Analyzing path: {deepPath}");
Console.WriteLine("   Full path traversal:");
var commonNames = deepPath.GetCommonNames().ToDictionary(c => c.Depth, c => c.Name);
foreach (var (depth, node) in deepPath.GetFullPath())
{
    if (depth == 0)
        continue; // Skip root
    var indent = new string(' ', (depth - 1) * 5);
    // Common names must be looked up as they often are a combination between parent and child
    var commonName = commonNames.GetValueOrDefault(depth) ?? node.Metadata.CommonName;
    Console.WriteLine($"   {indent}Depth {depth}: {node.Code} ({commonName})");
}

// 4. Node properties and metadata
Console.WriteLine("\n4. Node Properties and Metadata...");

string[] sampleNodes = ["411.1", "C101.31", "S206"];

foreach (var nodeCode in sampleNodes)
{
    GmodNode? node;
    // Access through direct lookup...
    try
    {
        node = gmod[nodeCode];
    }
    catch
    {
        Console.WriteLine($"   ✗ Node {nodeCode} not found");
        continue;
    }

    // Or through TryGet pattern
    if (gmod.TryGetNode(nodeCode, out node))
    {
        Console.WriteLine($"   Node: {nodeCode}");
        Console.WriteLine($"     → Name: {node.Metadata.Name}");
        Console.WriteLine($"     → Common Name: {node.Metadata.CommonName}");
        Console.WriteLine($"     → Is Mappable: {node.IsMappable}");
        Console.WriteLine($"     → Has Children: {node.Children.Any()}");
        Console.WriteLine();
    }
    else
    {
        Console.WriteLine($"   ✗ Node {nodeCode} not found");
    }
}

// 5. Path validation and error handling
Console.WriteLine("5. Path Validation and Error Handling...");

string[] invalidPaths = ["invalid.node", "411.1/invalid-child", "999.999/does-not-exist"];

foreach (var invalidPath in invalidPaths)
{
    if (!gmod.TryParsePath(invalidPath, out _))
    {
        Console.WriteLine($"   ✗ Path '{invalidPath}' is invalid (as expected)");
    }
    else
    {
        Console.WriteLine($"   ⚠ Path '{invalidPath}' unexpectedly parsed successfully");
    }
}

// 6. GMOD version conversion
Console.WriteLine("\n6. GMOD Version Conversion...");

(string path, VisVersion source, VisVersion target)[] conversionTests =
[
    ("111.3/H402", VisVersion.v3_7a, VisVersion.v3_8a),
    ("846/G203", VisVersion.v3_7a, VisVersion.v3_8a),
];

foreach (var (oldPathStr, sourceVersion, targetVersion) in conversionTests)
{
    var sourceGmod = VIS.Instance.GetGmod(sourceVersion);
    if (!sourceGmod.TryParsePath(oldPathStr, out var oldPath))
    {
        Console.WriteLine($"   ✗ Failed to parse {oldPathStr}");
        continue;
    }

    var newPath = VIS.Instance.ConvertPath(oldPath, targetVersion);
    if (newPath is not null)
    {
        Console.WriteLine($"   ✓ Converted: {oldPathStr} ({sourceVersion}) → {newPath} ({targetVersion})");
    }
    else
    {
        Console.WriteLine($"   ✗ Conversion returned null for {oldPathStr}");
    }
}

// 7. GMOD Traversal
Console.WriteLine("\n7. GMOD Traversal...");

// Simple traversal - count all nodes
var nodeCount = 0;
var completed = gmod.Traverse(
    (parents, node) =>
    {
        nodeCount++;
        return TraversalHandlerResult.Continue;
    }
);
Console.WriteLine($"   Traversal completed: {completed}, Nodes visited: {nodeCount}");

// Traversal with early stop - find first leaf node
GmodNode? firstLeaf = null;
completed = gmod.Traverse(
    (parents, node) =>
    {
        if (node.IsLeafNode)
        {
            firstLeaf = node;
            return TraversalHandlerResult.Stop;
        }
        return TraversalHandlerResult.Continue;
    }
);
Console.WriteLine($"   Traversal completed: {completed}, First leaf node: {firstLeaf}");

// Traversal from specific node
var startNode = gmod["411"];
var childCount = 0;
completed = gmod.Traverse(
    startNode,
    (parents, node) =>
    {
        childCount++;
        return TraversalHandlerResult.Continue;
    }
);

Console.WriteLine($"   Traversal completed: {completed}, Child nodes visited from '411': {childCount}");

Console.WriteLine("\n=== GMOD operations completed! ===");
