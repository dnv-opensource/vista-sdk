/*
   Local ID Query Operations - Vista SDK C#

   This example demonstrates how to use Local ID queries to filter and match
   Local IDs based on various criteria:
   - GmodPathQuery for matching paths with or without locations
   - MetadataTagsQuery for matching metadata tags
   - LocalIdQuery for combining path and tag queries
*/

using Vista.SDK;

Console.WriteLine("=== Local ID Query Operations ===\n");

var version = VisVersion.v3_4a;
var gmod = VIS.Instance.GetGmod(version);

// Sample Local IDs to query against
string[] localIdStrings =
[
    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
    "/dnv-v2/vis-3-4a/411.1/C101.31-5/meta/qty-temperature/cnt-exhaust.gas/pos-outlet",
    "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-pressure/cnt-fuel.oil",
    "/dnv-v2/vis-3-4a/411.1-2/C101.63/S206/meta/qty-pressure/cnt-lubricating.oil",
    "/dnv-v2/vis-3-9a/411.1/C101/sec/412.3/meta/qty-power",
    "/dnv-v2/vis-3-4a/511.11/C101.63/S206/meta/qty-pressure/cnt-lubricating.oil",
    "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
    "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo",
];

var parsedLocalIds = localIdStrings.Select(LocalId.Parse).ToList();

// =========================================================================
// 1. GmodPathQuery - Matching paths
// =========================================================================
Console.WriteLine("1. GmodPathQuery - Matching Paths");
Console.WriteLine(new string('-', 50));

// 1a. Exact path match (with locations)
Console.WriteLine("\n1a. Exact path match (including locations):");
var exactPath = gmod.ParsePath("411.1/C101.31-2");
var exactQuery = GmodPathQueryBuilder.From(exactPath).Build();

foreach (var lid in parsedLocalIds)
{
    if (exactQuery.Match(lid.PrimaryItem))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 1b. Path match ignoring locations
Console.WriteLine("\n1b. Path match ignoring locations (matches any cylinder):");
var pathNoLoc = gmod.ParsePath("411.1/C101.31-2");
var queryNoLoc = GmodPathQueryBuilder.From(pathNoLoc).WithoutLocations().Build();

foreach (var lid in parsedLocalIds)
{
    if (queryNoLoc.Match(lid.PrimaryItem))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 1c. Match specific node with any location
Console.WriteLine("\n1c. Match node 411.1 with any location using Nodes builder:");
var node411 = gmod["411.1"];
var nodesQuery = GmodPathQueryBuilder.Empty().WithNode(node411, true).Build();

foreach (var lid in parsedLocalIds)
{
    if (nodesQuery.Match(lid.PrimaryItem))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 1d. Match accross VIS versions
Console.WriteLine($"\n1d. Match path across VIS versions (3-7a and {VIS.LatestVisVersion.ToVersionString()}):");
var l1 = LocalId.Parse(
        "/dnv-v2/vis-3-7a/411.1/C111.1/C103.17/meta/qty-temperature/cnt-exhaust.gas/pos-outlet"
    );
var l2 = VIS.Instance.ConvertLocalId(
        l1,
        VIS.LatestVisVersion
    );
if (l2 is not null)
{
    var query = GmodPathQueryBuilder
        .From(l1.PrimaryItem)
        .WithoutLocations()
        .Build();
    foreach (var lid in new[] { l1, l2 })
    {
        if (query.Match(lid.PrimaryItem))
        {
            Console.WriteLine($"   ✓ Matched: {lid}");
        }
    }
}

// =========================================================================
// 2. MetadataTagsQuery - Matching tags
// =========================================================================
Console.WriteLine("\n\n2. MetadataTagsQuery - Matching Tags");
Console.WriteLine(new string('-', 50));

// 2a. Match by single tag
Console.WriteLine("\n2a. Match Local IDs with temperature quantity:");
var tempTagQuery = MetadataTagsQueryBuilder.Empty().WithTag(CodebookName.Quantity, "temperature").Build();

foreach (var lid in parsedLocalIds)
{
    if (tempTagQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 2b. Match by multiple tags
Console.WriteLine("\n2b. Match Local IDs with temperature AND outlet:");
var multiTagQuery = MetadataTagsQueryBuilder
    .Empty()
    .WithTag(CodebookName.Quantity, "temperature")
    .WithTag(CodebookName.Position, "outlet")
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (multiTagQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 2c. Exact tag match (no other tags allowed)
Console.WriteLine("\n2c. Match Local IDs with ONLY pressure and fuel.oil (exact match):");
var exactTagsQuery = MetadataTagsQueryBuilder
    .Empty()
    .WithTag(CodebookName.Quantity, "pressure")
    .WithTag(CodebookName.Content, "fuel.oil")
    .WithAllowOtherTags(false)
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (exactTagsQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 2d. Create query from existing LocalId
Console.WriteLine("\n2d. Create tag query from an existing LocalId:");
var sourceLid = parsedLocalIds[0];
var fromLidQuery = MetadataTagsQueryBuilder.From(sourceLid).Build();
Console.WriteLine($"   Source: {sourceLid}");
Console.WriteLine("   Matching Local IDs:");
foreach (var lid in parsedLocalIds)
{
    if (fromLidQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// =========================================================================
// 3. LocalIdQuery - Combined queries
// =========================================================================
Console.WriteLine("\n\n3. LocalIdQuery - Combined Path and Tag Queries");
Console.WriteLine(new string('-', 50));

// 3a. Match by primary item path
Console.WriteLine("\n3a. Match by primary item path (411.1/C101.63/S206):");
var primaryQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(
        GmodPathQueryBuilder.From(gmod.ParsePath("411.1/C101.63/S206")).WithoutLocations().Build()
    )
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (primaryQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 3b. Match by tags
Console.WriteLine("\n3b. Match by tags:");
var tagsQuery = LocalIdQueryBuilder
    .Empty()
    .WithTags(tags => tags.WithTag(CodebookName.Content, "sea.water").Build())
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (tagsQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 3c. Combined path and tag query
Console.WriteLine("\n3c. Combined: C101.31 cylinder (any) with temperature:");
var combinedQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(GmodPathQueryBuilder.From(gmod.ParsePath("411.1/C101.31-2")).WithoutLocations().Build())
    .WithTags(tags => tags.WithTag(CodebookName.Quantity, "temperature").Build())
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (combinedQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 3d. Create query from existing LocalId string
Console.WriteLine("\n3d. Create query from LocalId string:");
var stringQuery = LocalIdQueryBuilder
    .From("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet")
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (stringQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// =========================================================================
// 4. Advanced Query Patterns
// =========================================================================
Console.WriteLine("\n\n4. Advanced Query Patterns");
Console.WriteLine(new string('-', 50));

// 4a. Using NodesConfig for flexible node matching
Console.WriteLine("\n4a. Using nodes builder to match specific nodes (must have 411.1 and S206):");
var nodeConfigQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(nodes => nodes.WithNode(gmod["411.1"], true).WithNode(gmod["S206"], true).Build())
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (nodeConfigQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 4b. Using PathConfig to modify path matching
Console.WriteLine("\n4b. Using path configuration to customize path matching:");
var basePath = GmodPath.Parse("411.1/C101.31-2", version);
var pathConfigQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(basePath, path => path.WithoutLocations().Build())
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (pathConfigQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 4c. Using WithAnyNodeBefore to match only the leaf node
Console.WriteLine("\n4c. Using WithAnyNodeBefore to ignore parent nodes (any path before S206):");
var sensorPath = gmod.ParsePath("411.1/C101.63/S206");
var anyBeforeQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(sensorPath, path => path.WithoutLocations().WithAnyNodeBefore(nodes => nodes["S206"]).Build())
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (anyBeforeQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 4d. Using with_any_node_after for prefix-style matching
Console.WriteLine("\n4d. Using WithAnyNodeAfter for prefix matching (411.1/*):");
Console.WriteLine("   Match any path with 411.1, regardless of children:");
var prefixPath = gmod.ParsePath("411.1/C101.31");
var anyAfterQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(prefixPath, path => path.WithoutLocations().WithAnyNodeAfter(nodes => nodes["411.1"]).Build())
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (anyAfterQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// 4e. Query requiring NO secondary item
Console.WriteLine("\n4e. Match Local IDs that include 411.1 and have no secondary item:");
var noSecondaryQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(nodes => nodes.WithNode(gmod["411.1"], true).Build())
    .WithoutSecondaryItem()
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (noSecondaryQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

//  4f. Query with any or no secondary item
Console.WriteLine("\n4f. Match Local IDs that include 411.1 and have any or no secondary item:");
var anyOrNoSecondaryQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(nodes => nodes.WithNode(gmod["411.1"], true).Build())
    .WithAnySecondaryItem()
    .Build();

foreach (var lid in parsedLocalIds)
{
    if (anyOrNoSecondaryQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ Matched: {lid}");
    }
}

// =========================================================================
// 5. Practical Use Case: Filtering Sensor Data
// =========================================================================
Console.WriteLine("\n\n5. Practical Use Case: Filtering Sensor Data");
Console.WriteLine(new string('-', 50));

Console.WriteLine("\n   Scenario: Find all temperature sensors on the main engine");

var mePath = GmodPath.Parse("411.1/C101", version);
var engineTempQuery = LocalIdQueryBuilder
    .Empty()
    .WithPrimaryItem(mePath, path => path.WithAnyNodeAfter(nodes => nodes["C101"]).WithoutLocations().Build())
    .WithTags(tags => tags.WithTag(CodebookName.Quantity, "temperature").Build())
    .Build();

Console.WriteLine("   Query: Primary item contains 411.1/C101 (any location) + qty=temperature");
Console.WriteLine("   Results:");
foreach (var lid in parsedLocalIds)
{
    if (engineTempQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ {lid}");
    }
}

Console.WriteLine("\n   Scenario: Find all pressure sensors with fuel or lubricating oil");
Console.WriteLine("   Note: OR conditions require multiple queries");

var fuelOilQuery = LocalIdQueryBuilder
    .Empty()
    .WithTags(tags => tags.WithTag(CodebookName.Quantity, "pressure").WithTag(CodebookName.Content, "fuel.oil").Build())
    .Build();

var lubeOilQuery = LocalIdQueryBuilder
    .Empty()
    .WithTags(
        tags => tags.WithTag(CodebookName.Quantity, "pressure").WithTag(CodebookName.Content, "lubricating.oil").Build()
    )
    .Build();

Console.WriteLine("   Query: qty=pressure AND (cnt=fuel.oil OR cnt=lubricating.oil)");
Console.WriteLine("   Results:");
foreach (var lid in parsedLocalIds)
{
    if (fuelOilQuery.Match(lid) || lubeOilQuery.Match(lid))
    {
        Console.WriteLine($"   ✓ {lid}");
    }
}

Console.WriteLine("\n=== Query Operations Complete ===");
