/*
    Basic Usage Example - Vista SDK C#

    This example demonstrates the fundamental operations of the Vista SDK:
    - Initializing VIS
    - Working with GMOD paths
    - Creating Local IDs
    - Using codebooks
*/

using Vista.SDK;

Console.WriteLine("=== Vista SDK Basic Usage Example ===\n");

// 1. Initialize VIS instance
Console.WriteLine("1. Initializing VIS...");
var version = VisVersion.v3_4a;
Console.WriteLine($"   Loading data for VIS version: {version}");

var gmod = VIS.Instance.GetGmod(version);
var codebooks = VIS.Instance.GetCodebooks(version);

Console.WriteLine($"   ✓ GMOD loaded");
Console.WriteLine("   ✓ Codebooks loaded\n");

// 2. Work with Gmod node
Console.WriteLine("2. Working with Gmod nodes...");

var engineNode = gmod["C101"];
Console.WriteLine($"   Node code: {engineNode.Code}");
Console.WriteLine($"   Common name: {engineNode.Metadata.CommonName}");
Console.WriteLine($"   Category: {engineNode.Metadata.Category}");
Console.WriteLine($"   Is leaf: {engineNode.IsLeafNode}");

Console.WriteLine($"   Children ({engineNode.Children.Count}):");
foreach (var child in engineNode.Children.Take(5))
{
    Console.WriteLine($"     → {child.Code}: {child.Metadata.CommonName}");
}

if (engineNode.Children.Count > 5)
    Console.WriteLine($"     ... and {engineNode.Children.Count - 5} more");

Console.WriteLine();

// 3. Parse GMOD paths
Console.WriteLine("3. Working with GMOD paths...");
string[] pathStrings = ["411.1/C101.31-2", "612.21-1/C701.13/S93", "1021.1i-6P/H123"];

List<GmodPath> paths = [];
foreach (var pathStr in pathStrings)
{
    if (gmod.TryParsePath(pathStr, out var path))
    {
        paths.Add(path);
        var node = path.Node;
        Console.WriteLine($"   ✓ Parsed: {pathStr}");
        Console.WriteLine($"     → Node: {node} ({node.Metadata.CommonName})");
        Console.WriteLine($"     → Depth: {path.Length}");
    }
    else
    {
        Console.WriteLine($"   ✗ Failed to parse {pathStr}");
    }
}

Console.WriteLine();

// 4. Work with codebooks
Console.WriteLine("4. Using codebooks...");

var quantityBook = codebooks[CodebookName.Quantity];
var positionBook = codebooks[CodebookName.Position];
var contentBook = codebooks[CodebookName.Content];

var tags = new Dictionary<string, MetadataTag>
{
    ["quantity"] = quantityBook.CreateTag("temperature"),
    ["position"] = positionBook.CreateTag("centre"),
    ["content"] = contentBook.CreateTag("cooling.water"),
};

foreach (var (tagType, tag) in tags)
{
    Console.WriteLine($"   ✓ Created {tagType} tag: {tag.Value} (custom: {tag.IsCustom})");
}

Console.WriteLine();

// 5. Build Local IDs
Console.WriteLine("5. Creating Local IDs...");

for (var i = 0; i < Math.Min(2, paths.Count); i++)
{
    var path = paths[i];
    try
    {
        var localId = LocalIdBuilder
            .Create(version)
            .WithPrimaryItem(path)
            .WithMetadataTag(tags["quantity"])
            .WithMetadataTag(tags["content"])
            .WithMetadataTag(tags["position"])
            .Build();

        Console.WriteLine($"   ✓ Local ID {i + 1}: {localId}");
        Console.WriteLine($"     → Primary item: {localId.PrimaryItem}");
    }
    catch (Exception e)
    {
        Console.WriteLine($"   ✗ Failed to create Local ID for {path}: {e.Message}");
    }
}

Console.WriteLine();

// 6. Demonstrate parsing existing Local IDs
Console.WriteLine("6. Parsing existing Local IDs...");

string[] sampleLocalIds =
[
    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
    "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
];

foreach (var localIdStr in sampleLocalIds)
{
    if (LocalId.TryParse(localIdStr, out _, out var localId))
    {
        Console.WriteLine($"   ✓ Parsed: {localIdStr}");
        Console.WriteLine($"     → Primary: {localId.PrimaryItem}");
        Console.WriteLine($"     → Metadata tags: {localId.MetadataTags.Count}");

        foreach (var tag in localId.MetadataTags)
        {
            Console.WriteLine($"       - {tag.Name}: {tag.Value}");
        }
    }
    else
    {
        Console.WriteLine($"   ✗ Failed to parse {localIdStr}");
    }
}

Console.WriteLine("\n=== Example completed successfully! ===");
