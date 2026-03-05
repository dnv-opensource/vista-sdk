[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-csharp.yml?branch=main&label=C%23)](https://github.com/dnv-opensource/vista-sdk/actions)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-js.yml?branch=main&label=JS)](https://github.com/dnv-opensource/vista-sdk/actions)
![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-python.yml?branch=main&label=Python)
[![GitHub](https://img.shields.io/github/license/dnv-opensource/vista-sdk?style=flat-square)](https://github.com/dnv-opensource/vista-sdk/blob/main/LICENSE)<br/>
[![SDK NuGet current](https://img.shields.io/nuget/v/DNV.Vista.SDK?label=NuGet%20DNV.Vista.SDK)](https://www.nuget.org/packages/DNV.Vista.SDK)
[![SDK NuGet prerelease](https://img.shields.io/nuget/vpre/DNV.Vista.SDK?label=NuGet%20DNV.Vista.SDK)](https://www.nuget.org/packages/DNV.Vista.SDK)<br/>
[![SDK NPM current](https://img.shields.io/npm/v/dnv-vista-sdk?label=NPM%20dnv-vista-sdk)](https://www.npmjs.com/package/dnv-vista-sdk)
[![SDK NPM preview](https://img.shields.io/npm/v/dnv-vista-sdk/preview?label=NPM%20dnv-vista-sdk%20preview)](https://www.npmjs.com/package/dnv-vista-sdk)<br/>
![PyPI current](https://img.shields.io/pypi/v/vista-sdk?label=PyPI%20vista-sdk)

## Vista SDK

The Vista team at DNV are working on tooling related to

- DNV Vessel Information Structure (VIS)
- ISO 19847 - Ships and marine technology — Shipboard data servers to share field data at sea
- ISO 19848 - Ships and marine technology — Standard data for shipboard machinery and equipment

In this repository we codify the rules and principles of VIS and related ISO-standards to enable and support
users and implementers of the standards.

Our plan is to develop SDKs for some of the most common platforms. We are starting with .NET, Python and JavaScript.
We will be developing these SDKs as open source projects. Feel free to provide input, request changes or make contributions by creating issues in this repository.

For general documentation relating to VIS and related standards, see [docs.vista.dnv.com](https://docs.vista.dnv.com).

## 🎯 What Problem Does This Solve?

[ISO 19848](https://www.iso.org/standard/74324.html) defines a standard for identifying and describing sensor and event data channels on ships. It enables interoperability between different systems and stakeholders by providing:

- **Standardized naming conventions** - A structured way to identify what each data channel measures (e.g., "main engine cylinder 1 exhaust gas temperature at outlet")
- **Standardized data formats** - Common structures for exchanging data channel metadata (DataChannelList) and measurements (TimeSeriesData)

The **Vista SDK** implements the concepts defined in ISO 19848 and its Annex C ("dnv-v2" naming rule), providing tools to:

- **Parse standardized identifiers** - Validate and extract structured information from Local IDs and Universal IDs received from other systems
- **Build standardized identifiers** - Construct ISO 19848-compliant Local IDs and Universal IDs for your data channels
- **Navigate the GMOD** - Traverse the Generic Product Model hierarchy that describes ship functions and components
- **Work with Codebooks** - Use the standardized metadata vocabularies (quantity, content, position, state, etc.)
- **Exchange data** - Serialize and deserialize ISO 19848 DataChannelList and TimeSeriesData packages

## 📋 VIS Versions

The SDK supports multiple versions of VIS. Each version includes updated GMOD structures and codebooks.

**Versioning notes:**

- The SDK includes only the "a" (annual) releases to avoid excessive versioning churn
- A new VIS "a" release is typically published once a year (June)
- Generally, using the latest version is recommended for new projects

| Version   | Enum Value        |
| --------- | ----------------- |
| VIS 3.4a  | `v3_4a`           |
| VIS 3.5a  | `v3_5a`           |
| ...       |                   |
| VIS 3.9a  | `v3_9a`           |
| VIS 3.10a | `v3_10a` (Latest) |

### Tackling Different VIS Versions

In real-world scenarios, you may receive data using older VIS versions that need to be processed alongside newer versions. The SDK provides versioning utilities that enable on-the-fly upward conversion of GmodPaths and LocalIds between versions, allowing you to normalize data to a common version for consistent processing.

## 📦 SDK Implementations

We provide SDKs for the most common platforms. Each implementation has its own detailed README with installation instructions, quick start guides, and API documentation:

| Platform                  | Package                                                                                                            | Implementation     |
| ------------------------- | ------------------------------------------------------------------------------------------------------------------ | ------------------ |
| **C#/.NET**               | [![NuGet](https://img.shields.io/nuget/v/DNV.Vista.SDK?label=NuGet)](https://www.nuget.org/packages/DNV.Vista.SDK) | [csharp/](csharp/) |
| **JavaScript/TypeScript** | [![NPM](https://img.shields.io/npm/v/dnv-vista-sdk?label=NPM)](https://www.npmjs.com/package/dnv-vista-sdk)        | [js/](js/)         |
| **Python**                | [![PyPI](https://img.shields.io/pypi/v/vista-sdk?label=PyPI)](https://pypi.org/project/vista-sdk/)                 | [python/](python/) |

### Quick Installation

```bash
# C#/.NET
dotnet add package DNV.Vista.SDK

# JavaScript/TypeScript
npm install dnv-vista-sdk

# Python
pip install vista-sdk
```

## 📁 Repository Structure

```
📦vista-sdk
 ┣ 📂resources          # VIS data files (GMOD, Codebooks)
 ┃ ┣ 📜codebooks-vis-*.json.gz
 ┃ ┣ 📜gmod-vis-*.json.gz
 ┃ ┗ 📜gmod-vis-versioning-*.json.gz
 ┣ 📂schemas            # JSON/XML schemas for ISO standards
 ┃ ┣ 📂json
 ┃ ┃ ┣ 📜DataChannelList.schema.json
 ┃ ┃ ┗ 📜TimeSeriesData.schema.json
 ┃ ┗ 📂xml
 ┣ 📂csharp             # C#/.NET SDK
 ┣ 📂js                 # JavaScript/TypeScript SDK
 ┣ 📂python             # Python SDK
 ┣ 📂testdata           # Shared test data
 ┣ 📜LICENSE
 ┗ 📜README.md
```

## 🧩 SDK Outline

This section outlines the various components and modules in our SDKs.

| Component                  | Description                                       | C#  | JS  | Python |
| -------------------------- | ------------------------------------------------- | :-: | :-: | :----: |
| **Gmod**                   | Generic product model (traversal, node lookup)    |  ✓  |  ✓  |   ✓    |
| **GmodPath**               | Path parsing, full-path support, serialization    |  ✓  |  ✓  |   ✓    |
| **Pmod**\*                 | Asset-specific product model                      |     |  ✓  |        |
| **Codebooks**              | Metadata tag lookup and validation                |  ✓  |  ✓  |   ✓    |
| **Locations**              | Physical positioning and location parsing         |  ✓  |  ✓  |   ✓    |
| **Local ID**               | Parsing, building, and serialization              |  ✓  |  ✓  |   ✓    |
| **Universal ID**           | Parsing, building, and serialization              |  ✓  |  ✓  |   ✓    |
| **GmodPath Versioning**    | Path/node/LocalId conversion between VIS versions |  ✓  |  ✓  |   ✓    |
| **ImoNumber**              | IMO number validation                             |  ✓  |  ✓  |   ✓    |
| **LocalIdQuery**           | Programmatic filtering of Local IDs               |  ✓  |  ✓  |   ✓    |
| **GmodPathQuery**          | Querying and filtering GmodPaths                  |  ✓  |  ✓  |   ✓    |
| **MetadataTagsQuery**      | Querying and filtering metadata tags              |  ✓  |  ✓  |   ✓    |
| **DataChannelList (JSON)** | ISO 19848 data channel definitions                |  ✓  |  ✓  |   ✓    |
| **TimeSeriesData (JSON)**  | ISO 19848 timeseries and event data               |  ✓  |  ✓  |   ✓    |
| **MQTT Transport**         | MQTT-specific Local ID support                    |  ✓  |     |   ✓    |

> \* The naming "Pmod" (Product Model) is inspired by DNV class terminology where it refers to asset-specific class information model. In the Vista SDK, Pmod represents a subset of Gmod built from GmodPaths or LocalIds, which can originate from any source—not limited to class-specific data.

For more information on these concepts, check out [docs.vista.dnv.com](https://docs.vista.dnv.com).

## 🔧 API Patterns

All SDKs follow consistent design patterns:

### Immutability

Domain models are immutable. Builder APIs construct new instances while preserving unmodified data.

### Builder Pattern

```csharp
// C# Example
builder = Create(intro)
    .WithSomeValue(value)         // Throws on invalid input
    .TryWithOtherValue(value)     // Only apply valid changes
    .WithoutThirdValue()          // Removes component
```

- **`With*`** - Use when operation is expected to succeed; throws on invalid input
- **`TryWith*`** - Use for safe operations; only applies valid changes
- **`Without*`** - Removes specific components from the builder

## 📊 ISO 19848 Transport Packages

Part of the ISO 19848/19847 standards is the definition of data structures used for communicating and sharing sensor data:

- **DataChannelList** - Defines the data channels available on a ship
- **TimeSeriesData** - Contains the actual measurement data and events

Note that while compression isn't explicitly mentioned in these standards, the standards don't prohibit use of compression when implementing them, as long as the data structures remain the same.

### Schemas

The ISO 19848/19847 standards define XML and [JSON schemas](https://json-schema.org/) for transport packages:

- [DataChannelList.schema.json](schemas/json/DataChannelList.schema.json)
- [TimeSeriesData.schema.json](schemas/json/TimeSeriesData.schema.json)

## 📈 Benchmarks

We are developing benchmarks to track the performance characteristics of the libraries. See the respective implementations for details on methodology and results, including comparisons of JSON vs binary encodings and compression options.

## 🤝 Contributing

We welcome contributions! Feel free to:

- Create issues for bugs or feature requests
- Submit pull requests
- Provide feedback on API design

See the individual SDK READMEs for development setup instructions.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🔗 Links

- **Documentation**: [docs.vista.dnv.com](https://docs.vista.dnv.com)
- **GitHub**: [dnv-opensource/vista-sdk](https://github.com/dnv-opensource/vista-sdk)
- **Issues**: [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)
