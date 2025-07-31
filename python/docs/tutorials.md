# Vista SDK Python - Tutorial Guide

Welcome to the comprehensive tutorial for the Vista SDK Python implementation! This guide will walk you through common use cases and advanced features step by step.

## 🎯 Learning Path

1. [Getting Started](#getting-started)
2. [Understanding GMOD](#understanding-gmod-general-model-of-data)
3. [Working with Local IDs](#working-with-local-ids)
4. [Using Codebooks](#using-codebooks)
5. [Location Operations](#location-operations)
6. [Version Management](#version-management)
7. [Advanced Patterns](#advanced-patterns)
8. [Integration Examples](#integration-examples)

---

## Getting Started

### Prerequisites
- Python 3.10 or higher
- Vista SDK installed (`pip install vista-sdk`)

### Your First VIS Connection

```python
from vista_sdk import VIS, VisVersion

# Create VIS instance - this is your main entry point
vis = VIS()

# Load GMOD data for a specific version
gmod = vis.get_gmod(VisVersion.v3_4a)
print(f"Loaded GMOD with {len(list(gmod.all_nodes()))} nodes")

# Load codebooks for metadata validation
codebooks = vis.get_codebooks(VisVersion.v3_4a)
print(f"Available codebooks: {[book.name for book in codebooks]}")
```

**What's happening:**
- `VIS()` creates your connection to all VIS data
- Data is cached automatically - multiple calls are efficient
- Each VIS version has its own data set

---

## Understanding GMOD (General Model of Data)

GMOD represents the hierarchical structure of vessel equipment and systems.

### Tutorial 1: Exploring the GMOD Tree

```python
from vista_sdk import VIS, VisVersion

vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)

# Start at a high-level system
main_engine = gmod.get_node("411")
print(f"System: {main_engine.name}")
print(f"Code: {main_engine.code}")
print(f"Can be mapped: {main_engine.is_mappable}")

# Explore children
print("\nDirect children:")
for child in main_engine.children:
    print(f"  {child.code}: {child.name}")

# Go deeper - specific engine instance
engine_1 = main_engine.get_child("411.1")
print(f"\nEngine 1: {engine_1.name}")

# Even deeper - specific component
cooling_system = engine_1.get_child("C101")
print(f"Cooling system: {cooling_system.name}")
```

### Tutorial 2: Working with GMOD Paths

GMOD paths represent navigation through the hierarchy:

```python
from vista_sdk import GmodPath, VisVersion

# Parse a complete path
path_str = "411.1/C101.31-2"
path = GmodPath.parse(path_str, VisVersion.v3_4a)

print(f"Path: {path_str}")
print(f"Terminal node: {path.node.name}")
print(f"Is mappable: {path.node.is_mappable}")

# Walk the full path
print("\nFull path breakdown:")
for depth, node in path.get_full_path():
    indent = "  " * depth
    print(f"{indent}Level {depth}: {node.code} - {node.name}")

# Safe parsing for user input
user_input = "999.9/INVALID"
safe_path = GmodPath.try_parse(user_input, VisVersion.v3_4a)
if safe_path:
    print(f"Valid path: {safe_path.node.name}")
else:
    print(f"Invalid path: {user_input}")
```

**Key Concepts:**
- Paths use forward slashes (`/`) to separate levels
- Dashes indicate instances (e.g., `-2` for instance 2)
- Only "mappable" nodes can be used in Local IDs
- Always validate paths from external sources

---

## Working with Local IDs

Local IDs are standardized identifiers for data channels following ISO 19847/19848.

### Tutorial 3: Building Your First Local ID

```python
from vista_sdk import VIS, VisVersion, LocalIdBuilder, GmodPath

vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)
codebooks = vis.get_codebooks(VisVersion.v3_4a)

# Step 1: Define what you're measuring
path = GmodPath.parse("411.1/C101.31-2", VisVersion.v3_4a)
print(f"Measuring: {path.node.name}")

# Step 2: Build the Local ID
local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(path)
    .with_quantity_tag("temperature")
    .build())

print(f"Local ID: {local_id}")
print(f"Is valid: {local_id.is_valid}")
```

### Tutorial 4: Complex Local IDs with Multiple Tags

```python
# More complex example - cooled by seawater
path = GmodPath.parse("411.1/C101.31", VisVersion.v3_4a)

local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(path)
    .with_quantity_tag("temperature")
    .with_content_tag("cooling.water")
    .with_position_tag("inlet")
    .with_state_tag("normal")
    .build())

print(f"Complex Local ID: {local_id}")

# Examine the components
print(f"Primary item: {local_id.primary_item.node.name}")
print("Metadata tags:")
for tag in local_id.metadata_tags:
    print(f"  {tag.name.value}: {tag.value} {'(custom)' if tag.is_custom else ''}")
```

### Tutorial 5: Parsing Existing Local IDs

```python
from vista_sdk import LocalIdBuilder

# Parse a Local ID string
id_string = "/vis-3-4a/411.1/C101.31-2/meta:qty.temperature"

try:
    parsed_id = LocalIdBuilder.parse(
        id_string,
        gmod=gmod,
        codebooks=codebooks,
        locations=vis.get_locations(VisVersion.v3_4a)
    )

    print(f"Parsed Local ID: {parsed_id}")
    print(f"Equipment: {parsed_id.primary_item.node.name}")
    print(f"Quantity: {parsed_id.metadata_tags[0].value}")

except ValueError as e:
    print(f"Failed to parse: {e}")
```

---

## Using Codebooks

Codebooks provide standardized vocabularies for metadata tags.

### Tutorial 6: Exploring Available Tags

```python
from vista_sdk.codebook_names import CodebookName

codebooks = vis.get_codebooks(VisVersion.v3_4a)

# Explore quantity codebook
quantities = codebooks[CodebookName.Quantity]
print("Available quantities:")
for value in list(quantities.standard_values)[:10]:  # First 10
    print(f"  {value}")

# Check if a value is standard
print(f"\n'temperature' is standard: {quantities.has_standard_value('temperature')}")
print(f"'my_custom_temp' is standard: {quantities.has_standard_value('my_custom_temp')}")
```

### Tutorial 7: Creating and Validating Tags

```python
# Create tags from different codebooks
temp_tag = quantities.create_tag("temperature")
print(f"Temperature tag: {temp_tag.prefix}{temp_tag.value}")

# Create custom tag (not in standard vocabulary)
custom_tag = quantities.create_tag("my_special_measurement")
print(f"Custom tag: {custom_tag.prefix}{custom_tag.value}")
print(f"Is custom: {custom_tag.is_custom}")

# Safe tag creation
safe_tag = quantities.try_create_tag("invalid~tag~name")
if safe_tag:
    print(f"Created: {safe_tag}")
else:
    print("Failed to create tag")
```

### Tutorial 8: Position Validation

The Position codebook has special validation rules:

```python
positions = codebooks[CodebookName.Position]

# Test various position formats
test_positions = [
    "inlet",
    "outlet",
    "port.side",
    "frame.102",
    "invalid..position"
]

for pos in test_positions:
    result = positions.validate_position(pos)
    print(f"'{pos}': {result}")
```

---

## Location Operations

Locations provide physical positioning information.

### Tutorial 9: Working with Locations

```python
locations = vis.get_locations(VisVersion.v3_4a)

# Validate location strings
test_locations = [
    "engine.room.1",
    "bridge",
    "invalid.location.format"
]

for loc in test_locations:
    is_valid = locations.validate_location(loc)
    print(f"'{loc}': {'Valid' if is_valid else 'Invalid'}")
```

---

## Version Management

### Tutorial 10: Working Across VIS Versions

```python
# Load multiple versions
gmod_34 = vis.get_gmod(VisVersion.v3_4a)
gmod_35 = vis.get_gmod(VisVersion.v3_5a)

print(f"v3.4a nodes: {len(list(gmod_34.all_nodes()))}")
print(f"v3.5a nodes: {len(list(gmod_35.all_nodes()))}")

# Convert paths between versions
versioning = vis.get_gmod_versioning(VisVersion.v3_4a)

old_path = "411.1/C101.72"
try:
    new_path = versioning.convert_path(old_path, VisVersion.v3_5a)
    print(f"Converted: {old_path} -> {new_path}")
except ValueError as e:
    print(f"Conversion failed: {e}")
```

---

## Advanced Patterns

### Tutorial 11: Batch Processing Local IDs

```python
def process_sensor_data(sensor_configs):
    """Process multiple sensor configurations efficiently."""

    # Cache commonly used objects
    vis = VIS()
    gmod = vis.get_gmod(VisVersion.v3_4a)
    codebooks = vis.get_codebooks(VisVersion.v3_4a)

    results = []

    for config in sensor_configs:
        try:
            # Parse path once
            path = GmodPath.parse(config['path'], VisVersion.v3_4a)

            # Build Local ID
            builder = (LocalIdBuilder.create(VisVersion.v3_4a)
                .with_primary_item(path)
                .with_quantity_tag(config['quantity']))

            # Add optional tags
            if 'content' in config:
                builder = builder.with_content_tag(config['content'])
            if 'position' in config:
                builder = builder.with_position_tag(config['position'])

            local_id = builder.build()

            results.append({
                'config': config,
                'local_id': str(local_id),
                'valid': local_id.is_valid
            })

        except Exception as e:
            results.append({
                'config': config,
                'error': str(e),
                'valid': False
            })

    return results

# Example usage
sensors = [
    {'path': '411.1/C101.31-1', 'quantity': 'temperature', 'position': 'inlet'},
    {'path': '411.1/C101.31-2', 'quantity': 'pressure', 'content': 'cooling.water'},
    {'path': '411.2/C102.31', 'quantity': 'flow.rate'}
]

results = process_sensor_data(sensors)
for result in results:
    if result['valid']:
        print(f"✓ {result['local_id']}")
    else:
        print(f"✗ Error: {result.get('error', 'Invalid')}")
```

### Tutorial 12: Custom Validation Pipeline

```python
from typing import List, Tuple, Optional

class LocalIdValidator:
    """Custom validation pipeline for Local IDs."""

    def __init__(self, vis_version: VisVersion):
        self.vis = VIS()
        self.gmod = self.vis.get_gmod(vis_version)
        self.codebooks = self.vis.get_codebooks(vis_version)
        self.locations = self.vis.get_locations(vis_version)

    def validate_pipeline(self, local_id_str: str) -> Tuple[bool, List[str]]:
        """Validate Local ID with detailed error reporting."""

        errors = []

        try:
            # Parse the Local ID
            local_id = LocalIdBuilder.parse(
                local_id_str,
                self.gmod,
                self.codebooks,
                self.locations
            )

            # Custom validation rules
            if not local_id.is_valid:
                errors.append("Local ID failed basic validation")

            if local_id.has_custom_tag:
                errors.append("Contains custom tags - verify with domain expert")

            # Check if primary item is mappable
            if not local_id.primary_item.node.is_mappable:
                errors.append(f"Primary item '{local_id.primary_item.node.code}' is not mappable")

            # Validate tag combinations
            tag_names = [tag.name for tag in local_id.metadata_tags]
            if CodebookName.Quantity not in tag_names:
                errors.append("Missing required quantity tag")

            return len(errors) == 0, errors

        except ValueError as e:
            return False, [f"Parse error: {e}"]

# Usage
validator = LocalIdValidator(VisVersion.v3_4a)

test_ids = [
    "/vis-3-4a/411.1/C101.31/meta:qty.temperature",
    "/vis-3-4a/411.1/C101.31/meta:qty.~custom_measurement",
    "/vis-3-4a/invalid/path/meta:qty.temperature"
]

for test_id in test_ids:
    is_valid, errors = validator.validate_pipeline(test_id)
    print(f"\n{test_id}")
    print(f"Valid: {is_valid}")
    if errors:
        for error in errors:
            print(f"  - {error}")
```

---

## Integration Examples

### Tutorial 13: Time Series Data Integration

```python
import json
from datetime import datetime, timezone
from typing import Dict, Any

class TimeSeriesDataPackage:
    """Create VIS-compliant time series data packages."""

    def __init__(self, vis_version: VisVersion):
        self.vis_version = vis_version
        self.vis = VIS()
        self.gmod = self.vis.get_gmod(vis_version)
        self.codebooks = self.vis.get_codebooks(vis_version)
        self.locations = self.vis.get_locations(vis_version)

    def create_package(self,
                      vessel_id: str,
                      time_span_start: datetime,
                      time_span_end: datetime,
                      data_points: List[Dict[str, Any]]) -> Dict[str, Any]:
        """Create a complete time series package."""

        # Validate and build Local IDs for all data points
        validated_data = []

        for point in data_points:
            try:
                local_id = LocalIdBuilder.parse(
                    point['local_id'],
                    self.gmod,
                    self.codebooks,
                    self.locations
                )

                if local_id.is_valid:
                    validated_data.append({
                        'localId': str(local_id),
                        'value': point['value'],
                        'timestamp': point['timestamp'].isoformat()
                    })
                else:
                    print(f"Warning: Invalid Local ID {point['local_id']}")

            except ValueError as e:
                print(f"Error processing {point['local_id']}: {e}")

        # Create VIS-compliant package structure
        package = {
            'header': {
                'vesselId': vessel_id,
                'timeSpan': {
                    'start': time_span_start.isoformat(),
                    'end': time_span_end.isoformat()
                },
                'visVersion': self.vis_version.value,
                'author': 'Vista SDK Python',
                'dateCreated': datetime.now(timezone.utc).isoformat()
            },
            'timeSeriesData': validated_data
        }

        return package

# Example usage
package_builder = TimeSeriesDataPackage(VisVersion.v3_4a)

sample_data = [
    {
        'local_id': '/vis-3-4a/411.1/C101.31-1/meta:qty.temperature',
        'value': 85.5,
        'timestamp': datetime(2024, 1, 15, 10, 30, 0, tzinfo=timezone.utc)
    },
    {
        'local_id': '/vis-3-4a/411.1/C101.31-1/meta:qty.pressure',
        'value': 2.1,
        'timestamp': datetime(2024, 1, 15, 10, 30, 0, tzinfo=timezone.utc)
    }
]

package = package_builder.create_package(
    vessel_id="IMO123456789",
    time_span_start=datetime(2024, 1, 15, 10, 0, 0, tzinfo=timezone.utc),
    time_span_end=datetime(2024, 1, 15, 11, 0, 0, tzinfo=timezone.utc),
    data_points=sample_data
)

print(json.dumps(package, indent=2))
```

### Tutorial 14: Equipment Discovery and Mapping

```python
def discover_equipment_by_type(gmod: Gmod, equipment_type: str) -> List[GmodNode]:
    """Find all equipment of a specific type in the GMOD."""

    matching_nodes = []

    for node in gmod.all_nodes():
        if equipment_type.lower() in node.name.lower() or equipment_type.lower() in node.common_name.lower():
            if node.is_mappable:  # Can be used in Local IDs
                matching_nodes.append(node)

    return matching_nodes

def create_sensor_template(node: GmodNode, quantities: List[str]) -> List[str]:
    """Create Local ID templates for common measurements on equipment."""

    templates = []

    for quantity in quantities:
        try:
            # Create a basic GMOD path (assuming no instances for template)
            path = GmodPath.parse(node.code, VisVersion.v3_4a)

            local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
                .with_primary_item(path)
                .with_quantity_tag(quantity)
                .build())

            templates.append(str(local_id))

        except Exception as e:
            print(f"Could not create template for {node.code}/{quantity}: {e}")

    return templates

# Example: Find all pumps and create sensor templates
vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)

pumps = discover_equipment_by_type(gmod, "pump")
print(f"Found {len(pumps)} pump-related nodes:")

common_pump_measurements = ["pressure", "flow.rate", "temperature", "power"]

for pump in pumps[:5]:  # Show first 5
    print(f"\n{pump.code}: {pump.name}")
    templates = create_sensor_template(pump, common_pump_measurements)
    for template in templates:
        print(f"  {template}")
```

---

## Best Practices Summary

### 1. Error Handling
```python
# Always use try/except for parsing operations
try:
    local_id = LocalIdBuilder.parse(user_input, gmod, codebooks, locations)
except ValueError as e:
    logger.warning(f"Invalid Local ID from user: {e}")

# Use try_* methods when failure is acceptable
path = GmodPath.try_parse(path_string, vis_version)
if path is None:
    # Handle gracefully
    pass
```

### 2. Performance Optimization
```python
# Cache VIS instances and commonly used objects
class SensorProcessor:
    def __init__(self, vis_version: VisVersion):
        self.vis = VIS()  # Cache this
        self.gmod = self.vis.get_gmod(vis_version)
        self.codebooks = self.vis.get_codebooks(vis_version)

    def process_batch(self, sensor_data):
        # Reuse cached objects
        pass
```

### 3. Validation
```python
# Always validate Local IDs after building
local_id = builder.build()
if not local_id.is_valid:
    logger.error(f"Built invalid Local ID: {local_id}")

# Check for custom tags in production systems
if local_id.has_custom_tag:
    logger.info(f"Local ID contains custom tags: {local_id}")
```

### 4. Version Management
```python
# Be explicit about versions
def create_measurement_id(equipment_path: str, quantity: str, vis_version: VisVersion):
    path = GmodPath.parse(equipment_path, vis_version)
    return (LocalIdBuilder.create(vis_version)
        .with_primary_item(path)
        .with_quantity_tag(quantity)
        .build())
```

---

## Next Steps

Now that you've completed the tutorials, you can:

1. **Read the [API Reference](API.md)** for complete method documentation
2. **Explore the [samples/](../samples/)** directory for more examples
3. **Check the [README.md](../README.md)** for installation and setup details
4. **Contribute** by reporting issues or suggesting improvements

Happy coding with the Vista SDK! 🚢
