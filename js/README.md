# Vista SDK - JavaScript/TypeScript Implementation

[![NPM current](https://img.shields.io/npm/v/dnv-vista-sdk?label=NPM%20dnv-vista-sdk)](https://www.npmjs.com/package/dnv-vista-sdk)
[![NPM preview](https://img.shields.io/npm/v/dnv-vista-sdk/preview?label=NPM%20dnv-vista-sdk%20preview)](https://www.npmjs.com/package/dnv-vista-sdk)
[![JS Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-js.yml?branch=main&label=Build)](https://github.com/dnv-opensource/vista-sdk/actions)
[![GitHub](https://img.shields.io/github/license/dnv-opensource/vista-sdk?style=flat-square)](https://github.com/dnv-opensource/vista-sdk/blob/main/LICENSE)

The JavaScript/TypeScript implementation of the Vista SDK. For an overview of the SDK and its concepts, see the [main README](../README.md).

## 📦 Installation

### Prerequisites

- **Node.js 20** or later

### NPM Installation

```bash
npm install dnv-vista-sdk
```

### Additional Packages

```bash
# Experimental extensions (PMS Local IDs, transport models)
npm install dnv-vista-sdk-experimental
```

## 🚀 Quick Start

> 💡 For more complete examples, see the [samples](samples/) directory and [samples README](samples/README.md) for detailed descriptions of each example.

### Basic Usage

```typescript
import {
    VIS,
    VisVersion,
    LocalIdBuilder,
    LocalId,
    CodebookName,
} from "dnv-vista-sdk";

// Initialize VIS instance — getVIS loads gmod, codebooks, and locations together
const vis = VIS.instance;
const version = VisVersion.v3_4a;
const { gmod, codebooks, locations } = await vis.getVIS(version);

// Get a GMOD node
const node = gmod.getNode("411.1");
console.log(`Node code: ${node.code}`);
console.log(`Node common name: ${node.metadata.commonName}`);

// Parse a GMOD path (requires locations)
const path = gmod.parsePath("411.1/C101.31-2", locations);
console.log(`Parsed path: ${path}`);
console.log(`Node: ${path.node.metadata.commonName}`);

// Build a Local ID
const quantityTag = codebooks
    .getCodebook(CodebookName.Quantity)
    .createTag("temperature");
const contentTag = codebooks
    .getCodebook(CodebookName.Content)
    .createTag("exhaust.gas");
const positionTag = codebooks
    .getCodebook(CodebookName.Position)
    .createTag("inlet");

const localId = LocalIdBuilder.create(version)
    .withPrimaryItem(path)
    .withMetadataTag(quantityTag)
    .withMetadataTag(contentTag)
    .withMetadataTag(positionTag)
    .build();

console.log(`Local ID: ${localId}`);

// Parse an existing Local ID (sync requires gmod, codebooks, locations)
const localIdStr = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature";
const parsed = LocalId.parse(localIdStr, gmod, codebooks, locations);
console.log(`Parsed: ${parsed}`);
console.log(`Primary item: ${parsed.primaryItem}`);

// Or use the async variant (auto-resolves dependencies)
const parsed2 = await LocalId.parseAsync(localIdStr);
console.log(`Parsed (async): ${parsed2}`);
```

### Working with Codebooks

```typescript
import { VIS, VisVersion, CodebookName } from "dnv-vista-sdk";

const codebooks = await VIS.instance.getCodebooks(VisVersion.v3_4a);

// Get a specific codebook
const positionCodebook = codebooks.getCodebook(CodebookName.Position);

// Create metadata tags
const positionTag = positionCodebook.createTag("centre");
const quantityTag = codebooks
    .getCodebook(CodebookName.Quantity)
    .createTag("temperature");

console.log(`Position tag: ${positionTag}`);
console.log(`Quantity tag: ${quantityTag}`);

// Check if values are valid
console.log(
    `Is 'centre' valid? ${positionCodebook.hasStandardValue("centre")}`,
);
```

### GMOD Path Operations

```typescript
import { VIS, VisVersion } from "dnv-vista-sdk";

const { gmod, locations } = await VIS.instance.getVIS(VisVersion.v3_4a);

// Parse a path (requires locations)
const path = gmod.parsePath("411.1/C101.31-2", locations);

// Get path information
console.log(`Path depth: ${path.length}`);
console.log(`End node: ${path.node}`);
console.log(`Short path: ${path}`);
console.log(`Full path: ${path.toFullPathString()}`);

// Traverse the full path (getFullPath returns GmodNode[])
const fullPath = path.getFullPath();
for (let depth = 0; depth < fullPath.length; depth++) {
    const node = fullPath[depth];
    console.log(`Depth ${depth}: ${node.code} - ${node.metadata.commonName}`);
}
```

### Version Conversion

```typescript
import { VIS, VisVersion } from "dnv-vista-sdk";

const sourceVersion = VisVersion.v3_4a;
const targetVersion = VisVersion.v3_5a;

const { gmod, locations } = await VIS.instance.getVIS(sourceVersion);
const path = gmod.parsePath("411.1/C101.72/I101", locations);

try {
    const newPath = await VIS.instance.convertPath(
        sourceVersion,
        path,
        targetVersion,
    );
    console.log(`Converted: ${path} -> ${newPath}`);
} catch (e) {
    console.log(`Conversion failed: ${e}`);
}
```

## 📚 Core Components

For a detailed overview of VIS concepts (GMOD, Codebooks, Locations, etc.), see the [main README](../README.md).

### VIS (Vessel Information Structure)

The main entry point for accessing VIS data via `VIS.instance`. All data-loading methods are async.

### Local ID Builder

Construct standardized local identifiers:

```typescript
import { VIS, VisVersion, LocalIdBuilder, CodebookName } from "dnv-vista-sdk";

const version = VisVersion.v3_4a;
const { gmod, codebooks, locations } = await VIS.instance.getVIS(version);

const path = gmod.parsePath("411.1/C101.31", locations);
const quantityTag = codebooks
    .getCodebook(CodebookName.Quantity)
    .createTag("temperature");
const contentTag = codebooks
    .getCodebook(CodebookName.Content)
    .createTag("cooling.water");
const stateTag = codebooks.getCodebook(CodebookName.State).createTag("high");

const localId = LocalIdBuilder.create(version)
    .withPrimaryItem(path)
    .withMetadataTag(quantityTag)
    .withMetadataTag(contentTag)
    .withMetadataTag(stateTag)
    .build();
```

### Builder Pattern Support

The SDK follows a fluent builder pattern:

- **`with*()`** - Add or set values (throws on invalid input)
- **`tryWith*()`** - Only applies valid changes; returns builder for chaining
- **`without*()`** - Remove specific components

```typescript
// Using with - throws on failure
const builder = LocalIdBuilder.create(version)
    .withPrimaryItem(path)
    .withMetadataTag(quantityTag);

// Using tryWith - silently ignores invalid input, allows chaining
builder.tryWithMetadataTag(contentTag).tryWithMetadataTag(stateTag);

// Using without - removes a metadata tag by codebook name
builder.withoutMetadataTag(CodebookName.State);
```

## 🔧 Advanced Usage

### Parsing Local IDs

```typescript
import {
    VIS,
    VisVersion,
    LocalId,
    LocalIdBuilder,
    LocalIdParsingErrorBuilder,
    CodebookName,
    ParsingState,
} from "dnv-vista-sdk";

const { gmod, codebooks, locations } = await VIS.instance.getVIS(
    VisVersion.v3_4a,
);

const localIdStr = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature";

// Async parsing (auto-resolves dependencies)
const parsed = await LocalId.parseAsync(localIdStr);
console.log(`Parsed Local ID: ${parsed}`);
console.log(`Primary item: ${parsed.primaryItem}`);
console.log(`Quantity: ${parsed.getMetadataTag(CodebookName.Quantity)}`);

// Sync parsing with error handling
const errorBuilder = LocalIdParsingErrorBuilder.create();
try {
    const result = LocalId.parse(
        localIdStr,
        gmod,
        codebooks,
        locations,
        errorBuilder,
    );
    console.log(`Parsed: ${result}`);
} catch {
    for (const error of errorBuilder.errors) {
        console.log(`Error: ${ParsingState[error.type]} - ${error.message}`);
    }
}
```

### ISO 19848 Transport

```typescript
import {
    LocalId,
    DataChannelList,
    ShipId,
    Version,
    JSONExtensions,
    JSONSerializer,
} from "dnv-vista-sdk";

// Create a DataChannelList using plain object literals
const localId = await LocalId.parseAsync(
    "/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature",
);

const dcList: DataChannelList.DataChannelListPackage = {
    package: {
        header: {
            shipId: ShipId.parse("IMO1234567"),
            dataChannelListId: {
                id: "MySystem",
                version: Version.parse("1.0"),
                timestamp: new Date("2024-01-01T00:00:00Z"),
            },
            author: "Vista SDK Sample",
            dateCreated: new Date("2024-01-01T00:00:00Z"),
        },
        dataChannelList: {
            dataChannel: [
                {
                    dataChannelId: {
                        localId: localId,
                        shortId: "TEMP001",
                    },
                    property: {
                        dataChannelType: { type: "Inst", updateCycle: 1.0 },
                        format: { type: "Decimal" },
                        unit: {
                            unitSymbol: "°C",
                            quantityName: "Temperature",
                        },
                        name: "Temperature Sensor",
                    },
                },
            ],
        },
    },
};

// Serialize to JSON
const dto = JSONExtensions.DataChannelList.toJsonDto(dcList);
const json = JSONSerializer.serializeDataChannelList(dto);

// Deserialize from JSON
const loadedDto = JSONSerializer.deserializeDataChannelList(json);
const domainModel =
    await JSONExtensions.DataChannelList.toDomainModel(loadedDto);
```

### GMOD Traversal

```typescript
import {
    VIS,
    VisVersion,
    GmodNode,
    TraversalHandlerResult,
} from "dnv-vista-sdk";

const gmod = await VIS.instance.getGmod(VisVersion.v3_4a);

// Traverse the entire GMOD tree
gmod.traverse((parents: GmodNode[], node: GmodNode) => {
    const depth = parents.length;
    const indent = " ".repeat(depth * 2);
    console.log(`${indent}${node.code}: ${node.metadata.commonName}`);
    return TraversalHandlerResult.Continue;
});

// Traverse from a specific node
const startNode = gmod.getNode("400");
gmod.traverse(
    (parents: GmodNode[], node: GmodNode) => {
        console.log(`${node.code}`);
        return TraversalHandlerResult.Continue;
    },
    { rootNode: startNode },
);
```

## 📦 Packages

| Package                                                          | Description                                                            |
| ---------------------------------------------------------------- | ---------------------------------------------------------------------- |
| [`dnv-vista-sdk`](packages/vista-sdk/)                           | Core SDK library — GMOD, Codebooks, LocalId, ISO 19848 transport       |
| [`dnv-vista-sdk-experimental`](packages/vista-sdk-experimental/) | Experimental extensions — PMS Local IDs, experimental transport models |

## 🧪 Testing

### Running Tests

```bash
# Run all tests (from js/ root)
npm test

# Run tests for a specific package
npm test --workspace=packages/vista-sdk
npm test --workspace=packages/vista-sdk-experimental
```

### Running Samples

```bash
# Compile samples
cd samples
npx tsc

# Run a specific sample
node dist/BasicUsage.js
node dist/CodebooksExample.js
node dist/GmodOperations.js
node dist/AdvancedLocalIds.js
node dist/LocalIdQuery.js
node dist/GmodSubset.js
node dist/SensorDataFlow.js
node dist/Iso19848Json.js
```

See the [samples README](samples/README.md) for detailed descriptions of each sample.

## 🛠️ Development

### Setting up Development Environment

```bash
# Clone the repository
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/js

# Install dependencies
npm install

# Build all packages
npm run build

# Run tests
npm test

# Run a sample
cd samples && npx tsc && node dist/BasicUsage.js
```

### Project Structure

```
js/
├── packages/
│   ├── vista-sdk/                     # Core SDK library
│   │   ├── lib/                       # Source code
│   │   ├── tests/                     # Unit tests
│   │   └── dist/                      # Build output
│   └── vista-sdk-experimental/        # Experimental extensions
│       ├── lib/                       # Source code
│       ├── tests/                     # Unit tests
│       └── dist/                      # Build output
├── samples/                           # Usage examples
│   ├── BasicUsage.ts
│   ├── CodebooksExample.ts
│   ├── GmodOperations.ts
│   ├── AdvancedLocalIds.ts
│   ├── LocalIdQuery.ts
│   ├── GmodSubset.ts
│   ├── SensorDataFlow.ts
│   └── Iso19848Json.ts
├── package.json                       # Workspace root
└── jest.config.js                     # Test configuration
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Run the test suite (`npm test`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🔗 Links

- **Documentation**: [docs.vista.dnv.com](https://docs.vista.dnv.com)
- **GitHub Repository**: [vista-sdk](https://github.com/dnv-opensource/vista-sdk)
- **NPM Package**: [dnv-vista-sdk](https://www.npmjs.com/package/dnv-vista-sdk)
- **Issues**: [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)

## 📞 Support

For questions and support:

- Create an issue on [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)
- Check the [documentation](https://docs.vista.dnv.com)
- Review the [samples](samples/) directory for examples
