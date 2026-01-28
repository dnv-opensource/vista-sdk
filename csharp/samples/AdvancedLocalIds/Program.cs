/*
    Advanced Local ID Operations - Vista SDK C#

    This example demonstrates advanced Local ID operations:
    - Building complex Local IDs with multiple components
    - Error handling during parsing
    - Working with custom tags
    - Local ID validation and inspection
*/
using Vista.SDK;

Console.WriteLine("=== Advanced Local ID Operations ===\n");

var version = VisVersion.v3_4a;
var gmod = VIS.Instance.GetGmod(version);
var codebooks = VIS.Instance.GetCodebooks(version);

// 1. Building complex Local IDs
Console.WriteLine("1. Building Complex Local IDs...");

var primaryPath = gmod.ParsePath("411.1/C101.31-2");
var secondaryPath = gmod.ParsePath("411.1/C101.63/S206");

var complexLocalId = LocalIdBuilder
    .Create(version)
    .WithPrimaryItem(primaryPath)
    .WithSecondaryItem(secondaryPath)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Content, "exhaust.gas"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.State, "high"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Position, "inlet"))
    .Build();

Console.WriteLine($"   Complex Local ID: {complexLocalId}");
Console.WriteLine($"   Has secondary item: {complexLocalId.SecondaryItem is not null}");
Console.WriteLine($"   Number of metadata tags: {complexLocalId.MetadataTags.Count}");

// 2. Working with custom tags
Console.WriteLine("\n2. Working with Custom Tags...");

var customQuantity = codebooks.TryCreateTag(CodebookName.Quantity, "custom_temperature");
var customPosition = codebooks.TryCreateTag(CodebookName.Position, "custom_location");

if (customQuantity is not null && customPosition is not null)
{
    var customLocalId = LocalIdBuilder
        .Create(version)
        .WithPrimaryItem(primaryPath)
        .WithMetadataTag(customQuantity.Value)
        .WithMetadataTag(customPosition.Value)
        .Build();

    Console.WriteLine($"   Custom Local ID: {customLocalId}");
    Console.WriteLine($"   Quantity tag is custom: {customLocalId.Quantity?.IsCustom}");
    Console.WriteLine($"   Position tag is custom: {customLocalId.Position?.IsCustom}");
}

// 3. Error handling and validation
Console.WriteLine("\n3. Error Handling and Validation...");

string[] invalidLocalIds =
[
    "/dnv-v2/vis-3-4a/invalid-path/meta/qty-temperature",
    "/invalid-naming-rule/vis-3-4a/411.1/meta/qty-temperature",
    "/dnv-v2/vis-3-4a/411.1/meta/qty-invalid_quantity",
];

foreach (var invalidId in invalidLocalIds)
{
    Console.WriteLine($"\n   Testing invalid Local ID: {invalidId}");

    if (!LocalIdBuilder.TryParse(invalidId, out var errors, out var localId))
    {
        Console.WriteLine("     ✗ Parsing failed as expected");
        if (errors.HasErrors)
        {
            Console.WriteLine("     Errors found:");
            foreach (var (errorType, message) in errors)
            {
                Console.WriteLine($"       - {errorType}: {message}");
            }
        }
    }
    else
    {
        Console.WriteLine($"     ⚠ Unexpectedly parsed successfully: {localId}");
    }
}

// 4. Local ID inspection and analysis
Console.WriteLine("\n4. Local ID Inspection and Analysis...");

string[] validLocalIds =
[
    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
    "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
    "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
];

foreach (var localIdStr in validLocalIds)
{
    Console.WriteLine($"\n   Analyzing: {localIdStr}");

    if (LocalId.TryParse(localIdStr, out _, out var localId))
    {
        Console.WriteLine("     ✓ Successfully parsed");
        Console.WriteLine($"     ✓ VIS Version: {localId.VisVersion.ToVersionString()}");
        Console.WriteLine($"     ✓ Has custom tags: {localId.HasCustomTag}");
        Console.WriteLine($"     ✓ Primary item: {localId.PrimaryItem}");

        if (localId.SecondaryItem is not null)
        {
            Console.WriteLine($"     ✓ Secondary item: {localId.SecondaryItem}");
        }

        Console.WriteLine("     ✓ Metadata tags:");
        foreach (var tag in localId.MetadataTags)
        {
            Console.WriteLine($"       - {tag.Name}: {tag.Value} (custom: {tag.IsCustom})");
        }
    }
    else
    {
        Console.WriteLine($"     ✗ Failed to parse");
    }
}

// 5. Builder pattern variations
Console.WriteLine("\n5. Builder Pattern Variations...");

var builder = LocalIdBuilder.Create(version);

// Try adding components safely
if (gmod.TryParsePath("411.1/C101.31", out var pathForBuilder))
{
    builder = builder.WithPrimaryItem(pathForBuilder);
}

// Try with valid tags
var tempTag = codebooks.TryCreateTag(CodebookName.Quantity, "temperature");
if (tempTag is not null)
{
    builder = builder.WithMetadataTag(tempTag.Value);
}

var exhaustTag = codebooks.TryCreateTag(CodebookName.Content, "exhaust.gas");
if (exhaustTag is not null)
{
    builder = builder.WithMetadataTag(exhaustTag.Value);
}

try
{
    var safeLocalId = builder.Build();
    Console.WriteLine($"   Safe building result: {safeLocalId}");
}
catch (Exception e)
{
    Console.WriteLine($"   Safe building failed: {e.Message}");
}

// 6. Verbose mode demonstration
Console.WriteLine("\n6. Verbose Mode...");

var verboseLocalId = LocalIdBuilder
    .Create(version)
    .WithVerboseMode(true)
    .WithPrimaryItem(primaryPath)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"))
    .Build();

var regularLocalId = LocalIdBuilder
    .Create(version)
    .WithVerboseMode(false)
    .WithPrimaryItem(primaryPath)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"))
    .Build();

Console.WriteLine($"   Verbose mode: {verboseLocalId}");
Console.WriteLine($"   Regular mode: {regularLocalId}");

Console.WriteLine("\n=== Advanced operations completed! ===");
