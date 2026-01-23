# Vista SDK Python - API Reference

This document provides comprehensive API documentation for the Vista SDK Python implementation.

## 🏗️ Core Architecture

The Vista SDK is built around these main components:

- **[VIS](#vis-vessel-information-structure)** - Main entry point and data access
- **[GMOD](#gmod-general-model-of-data)** - Equipment and system hierarchy
- **[Local IDs](#local-ids)** - Standardized identifiers for data channels
- **[Codebooks](#codebooks)** - Metadata tag definitions and validation
- **[Locations](#locations)** - Physical positioning information

## 📚 Quick Reference

### Essential Imports

```python
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion
from vista_sdk.local_id_builder import LocalIdBuilder
from vista_sdk.gmod_path import GmodPath
from vista_sdk.local_id import LocalId
from vista_sdk.codebook_names import CodebookName
```

### Common Operations

```python
# Initialize VIS
vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)
codebooks = vis.get_codebooks(VisVersion.v3_4a)

# Parse GMOD path
path = GmodPath.parse(gmod, "411.1/C101.31-2")

# Create quantity tag
quantity_tag = codebooks.create_tag(CodebookName.Quantity, "temperature")

# Build Local ID
local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(path)
    .with_metadata_tag(quantity_tag)
    .build())
```

---

## VIS (Vessel Information Structure)

The main entry point for accessing all VIS data and services.

### Class: `VIS`

#### Methods

##### `get_gmod(vis_version: VisVersion) -> Gmod`

Retrieve GMOD data for a specific VIS version.

**Parameters:**

- `vis_version`: The VIS version to load

**Returns:** `Gmod` instance with cached data

**Example:**

```python
vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)
```

##### `get_codebooks(vis_version: VisVersion) -> Codebooks`

Retrieve codebooks for a specific VIS version.

**Parameters:**

- `vis_version`: The VIS version to load

**Returns:** `Codebooks` instance with all available codebooks

**Example:**

```python
codebooks = vis.get_codebooks(VisVersion.v3_4a)
position_book = codebooks[CodebookName.Position]
```

##### `get_locations(vis_version: VisVersion) -> Locations`

Retrieve location definitions for a specific VIS version.

**Parameters:**

- `vis_version`: The VIS version to load

**Returns:** `Locations` instance for positioning operations

##### `get_gmod_versioning(vis_version: VisVersion) -> GmodVersioning`

Get version conversion capabilities for GMOD paths.

**Parameters:**

- `vis_version`: The base VIS version for conversions

**Returns:** `GmodVersioning` instance for path conversion operations

**Example:**

```python
versioning = vis.get_gmod_versioning(VisVersion.v3_4a)
new_path = versioning.convert_path("411.1/C101.72", VisVersion.v3_5a)
```

---

## GMOD (Generic Product Model)

Represents the hierarchical structure of vessel equipment and systems.

### Class: `Gmod`

#### Methods

##### `get_node(code: str) -> GmodNode`

Retrieve a specific node by its code.

**Parameters:**

- `code`: The node code (e.g., "411.1", "C101.31")

**Returns:** `GmodNode` instance

**Raises:** `KeyError` if node not found

##### `all_nodes() -> Iterator[GmodNode]`

Iterate over all nodes in the GMOD.

**Returns:** Iterator of all `GmodNode` instances

##### `try_get_node(code: str) -> GmodNode | None`

Safely retrieve a node, returning None if not found.

**Parameters:**

- `code`: The node code to search for

**Returns:** `GmodNode` instance or `None`

### Class: `GmodNode`

#### Properties

- `code: str` - The node's unique code
- `name: str` - Human-readable name
- `common_name: str` - Common/display name
- `is_mappable: bool` - Whether the node can be used in Local IDs
- `children: Iterator[GmodNode]` - Child nodes

#### Methods

##### `get_child(code: str) -> GmodNode`

Get a direct child node by code.

**Parameters:**

- `code`: Child node code

**Returns:** Child `GmodNode`

**Raises:** `KeyError` if child not found

### Class: `GmodPath`

Represents a path through the GMOD hierarchy.

#### Static Methods

##### `parse(path: str, vis_version: VisVersion) -> GmodPath`

Parse a path string into a GmodPath.

**Parameters:**

- `path`: Path string (e.g., "411.1/C101.31-2")
- `vis_version`: VIS version for parsing context

**Returns:** `GmodPath` instance

**Raises:** `ValueError` for invalid paths

##### `try_parse(path: str, vis_version: VisVersion) -> GmodPath | None`

Safely parse a path, returning None if invalid.

#### Properties

- `node: GmodNode` - The terminal node of the path
- `vis_version: VisVersion` - Associated VIS version

#### Methods

##### `get_full_path() -> Iterator[tuple[int, GmodNode]]`

Iterate through the complete path from root to terminal node.

**Returns:** Iterator of (depth, node) tuples

---

## Local IDs

Standardized identifiers for data channels following ISO standards.

### Class: `LocalIdBuilder`

Builder for constructing Local IDs using the fluent interface pattern.

#### Static Methods

##### `create(vis_version: VisVersion) -> LocalIdBuilder`

Create a new builder instance.

**Parameters:**

- `vis_version`: VIS version for the Local ID

**Returns:** New `LocalIdBuilder` instance

##### `parse(local_id: str, gmod: Gmod, codebooks: Codebooks, locations: Locations) -> LocalId`

Parse a Local ID string.

**Parameters:**

- `local_id`: Local ID string to parse
- `gmod`: GMOD instance for path validation
- `codebooks`: Codebooks for tag validation
- `locations`: Locations for position validation

**Returns:** Parsed `LocalId` instance

**Raises:** `ValueError` for invalid Local IDs

##### `try_parse(...) -> LocalId | None`

Safely parse a Local ID, returning None if invalid.

#### Builder Methods

##### `with_primary_item(path: GmodPath) -> LocalIdBuilder`

Set the primary GMOD path.

**Parameters:**

- `path`: Primary GMOD path

**Returns:** Updated builder

##### `with_secondary_item(path: GmodPath) -> LocalIdBuilder`

Set the secondary GMOD path (for relationships).

**Parameters:**

- `path`: Secondary GMOD path

**Returns:** Updated builder

##### `with_metadata_tag(tag: MetadataTag) -> LocalIdBuilder`

Add a metadata tag (quantity, position, content, etc.).

**Parameters:**

- `tag`: MetadataTag instance created from codebooks

**Returns:** Updated builder

**Example:**

```python
# Create tags from codebooks
quantity_tag = codebooks.create_tag(CodebookName.Quantity, "temperature")
content_tag = codebooks.create_tag(CodebookName.Content, "cooling.water")

builder = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(path)
    .with_metadata_tag(quantity_tag)
    .with_metadata_tag(content_tag))
```

**Note:** All metadata tags (quantity, position, content, state, command, type, detail) are added using the same `with_metadata_tag()` method. Tags must be created from codebooks first.

##### `with_verbose_mode(verbose: bool) -> LocalIdBuilder`

Enable/disable verbose path representation.

**Parameters:**

- `verbose`: Whether to use verbose mode

**Returns:** Updated builder

##### `build() -> LocalId`

Build the final Local ID.

**Returns:** Constructed `LocalId` instance

**Raises:** `ValueError` if required components missing

### Class: `LocalId`

Represents a complete, validated Local ID.

#### Properties

- `vis_version: VisVersion` - Associated VIS version
- `primary_item: GmodPath` - Primary GMOD path
- `secondary_item: GmodPath | None` - Secondary path (if any)
- `metadata_tags: list[MetadataTag]` - All metadata tags
- `is_valid: bool` - Whether the Local ID is valid
- `has_custom_tag: bool` - Whether any tags are custom

#### Methods

##### `__str__() -> str`

Get the string representation of the Local ID.

**Returns:** Complete Local ID string

---

## Codebooks

Standardized vocabularies for metadata tags.

### Class: `Codebooks`

Container for all codebook types.

#### Methods

##### `__getitem__(name: CodebookName) -> Codebook`

Get a specific codebook by name.

**Parameters:**

- `name`: Codebook identifier

**Returns:** `Codebook` instance

**Example:**

```python
codebooks = vis.get_codebooks(VisVersion.v3_4a)
positions = codebooks[CodebookName.Position]
```

##### `create_tag(name: CodebookName, value: str) -> MetadataTag`

Create a metadata tag from any codebook.

**Parameters:**

- `name`: Codebook to use
- `value`: Tag value

**Returns:** `MetadataTag` instance

**Raises:** `ValueError` for invalid values

### Class: `Codebook`

Individual codebook with validation rules.

#### Properties

- `name: CodebookName` - Codebook identifier
- `standard_values: Iterator[str]` - All standard values
- `groups: Iterator[str]` - All value groups

#### Methods

##### `create_tag(value: str) -> MetadataTag`

Create a validated metadata tag.

**Parameters:**

- `value`: Tag value to create

**Returns:** `MetadataTag` instance

**Raises:** `ValueError` for invalid values

##### `try_create_tag(value: str) -> MetadataTag | None`

Safely create a tag, returning None if invalid.

##### `has_standard_value(value: str) -> bool`

Check if a value is in the standard vocabulary.

**Parameters:**

- `value`: Value to check

**Returns:** True if standard, False if custom/invalid

##### `validate_position(position: str) -> PositionValidationResult`

Special validation for position tags (only available on Position codebook).

**Parameters:**

- `position`: Position string to validate

**Returns:** Validation result enum

### Enum: `CodebookName`

Available codebook types:

- `Quantity` - Physical quantities (temperature, pressure, etc.)
- `Content` - Substances and materials
- `Position` - Physical positions and locations
- `State` - Conditions and states
- `Command` - Actions and commands
- `Type` - Classification types
- `Detail` - Additional details

### Class: `MetadataTag`

Individual metadata tag with validation.

#### Properties

- `name: CodebookName` - Source codebook
- `value: str` - Tag value
- `is_custom: bool` - Whether this is a custom (non-standard) value
- `prefix: str` - Prefix character ('-' for standard, '~' for custom)

---

## Locations

Physical positioning and location definitions.

### Class: `Locations`

Location validation and processing.

#### Methods

##### `validate_location(location: str) -> bool`

Validate a location string.

**Parameters:**

- `location`: Location string to validate

**Returns:** True if valid

---

## Version Management

### Enum: `VisVersion`

Available VIS versions:

- `v3_4a` - VIS 3-4a
- `v3_5a` - VIS 3-5a
- `v3_6a` - VIS 3-6a
- `v3_7a` - VIS 3-7a
- `v3_8a` - VIS 3-8a
- `v3_9a` - VIS 3-9a

### Class: `GmodVersioning`

GMOD path conversion between versions.

#### Methods

##### `convert_path(path: str, target_version: VisVersion) -> str`

Convert a path from one VIS version to another.

**Parameters:**

- `path`: Source path string
- `target_version`: Target VIS version

**Returns:** Converted path string

**Raises:** `ValueError` if conversion not possible

---

## Error Handling

### Common Exceptions

- `ValueError` - Invalid input data or failed validation
- `KeyError` - Requested item not found (nodes, codebooks, etc.)
- `FileNotFoundError` - Required resource files missing

### Best Practices

1. **Use try/catch blocks** for parsing operations
2. **Use try\_\* methods** when failure is acceptable
3. **Validate inputs** before building complex objects
4. **Check is_valid** properties on built objects

### Example Error Handling

```python
# Safe parsing
try:
    local_id = LocalId.parse(id_string)
    if local_id.is_valid:
        print(f"Valid Local ID: {local_id}")
    else:
        print("Local ID parsed but has validation issues")
except ValueError as e:
    print(f"Invalid Local ID format: {e}")

# Safe building
vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)

builder = LocalIdBuilder.create(VisVersion.v3_4a)
if path := GmodPath.try_parse(gmod, "411.1/C101.31"):
    builder = builder.with_primary_item(path)
else:
    print("Invalid GMOD path")
```

---

## Performance Notes

### Caching

- VIS instances cache loaded data automatically
- Multiple calls to `get_gmod()` with the same version return cached results
- Codebooks and locations are also cached per version

### Memory Usage

- Large GMOD structures are loaded on demand
- Consider using `try_*` methods to avoid exceptions in tight loops
- Cache frequently-used codebooks locally if making many tag operations

### Threading

- VIS instances are thread-safe for read operations
- Multiple threads can safely access the same VIS instance
- Builder instances are NOT thread-safe - create separate builders per thread

---

## Migration Guide

### From C# SDK

Key differences when migrating from the C# implementation:

| C#                        | Python                     | Notes                           |
| ------------------------- | -------------------------- | ------------------------------- |
| `VIS.Instance`            | `VIS()`                    | Python uses regular constructor |
| `GmodPath.TryParse()`     | `GmodPath.try_parse()`     | Python naming convention        |
| `LocalIdBuilder.Create()` | `LocalIdBuilder.create()`  | Static method                   |
| `codebook.StandardValues` | `codebook.standard_values` | Property vs method              |
| `tag.IsCustom`            | `tag.is_custom`            | Property naming                 |

### Code Conversion Example

**C#:**

```csharp
var vis = VIS.Instance;
var gmod = vis.GetGmod(VisVersion.v3_4a);
var path = GmodPath.Parse("411.1/C101.31", VisVersion.v3_4a);
var localId = LocalIdBuilder.Create(VisVersion.v3_4a)
    .WithPrimaryItem(path)
    .WithQuantityTag("temperature")
    .Build();
```

**Python:**

```python
vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)
codebooks = vis.get_codebooks(VisVersion.v3_4a)

path = GmodPath.parse(gmod, "411.1/C101.31")
quantity_tag = codebooks.create_tag(CodebookName.Quantity, "temperature")

local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(path)
    .with_metadata_tag(quantity_tag)
    .build())
```
