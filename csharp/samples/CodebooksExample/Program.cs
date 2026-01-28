/*
    Codebooks and Metadata Tags - Vista SDK C#

    This example demonstrates working with codebooks and metadata tags:
    - Loading and exploring codebooks
    - Creating standard and custom metadata tags
    - Validating tag values
    - Working with different tag types
*/

using Vista.SDK;

Console.WriteLine("=== Codebooks and Metadata Tags Example ===\n");

var version = VisVersion.v3_6a;
var codebooks = VIS.Instance.GetCodebooks(version);

// 1. Exploring available codebooks
Console.WriteLine("1. Available Codebooks...");

CodebookName[] availableCodebooks = Enum.GetValues<CodebookName>();

foreach (var codebookName in availableCodebooks)
{
    var codebook = codebooks[codebookName];
    Console.WriteLine($"   {codebookName}:");
    Console.WriteLine($"     → Standard values: {codebook.StandardValues.Count}");
    Console.WriteLine($"     → Groups: {codebook.Groups.Count}");
    Console.WriteLine();
}

// 2. Creating standard metadata tags
Console.WriteLine("2. Creating Metadata Tags...");

var tagsToCreate = new Dictionary<CodebookName, string[]>
{
    [CodebookName.Quantity] =  ["temperature", "pressure", "flow"],
    [CodebookName.Position] =  ["centre", "inlet", "outlet"],
    [CodebookName.State] =  ["opened", "closed", "high", "low"],
    [CodebookName.Content] =  ["cooling.water", "exhaust.gas", "fuel.oil"],
};

foreach (var (codebookName, values) in tagsToCreate)
{
    Console.WriteLine($"   {codebookName} tags:");
    var codebook = codebooks[codebookName];

    foreach (var value in values)
    {
        var tag = codebook.TryCreateTag(value);
        if (tag is not null)
        {
            Console.WriteLine($"     ✓ {value}: {tag} (custom: {tag.Value.IsCustom})");
        }
        else
        {
            Console.WriteLine($"     ✗ {value}: Failed to create tag");
        }
    }
    Console.WriteLine();
}

// 3. Working with custom tags
Console.WriteLine("3. Working with Custom Tags...");

(CodebookName name, string value)[] customExamples =
[
    (CodebookName.Quantity, "custom_temperature"),
    (CodebookName.Position, "custom_location"),
    (CodebookName.Content, "special.fluid"),
    (CodebookName.State, "partially_open"),
];

foreach (var (codebookName, customValue) in customExamples)
{
    var codebook = codebooks[codebookName];
    var customTag = codebook.TryCreateTag(customValue);
    if (customTag is not null)
    {
        Console.WriteLine($"   ✓ {codebookName}: {customTag} (custom: {customTag.Value.IsCustom})");
    }
    else
    {
        Console.WriteLine($"   ✗ {codebookName}: Failed to create tag for '{customValue}'");
    }
}

// 4. Tag validation
Console.WriteLine("\n4. Tag Validation...");

(CodebookName name, string value, bool expectedValid)[] validationTests =
[
    (CodebookName.Position, "centre", true),
    (CodebookName.Position, "invalid_position", false),
    (CodebookName.Quantity, "temperature", true),
    (CodebookName.Quantity, "nonexistent_quantity", false),
    (CodebookName.State, "opened", true),
    (CodebookName.State, "maybe_opened", false),
];

foreach (var (codebookName, testValue, expectedValid) in validationTests)
{
    var codebook = codebooks[codebookName];
    var isValid = codebook.HasStandardValue(testValue);
    var status = isValid == expectedValid ? "✓" : "✗";
    Console.WriteLine($"   {status} {codebookName}.{testValue}: valid={isValid} (expected: {expectedValid})");
}

// 5. Exploring codebook content
Console.WriteLine("\n5. Exploring Codebook Content...");

var stateCodebook = codebooks[CodebookName.State];
Console.WriteLine("   State codebook sample values:");

var values2 = stateCodebook.StandardValues.Take(10);
foreach (var value in values2)
{
    Console.WriteLine($"     - {value}");
}

Console.WriteLine("   Quantity codebook sample groups:");
var groups = stateCodebook.Groups.Take(5);
foreach (var group in groups)
{
    Console.WriteLine($"     - {group}");
}

Console.WriteLine("\n=== Codebooks example completed! ===");
