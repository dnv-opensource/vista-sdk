/*
    Advanced Local ID Operations - Vista SDK C#

    This example demonstrates advanced Local ID operations:
    - Building complex Local IDs with multiple components
    - Error handling during parsing
    - Working with custom tags
    - Local ID validation and inspection
*/
using Vista.SDK;
using Vista.SDK.Mqtt;

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

// 7. MQTT LocalId: formatting and components
Console.WriteLine("\n7. MQTT LocalId: MQTT-compatible formatting...");

string[] mqttExamples =
[
    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature",
    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
    "/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet",
];

foreach (var localIdStr in mqttExamples)
{
    if (LocalId.TryParse(localIdStr, out _, out var parsedLocalId))
    {
        var mqttLocalId = parsedLocalId.Builder.BuildMqtt();
        Console.WriteLine($"   Standard : {parsedLocalId}");
        Console.WriteLine($"   MQTT     : {mqttLocalId}");
        Console.WriteLine();
    }
}

// All 8 slots filled
var fullPrimary = gmod.ParsePath("411.1/C101.31-2");
var fullSecondary = gmod.ParsePath("411.1/C101.31-5");
var fullBuilder = LocalIdBuilder
    .Create(version)
    .WithPrimaryItem(fullPrimary)
    .WithSecondaryItem(fullSecondary)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Content, "exhaust.gas"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Calculation, "average"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.State, "high"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Command, "start"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Type, "instantaneous"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Position, "inlet"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Detail, "my_sensor_42"));

var fullStandard = fullBuilder.Build();
var fullMqtt = fullBuilder.BuildMqtt();

Console.WriteLine("   All 8 slots + secondary + free-form detail:");
Console.WriteLine($"     Standard : {fullStandard}");
Console.WriteLine($"     MQTT     : {fullMqtt}");
Console.WriteLine($"     Slots    : qty/cnt/calc/state/cmd/type/pos/detail");
Console.WriteLine($"     Detail   : '{fullStandard.Detail?.Value}' (IsCustom: {fullStandard.Detail?.IsCustom})");
Console.WriteLine();

// Reading components
var compPrimary = gmod.ParsePath("411.1/C101.31-2");
var compSecondary = gmod.ParsePath("411.1/C101.31-5");
var compBuilder = LocalIdBuilder
    .Create(version)
    .WithPrimaryItem(compPrimary)
    .WithSecondaryItem(compSecondary)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"))
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Content, "exhaust.gas"));

var mqttComp = compBuilder.BuildMqtt();

Console.WriteLine("   Reading components:");
Console.WriteLine($"     VisVersion     : {mqttComp.VisVersion.ToVersionString()}");
Console.WriteLine($"     PrimaryItem    : {mqttComp.PrimaryItem}");
Console.WriteLine($"     SecondaryItem  : {mqttComp.SecondaryItem}");
Console.WriteLine($"     Quantity       : {mqttComp.Quantity}");
Console.WriteLine($"     Content        : {mqttComp.Content}");
Console.WriteLine($"     Calculation    : {mqttComp.Calculation?.ToString() ?? "(none)"}");
Console.WriteLine();

// 8. MQTT LocalId: builder-level state and equality
Console.WriteLine("8. MQTT LocalId: builder-level state and equality...");

// Builder-level state not reflected in MQTT format
var verbosePrimary = gmod.ParsePath("411.1/C101.63/S206");
var verboseBuilder = LocalIdBuilder
    .Create(version)
    .WithVerboseMode(true)
    .WithPrimaryItem(verbosePrimary)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"));

var mqttVerbose = verboseBuilder.BuildMqtt();

Console.WriteLine("   Builder-level state (accessible via .Builder):");
Console.WriteLine("     VerboseMode/HasCustomTag/MetadataTags are not duplicated on MqttLocalId.");
Console.WriteLine("     VerboseMode has no effect on the MQTT format (unlike LocalId.ToString()).");
Console.WriteLine($"     Builder.VerboseMode    : {mqttVerbose.Builder.VerboseMode}");
Console.WriteLine($"     Builder.HasCustomTag   : {mqttVerbose.Builder.HasCustomTag}");
Console.WriteLine($"     Builder.MetadataTags   : {mqttVerbose.Builder.MetadataTags.Count} tag(s)");
Console.WriteLine($"     MQTT (no '~' despite VerboseMode=true): {mqttVerbose}");
Console.WriteLine();

// Equality
var eqPrimary = gmod.ParsePath("411.1/C101.31-2");
var eqBuilder = LocalIdBuilder
    .Create(version)
    .WithPrimaryItem(eqPrimary)
    .WithMetadataTag(codebooks.CreateTag(CodebookName.Quantity, "temperature"));

var a = eqBuilder.BuildMqtt();
var b = eqBuilder.BuildMqtt();
var c = eqBuilder.WithMetadataTag(codebooks.CreateTag(CodebookName.Content, "exhaust.gas")).BuildMqtt();

Console.WriteLine("   Equality:");
Console.WriteLine($"     a == b (same builder) : {a == b}");
Console.WriteLine($"     a == c (extra tag)    : {a == c}");
Console.WriteLine();

Console.WriteLine("   MQTT format differences vs standard:");
Console.WriteLine("     - No leading '/'");
Console.WriteLine("     - Underscores instead of slashes in paths");
Console.WriteLine("     - No 'meta/' section");
Console.WriteLine("     - '_' placeholder for absent metadata slots");
Console.WriteLine("     - 8 fixed slots: qty/cnt/calc/state/cmd/type/pos/detail");

Console.WriteLine("\n=== Advanced operations completed! ===");
