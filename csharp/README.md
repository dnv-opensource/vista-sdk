# Vista SDK - C# Implementation

[![NuGet current](https://img.shields.io/nuget/v/DNV.Vista.SDK?label=NuGet%20DNV.Vista.SDK)](https://www.nuget.org/packages/DNV.Vista.SDK)
[![NuGet prerelease](https://img.shields.io/nuget/vpre/DNV.Vista.SDK?label=NuGet%20DNV.Vista.SDK%20preview)](https://www.nuget.org/packages/DNV.Vista.SDK)
[![C# Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-csharp.yml?branch=main&label=Build)](https://github.com/dnv-opensource/vista-sdk/actions)
[![GitHub](https://img.shields.io/github/license/dnv-opensource/vista-sdk?style=flat-square)](https://github.com/dnv-opensource/vista-sdk/blob/main/LICENSE)

The C# implementation of the Vista SDK. For an overview of the SDK and its concepts, see the [main README](../README.md).

## 📦 Installation

### Prerequisites

- **.NET 8.0** or later

### NuGet Installation

```bash
dotnet add package DNV.Vista.SDK
```

### Additional Packages

```bash
# JSON serialization support (System.Text.Json)
dotnet add package DNV.Vista.SDK.System.Text.Json

# MQTT integration
dotnet add package DNV.Vista.SDK.Mqtt
```

## 🚀 Quick Start

> 💡 For more complete examples, see the [samples](samples/) directory and [samples README](samples/README.md) for detailed descriptions of each example.

### Basic Usage

```csharp
using Vista.SDK;

// Initialize VIS instance
var version = VisVersion.v3_4a;
var gmod = VIS.Instance.GetGmod(version);
var codebooks = VIS.Instance.GetCodebooks(version);

// Get a GMOD node by lookup
var node = gmod["411.1"];
Console.WriteLine($"Node code: {node.Code}");
Console.WriteLine($"Node common name: {node.Metadata.CommonName}");

// Get a GMOD node by TryGet pattern
if(gmod.TryGetNode("411.1", out var node2))
{
    Console.WriteLine($"Node code: {node2.Code}");
    Console.WriteLine($"Node common name: {node2.Metadata.CommonName}");
}
else
{
    Console.WriteLine("Node not found");
}

// Parse a GMOD path
if (gmod.TryParsePath("411.1/C101.31-2", out var path))
{
    Console.WriteLine($"Parsed path: {path}");
    Console.WriteLine($"Node: {path.Node.Metadata.CommonName}");
}

// Build a Local ID
var quantityTag = codebooks[CodebookName.Quantity].CreateTag("temperature");
var contentTag = codebooks[CodebookName.Content].CreateTag("exhaust.gas");
var positionTag = codebooks[CodebookName.Position].CreateTag("inlet");

var localId = LocalIdBuilder.Create(version)
    .WithPrimaryItem(path)
    .WithMetadataTag(quantityTag)
    .WithMetadataTag(contentTag)
    .WithMetadataTag(positionTag)
    .Build();

Console.WriteLine($"Local ID: {localId}");

// Parse an existing Local ID
var localIdStr = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature";
if (LocalId.TryParse(localIdStr, out var errors, out var parsedLocalId))
{
    Console.WriteLine($"Parsed: {parsedLocalId}");
    Console.WriteLine($"Primary item: {parsedLocalId.PrimaryItem}");
}
```

### Working with Codebooks

```csharp
using Vista.SDK;

var codebooks = VIS.Instance.GetCodebooks(VisVersion.v3_4a);

// Get a specific codebook
var positionCodebook = codebooks[CodebookName.Position];

// Create metadata tags
var positionTag = positionCodebook.CreateTag("centre");
var quantityTag = codebooks[CodebookName.Quantity].CreateTag("temperature");

Console.WriteLine($"Position tag: {positionTag}");
Console.WriteLine($"Quantity tag: {quantityTag}");

// Check if values are valid
Console.WriteLine($"Is 'centre' valid? {positionCodebook.HasStandardValue("centre")}");
```

### GMOD Path Operations

```csharp
using Vista.SDK;

var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);

// Parse a path
var path = gmod.ParsePath("411.1/C101.31-2");

// Get path information
Console.WriteLine($"Path depth: {path.Length}");
Console.WriteLine($"End node: {path.Node}");
Console.WriteLine($"Short path: {path}");
Console.WriteLine($"Full path: {path.ToFullPathString()}");

// Traverse the path
foreach (var (depth, node) in path.GetFullPath())
{
    Console.WriteLine($"Depth {depth}: {node.Code} - {node.Metadata.CommonName}");
}
```

### Version Conversion

```csharp
using Vista.SDK;

var sourceVersion = VisVersion.v3_4a;
var targetVersion = VisVersion.v3_5a;

var gmod = VIS.Instance.GetGmod(sourceVersion);
var path = gmod.ParsePath("411.1/C101.72/I101");

try
{
    var newPath = VIS.Instance.ConvertPath(sourceVersion, path, targetVersion);
    Console.WriteLine($"Converted: {path} -> {newPath}");
}
catch (Exception e)
{
    Console.WriteLine($"Conversion failed: {e.Message}");
}
```

## 📚 Core Components

For a detailed overview of VIS concepts (GMOD, Codebooks, Locations, etc.), see the [main README](../README.md).

### VIS (Vessel Information Structure)

The main entry point for accessing VIS data via `VIS.Instance`.

### Local ID Builder

Construct standardized local identifiers:

```csharp
using Vista.SDK;

var version = VisVersion.v3_4a;
var gmod = VIS.Instance.GetGmod(version);
var codebooks = VIS.Instance.GetCodebooks(version);

var path = gmod.ParsePath("411.1/C101.31");
var quantityTag = codebooks[CodebookName.Quantity].CreateTag("temperature");
var contentTag = codebooks[CodebookName.Content].CreateTag("cooling.water");
var stateTag = codebooks[CodebookName.State].CreateTag("high");

var localId = LocalIdBuilder.Create(version)
    .WithPrimaryItem(path)
    .WithMetadataTag(quantityTag)
    .WithMetadataTag(contentTag)
    .WithMetadataTag(stateTag)
    .Build();
```

### Builder Pattern Support

The SDK follows a fluent builder pattern:

- **`With*()`** - Add or set values (throws on invalid input)
- **`TryWith*()`** - Only applies valid changes; returns builder for chaining
- **`Without*()`** - Remove specific components

```csharp
// Using With - throws on failure
var builder = LocalIdBuilder.Create(version)
    .WithPrimaryItem(path)
    .WithMetadataTag(quantityTag);

// Using TryWith - silently ignores invalid input, allows chaining
builder = builder
    .TryWithMetadataTag(contentTag)
    .TryWithMetadataTag(stateTag);

// Using Without - removes property
builder = builder.WithoutState();
```

## 🔧 Advanced Usage

### Parsing Local IDs

```csharp
using Vista.SDK;

var localIdStr = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature";

if (LocalId.TryParse(localIdStr, out var errors, out var localId))
{
    Console.WriteLine($"Parsed Local ID: {localId}");
    Console.WriteLine($"Primary item: {localId.PrimaryItem}");
    Console.WriteLine($"Quantity: {localId.Quantity}");
}
else
{
    Console.WriteLine($"Failed to parse: {errors}");
}
```

### ISO 19848 Transport

```csharp
using Vista.SDK;
using Vista.SDK.Transport.Json;
using Domain = Vista.SDK.Transport.DataChannel;

// Create a DataChannelList
var dcList = new Domain.DataChannelList();
var localId = LocalId.Parse("/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature");
var dcId = new Domain.DataChannelId { LocalId = localId, ShortId = "TEMP001" };
var property = new Domain.Property
{
    DataChannelType = new Domain.DataChannelType { Type = "Inst" },
    Name = "Temperature Sensor",
    Unit = new Domain.Unit { UnitSymbol = "°C" }
};
dcList.Add(new Domain.DataChannel { DataChannelId = dcId, Property = property });

// Serialize to JSON
var jsonDto = dcList.ToJsonDto();
var json = jsonDto.Serialize();

// Deserialize from JSON
var loadedDto = Serializer.DeserializeDataChannelList(json);
var domainModel = loadedDto.ToDomainModel();
```

### GMOD Traversal

```csharp
using Vista.SDK;

var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);

// Traverse the entire GMOD tree
gmod.Traverse((parents, node) =>
{
    var depth = parents.Length;
    var indent = new string(' ', depth * 2);
    Console.WriteLine($"{indent}{node.Code}: {node.Metadata.CommonName}");
    return TraversalHandlerResult.Continue;
});

// Traverse from a specific node
var rootNode = gmod["400"];
gmod.Traverse(rootNode, (parents, node) =>
{
    Console.WriteLine($"{node.Code}");
    return TraversalHandlerResult.Continue;
});
```

## 🧪 Testing

### Running Tests

```bash
# Run all tests
dotnet test

# Run specific test project
dotnet test test/Vista.SDK.Tests

# Run with coverage
dotnet test --collect:"XPlat Code Coverage"
```

### Running Samples

```bash
# Run a specific sample
dotnet run --project samples/BasicUsage

# Run all samples
dotnet run --project samples/BasicUsage
dotnet run --project samples/CodebooksExample
dotnet run --project samples/GmodOperations
dotnet run --project samples/AdvancedLocalIds
dotnet run --project samples/LocalIdQuery
dotnet run --project samples/GmodSubset
dotnet run --project samples/SensorDataFlow
dotnet run --project samples/Iso19848Json
```

## 📈 Performance

The C# implementation includes comprehensive benchmarks. See [benchmark/README.md](benchmark//README.md) for details.

### Running Benchmarks

```bash
cd benchmark/Vista.SDK.Benchmarks
dotnet run -c Release
```

## 🛠️ Development

### Setting up Development Environment

```bash
# Clone the repository
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/csharp

# Build the solution
dotnet build Vista.SDK.sln

# Run tests
dotnet test Vista.SDK.sln

# Run a sample
dotnet run --project samples/BasicUsage
```

### Project Structure

```
csharp/
├── src/
│   ├── Vista.SDK/                  # Core SDK library
│   ├── Vista.SDK.System.Text.Json/ # JSON serialization
│   ├── Vista.SDK.Mqtt/             # MQTT integration
│   └── Vista.SDK.SourceGenerator/  # Source generation
├── test/
│   ├── Vista.SDK.Tests/            # Unit tests
│   └── Vista.SDK.SmokeTests/       # Integration tests
├── samples/                        # Usage examples
└── benchmark/                      # Performance benchmarks
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Run the test suite (`dotnet test`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🔗 Links

- **Documentation**: [docs.vista.dnv.com](https://docs.vista.dnv.com)
- **GitHub Repository**: [vista-sdk](https://github.com/dnv-opensource/vista-sdk)
- **NuGet Package**: [DNV.Vista.SDK](https://www.nuget.org/packages/DNV.Vista.SDK)
- **Issues**: [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)

## 📞 Support

For questions and support:

- Create an issue on [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)
- Check the [documentation](https://docs.vista.dnv.com)
- Review the [samples](samples/) directory for examples
