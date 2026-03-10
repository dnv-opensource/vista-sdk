# Vista SDK JS/TS Samples

This directory contains practical examples demonstrating how to use the Vista SDK JavaScript/TypeScript implementation. Each sample focuses on specific aspects of the SDK and can be run independently.

## 📁 Sample Files

### 🚀 [`BasicUsage.ts`](BasicUsage.ts)

**Perfect starting point** - Demonstrates fundamental operations:

- Initializing VIS and loading data
- Parsing GMOD paths
- Working with codebooks and metadata tags
- Building and parsing Local IDs

### 🔧 [`CodebooksExample.ts`](CodebooksExample.ts)

**Deep dive into codebooks** - Shows how to:

- Explore available codebooks and their content
- Create standard and custom metadata tags
- Validate tag values
- Handle position validation special cases

### 🏗️ [`GmodOperations.ts`](GmodOperations.ts)

**GMOD (Generic Product Model) operations** - Covers:

- Parsing and analyzing GMOD paths
- Exploring node hierarchies and metadata
- Working with different VIS versions
- Path validation and error handling
- Version conversion between VIS versions
- GMOD traversal patterns (full traversal, early stop, from specific node)

### 🎯 [`AdvancedLocalIds.ts`](AdvancedLocalIds.ts)

**Advanced Local ID operations** - Demonstrates:

- Building complex Local IDs with multiple components (primary + secondary items)
- Error handling and validation with `LocalIdParsingErrorBuilder`
- Working with custom tags
- Builder pattern variations and safe operations
- Verbose mode usage

### 🔍 [`LocalIdQuery.ts`](LocalIdQuery.ts)

**Local ID Query operations** - Shows how to:

- Use `GmodPathQueryBuilder` to match paths with or without locations
- Use `MetadataTagsQueryBuilder` to filter by metadata tags
- Combine path and tag queries with `LocalIdQueryBuilder`
- Use `withAnyNodeBefore` / `withAnyNodeAfter` for flexible matching
- Filter sensor data in practical scenarios

### 🌳 [`GmodSubset.ts`](GmodSubset.ts)

**Asset Model / Digital Twin structure** - Demonstrates:

- Building an asset model tree from GmodPaths
- Creating hierarchical visualizations
- Extracting equipment from LocalIds
- JSON export for visualization (e.g. D3.js)
- Looking up nodes by code across the model

### 📡 [`SensorDataFlow.ts`](SensorDataFlow.ts)

**Proprietary to ISO19848 transformation** - Shows how to:

- Create DataChannelList for sensor mappings
- Transform proprietary sensor readings to ISO19848 format
- Group channels by update_cycle (ISO19848 recommendation)
- Build TimeSeriesDataPackage with TabularData
- Handle unknown/invalid sensor IDs
- Serialize to standard JSON format

### 📋 [`Iso19848Json.ts`](Iso19848Json.ts)

**ISO19848 JSON serialization** - Demonstrates:

- Creating DataChannelList and TimeSeriesData domain models
- Converting between JSON DTOs and domain models
- Looking up data channels by ShortId and LocalId
- JSON roundtrip serialization/deserialization

## 🚀 Running the Samples

### Prerequisites

Make sure the SDK is built:

```bash
cd js
npm install
npm run build
```

### Running Individual Samples

From the `js/samples` directory:

```bash
# Compile all samples
npx tsc

# Run individual samples
node dist/BasicUsage.js
node dist/CodebooksExample.js
node dist/GmodOperations.js
node dist/AdvancedLocalIds.js
node dist/LocalIdQuery.js
node dist/GmodSubset.js
node dist/SensorDataFlow.js
node dist/Iso19848Json.js
```

Or compile and run in one step:

```bash
npx tsc && node dist/BasicUsage.js
```

## 📚 What You'll Learn

### From `BasicUsage`:

- How to initialize the VIS system asynchronously
- Basic GMOD path parsing with locations
- Creating simple Local IDs with metadata tags
- Parsing existing Local ID strings

### From `CodebooksExample`:

- Understanding different codebook types (Quantity, Position, State, etc.)
- Creating and validating tags
- Working with custom values (prefixed with `~`)
- Exploring codebook standard values and groups

### From `GmodOperations`:

- GMOD structure and navigation
- Node properties and relationships
- Version conversion between VIS versions
- Path validation techniques
- Traversal patterns (count nodes, find first leaf, traverse subtree)

### From `AdvancedLocalIds`:

- Complex Local ID construction with primary and secondary items
- Error handling with `LocalIdParsingErrorBuilder`
- Builder pattern best practices
- Verbose mode for including common names in Local IDs

### From `LocalIdQuery`:

- `GmodPathQuery` for path matching (exact, without locations, by nodes)
- `MetadataTagsQuery` for tag filtering (single, multiple, exact match)
- `LocalIdQuery` for combined path + tag queries
- `withAnyNodeBefore` / `withAnyNodeAfter` for wildcard-style matching
- Practical filtering scenarios for sensor data

### From `GmodSubset`:

- Building asset models from GmodPaths
- Tree structure creation and navigation
- Extracting paths from LocalIds
- JSON export for visualization tools

### From `SensorDataFlow`:

- End-to-end sensor data transformation pipeline
- DataChannelList as the mapping between system IDs and LocalIds
- Grouping channels by update_cycle for ISO19848 compliance
- Building TabularData with proper timestamp alignment

### From `Iso19848Json`:

- JSON serialization/deserialization of ISO19848 data
- Converting between JSON DTOs and domain models using `JSONExtensions`
- Looking up data channels by ShortId and LocalId
- Validating JSON roundtrips

## 🛠️ Common Patterns

All samples follow these patterns:

### 1. VIS Initialization (async)

```typescript
import { VIS, VisVersion } from "dnv-vista-sdk";

const version = VisVersion.v3_4a;
const vis = VIS.instance;
const { gmod, codebooks, locations } = await vis.getVIS(version);
```

### 2. Error Handling

```typescript
const path = gmod.tryParsePath(pathStr, locations);
if (path) {
    console.log(`✓ Success: ${path}`);
} else {
    console.log(`✗ Error: Invalid path`);
}
```

### 3. Builder Pattern

```typescript
import { LocalIdBuilder, CodebookName } from "dnv-vista-sdk";

const quantityTag = codebooks.createTag(CodebookName.Quantity, "temperature");

const localId = LocalIdBuilder.create(version)
    .withPrimaryItem(path)
    .withMetadataTag(quantityTag)
    .build();
```

### 4. Async Parsing

```typescript
import { LocalId } from "dnv-vista-sdk";

// Parse with auto-loading VIS data
const localId = await LocalId.parseAsync(localIdString);

// Or parse with pre-loaded data
const localId = LocalId.parse(localIdString, gmod, codebooks, locations);
```

## 🔗 JS/TS vs C# API Differences

| Feature         | C#                            | JS/TS                                                                         |
| --------------- | ----------------------------- | ----------------------------------------------------------------------------- |
| VIS Loading     | Synchronous                   | Async (`await vis.getVIS(...)`)                                               |
| Path Parsing    | `gmod.TryParsePath(str, ...)` | `gmod.tryParsePath(str, locations)`                                           |
| Codebook Access | `codebooks[CodebookName.X]`   | `codebooks.getCodebook(CodebookName.X)`                                       |
| Tag Creation    | `codebook.CreateTag(value)`   | `codebook.createTag(value)`                                                   |
| LocalId Parsing | `LocalId.TryParse(str, ...)`  | `LocalId.parse(str, gmod, codebooks, locations)` or `LocalId.parseAsync(str)` |
| Query Match     | `query.Match(localId)`        | `await query.match(localId)` (async for path queries)                         |

## 🔗 Next Steps

After running these samples:

1. **Explore the API**: Look at the source code in `packages/vista-sdk/lib/`
2. **Run the tests**: Execute `npm test` at the `js/` level
3. **Build your own**: Use these patterns in your own applications

## 📄 License

These samples are part of the Vista SDK and are licensed under the MIT License.
