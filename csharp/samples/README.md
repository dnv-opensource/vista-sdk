# Vista SDK C# Samples

This directory contains practical examples demonstrating how to use the Vista SDK C# implementation. Each sample focuses on specific aspects of the SDK and can be run independently.

## 📁 Sample Files

### 🚀 [`BasicUsage`](BasicUsage/Program.cs)

**Perfect starting point** - Demonstrates fundamental operations:

- Initializing VIS and loading data
- Parsing GMOD paths
- Working with codebooks and metadata tags
- Building and parsing Local IDs

```bash
dotnet run --project samples/BasicUsage
```

### 🔧 [`CodebooksExample`](CodebooksExample/Program.cs)

**Deep dive into codebooks** - Shows how to:

- Explore available codebooks and their content
- Create standard and custom metadata tags
- Validate tag values
- Handle position validation special cases

```bash
dotnet run --project samples/CodebooksExample
```

### 🏗️ [`GmodOperations`](GmodOperations/Program.cs)

**GMOD (Generic Product Model) operations** - Covers:

- Parsing and analyzing GMOD paths
- Exploring node hierarchies and metadata
- Working with different VIS versions
- Path validation and error handling
- GMOD traversal patterns

```bash
dotnet run --project samples/GmodOperations
```

### 🎯 [`AdvancedLocalIds`](AdvancedLocalIds/Program.cs)

**Advanced Local ID operations** - Demonstrates:

- Building complex Local IDs with multiple components
- Error handling and validation
- Working with custom tags
- Builder pattern variations and safe operations

```bash
dotnet run --project samples/AdvancedLocalIds
```

### 🔍 [`LocalIdQuery`](LocalIdQuery/Program.cs)

**Local ID Query operations** - Shows how to:

- Use GmodPathQuery to match paths with or without locations
- Use MetadataTagsQuery to filter by metadata tags
- Combine path and tag queries with LocalIdQuery
- Use node and path configurations for flexible matching
- Filter sensor data in practical scenarios

```bash
dotnet run --project samples/LocalIdQuery
```

### 🌳 [`GmodSubset`](GmodSubset/Program.cs)

**Asset Model / Digital Twin structure** - Demonstrates:

- Building an asset model tree from GmodPaths
- Creating hierarchical visualizations
- Extracting unique nodes across multiple paths
- Generating LocalIds for sensor data

```bash
dotnet run --project samples/GmodSubset
```

### 📡 [`SensorDataFlow`](SensorDataFlow/Program.cs)

**Proprietary to ISO19848 transformation** - Shows how to:

- Create DataChannelList for sensor mappings
- Transform proprietary sensor readings to ISO19848 format
- Group channels by update_cycle (ISO19848 recommendation)
- Build TimeSeriesDataPackage with TabularData
- Handle unknown/invalid sensor IDs

```bash
dotnet run --project samples/SensorDataFlow
```

### 📋 [`Iso19848Json`](Iso19848Json/Program.cs)

**ISO19848 JSON serialization** - Demonstrates:

- Loading DataChannelList from JSON
- Loading TimeSeriesData from JSON
- Converting between JSON DTOs and domain models
- Looking up data channels by ShortId and LocalId
- JSON roundtrip serialization

```bash
dotnet run --project samples/Iso19848Json
```

## 🚀 Running the Samples

### Prerequisites

Make sure you have the .NET SDK installed and the Vista SDK project built:

```bash
cd csharp
dotnet build
```

### Running Individual Samples

```bash
# Run from the csharp directory
dotnet run --project samples/BasicUsage
dotnet run --project samples/CodebooksExample
dotnet run --project samples/GmodOperations
dotnet run --project samples/AdvancedLocalIds
dotnet run --project samples/LocalIdQuery
dotnet run --project samples/GmodSubset
dotnet run --project samples/SensorDataFlow
dotnet run --project samples/Iso19848Json
```

## 📚 What You'll Learn

### From `BasicUsage`:

- How to initialize the VIS system
- Basic GMOD path parsing
- Creating simple Local IDs
- Working with metadata tags

### From `CodebooksExample`:

- Understanding different codebook types
- Creating and validating tags
- Working with custom values
- Position validation rules

### From `GmodOperations`:

- GMOD structure and navigation
- Node properties and relationships
- Version-specific operations
- Path validation techniques
- Traversal patterns

### From `AdvancedLocalIds`:

- Complex Local ID construction
- Error handling strategies
- Builder pattern best practices
- Verbose mode usage

### From `LocalIdQuery`:

- GmodPathQuery for path matching
- MetadataTagsQuery for tag filtering
- LocalIdQuery for combined queries
- Practical filtering scenarios

### From `GmodSubset`:

- Building asset models from GmodPaths
- Tree structure creation and navigation
- Path hierarchy extraction
- Digital twin foundations

### From `SensorDataFlow`:

- Transforming proprietary sensor data to ISO19848
- Creating DataChannelList for sensor mappings
- Grouping channels by update_cycle
- Building TimeSeriesDataPackage with TabularData

### From `Iso19848Json`:

- JSON serialization/deserialization of ISO19848 data
- Converting between JSON DTOs and domain models
- Looking up data channels by ShortId and LocalId
- Validating JSON roundtrips

## 🔧 Customizing the Examples

Feel free to modify these samples to experiment with:

- **Different VIS versions**: Change `VisVersion.v3_4a` to other versions
- **Different paths**: Try your own GMOD paths
- **Custom metadata**: Create your own tag combinations
- **Error scenarios**: Test with invalid inputs to see error handling

## 🛠️ Common Patterns

All samples follow these patterns:

### 1. VIS Initialization

```csharp
var version = VisVersion.v3_4a;
var gmod = VIS.Instance.GetGmod(version);
var codebooks = VIS.Instance.GetCodebooks(version);
var locations = VIS.Instance.GetLocations(version);
```

### 2. Error Handling

```csharp
if (gmod.TryParsePath(pathStr, out var path))
{
    Console.WriteLine($"✓ Success: {path}");
}
else
{
    Console.WriteLine($"✗ Error: Invalid path");
}
```

### 3. Builder Pattern

```csharp
var quantityTag = codebooks.CreateTag(CodebookName.Quantity, "temperature");

var localId = LocalIdBuilder
    .Create(version)
    .WithPrimaryItem(path)
    .WithMetadataTag(quantityTag)
    .Build();
```

## 🔗 Next Steps

After running these samples:

1. **Explore the API**: Look at the source code in `src/Vista.SDK/`
2. **Run the tests**: Execute `dotnet test` to see comprehensive examples
3. **Build your own**: Use these patterns in your own applications

## 📄 License

These samples are part of the Vista SDK and are licensed under the MIT License.
