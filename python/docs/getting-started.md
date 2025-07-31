# Vista SDK Python - Getting Started Guide

Welcome to the Vista SDK Python implementation! This guide will help you get up and running quickly with vessel data integration using the DNV Vessel Information Structure (VIS) standard.

## 🎯 What is Vista SDK?

The Vista SDK provides a Python interface for working with:
- **VIS (Vessel Information Structure)** - Standardized data model for vessel equipment and systems
- **ISO 19847/19848** - International standards for vessel data exchange
- **Local IDs** - Unique identifiers for data channels and measurements
- **GMOD** - General Model of Data representing equipment hierarchy
- **Codebooks** - Standardized vocabularies for metadata

## 🚀 Quick Installation

```bash
# Install from PyPI (when available)
pip install vista-sdk

# Or install from source
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/python
pip install -e .
```

## ⚡ 5-Minute Quick Start

Let's create your first Local ID - a standardized identifier for vessel data:

```python
from vista_sdk import VIS, VisVersion, LocalIdBuilder, GmodPath

# 1. Initialize VIS connection
vis = VIS()

# 2. Load data for VIS version 3.4a
gmod = vis.get_gmod(VisVersion.v3_4a)
codebooks = vis.get_codebooks(VisVersion.v3_4a)

# 3. Find equipment in the hierarchy
engine_path = GmodPath.parse("411.1/C101.31-2", VisVersion.v3_4a)
print(f"Equipment: {engine_path.node.name}")
# Output: Equipment: Fresh water cooler

# 4. Build a Local ID for temperature measurement
local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(engine_path)
    .with_quantity_tag("temperature")
    .with_position_tag("inlet")
    .build())

print(f"Local ID: {local_id}")
# Output: /vis-3-4a/411.1/C101.31-2/meta:qty.temperature-pos.inlet

# 5. Validate the result
print(f"Valid: {local_id.is_valid}")
# Output: Valid: True
```

**Congratulations!** You just created a standardized identifier for inlet temperature on a fresh water cooler.

## 🏗️ Core Concepts

### 1. VIS Versions
Different versions of the VIS standard contain different equipment definitions:

```python
# Load different versions
gmod_34 = vis.get_gmod(VisVersion.v3_4a)  # VIS 3.4a
gmod_35 = vis.get_gmod(VisVersion.v3_5a)  # VIS 3.5a
gmod_36 = vis.get_gmod(VisVersion.v3_6a)  # VIS 3.6a
# ... up to v3_9a

print(f"VIS 3.4a has {len(list(gmod_34.all_nodes()))} equipment nodes")
```

### 2. GMOD Hierarchy
Equipment is organized in a tree structure:

```python
# Navigate the equipment tree
main_engine = gmod.get_node("411")        # Main engine system
engine_1 = main_engine.get_child("411.1") # Engine instance 1
cooling = engine_1.get_child("C101")      # Cooling system
cooler = cooling.get_child("C101.31")     # Fresh water cooler

print(f"Path: {main_engine.name} -> {engine_1.name} -> {cooling.name} -> {cooler.name}")
```

### 3. Local ID Structure
Local IDs follow the pattern: `/vis-version/equipment-path/metadata`

```
/vis-3-4a/411.1/C101.31-2/meta:qty.temperature-pos.inlet
│        │               │
├─ VIS version           ├─ Metadata tags
└─ Equipment path        └─ Quantity + Position
```

### 4. Metadata Tags
Tags describe what you're measuring:

```python
from vista_sdk.codebook_names import CodebookName

codebooks = vis.get_codebooks(VisVersion.v3_4a)

# Explore available tags
quantities = codebooks[CodebookName.Quantity]
positions = codebooks[CodebookName.Position]

print("Sample quantities:", list(quantities.standard_values)[:5])
print("Sample positions:", list(positions.standard_values)[:5])
```

## 📋 Common Use Cases

### Use Case 1: Sensor Data Collection

```python
def create_sensor_local_id(equipment_path: str, measurement: str):
    """Create Local ID for a sensor reading."""

    try:
        # Parse the equipment path
        path = GmodPath.parse(equipment_path, VisVersion.v3_4a)

        # Build the Local ID
        local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
            .with_primary_item(path)
            .with_quantity_tag(measurement)
            .build())

        return str(local_id) if local_id.is_valid else None

    except ValueError as e:
        print(f"Error creating Local ID: {e}")
        return None

# Example: Temperature sensors on different equipment
sensors = [
    ("411.1/C101.31-1", "temperature"),  # Engine 1 cooler 1
    ("411.1/C101.31-2", "temperature"),  # Engine 1 cooler 2
    ("412.1/C101.31-1", "temperature"),  # Engine 2 cooler 1
]

for equipment, measurement in sensors:
    local_id = create_sensor_local_id(equipment, measurement)
    if local_id:
        print(f"✓ {equipment} {measurement}: {local_id}")
    else:
        print(f"✗ Failed to create Local ID for {equipment}")
```

### Use Case 2: Parsing Existing Local IDs

```python
def analyze_local_id(local_id_string: str):
    """Parse and analyze an existing Local ID."""

    try:
        local_id = LocalIdBuilder.parse(
            local_id_string,
            gmod=gmod,
            codebooks=codebooks,
            locations=vis.get_locations(VisVersion.v3_4a)
        )

        print(f"Local ID: {local_id}")
        print(f"Equipment: {local_id.primary_item.node.name}")
        print(f"Equipment code: {local_id.primary_item.node.code}")
        print(f"Is valid: {local_id.is_valid}")

        print("Metadata tags:")
        for tag in local_id.metadata_tags:
            custom_indicator = " (custom)" if tag.is_custom else ""
            print(f"  {tag.name.value}: {tag.value}{custom_indicator}")

    except ValueError as e:
        print(f"Failed to parse Local ID: {e}")

# Example
analyze_local_id("/vis-3-4a/411.1/C101.31-2/meta:qty.temperature-pos.inlet")
```

### Use Case 3: Equipment Discovery

```python
def find_equipment_by_name(search_term: str):
    """Find equipment containing a search term."""

    matches = []
    for node in gmod.all_nodes():
        if search_term.lower() in node.name.lower():
            if node.is_mappable:  # Can be used in Local IDs
                matches.append(node)

    return matches

# Find all pumps
pumps = find_equipment_by_name("pump")
print(f"Found {len(pumps)} pump-related equipment:")

for pump in pumps[:5]:  # Show first 5
    print(f"  {pump.code}: {pump.name}")

    # Create a sample Local ID for flow measurement
    try:
        path = GmodPath.parse(pump.code, VisVersion.v3_4a)
        local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
            .with_primary_item(path)
            .with_quantity_tag("flow.rate")
            .build())
        print(f"    Flow rate: {local_id}")
    except:
        print(f"    (Cannot create Local ID for {pump.code})")
```

## 🔧 Development Setup

### Prerequisites
- Python 3.8 or higher
- Git (for development from source)

### Install for Development

```bash
# Clone the repository
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/python

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install in development mode
pip install -e .

# Install development dependencies
pip install -e ".[dev]"
```

### Run Tests

```bash
# Run all tests
pytest

# Run with coverage
pytest --cov=vista_sdk

# Run specific test file
pytest tests/test_local_id.py
```

### Code Quality Tools

```bash
# Format code
ruff format .

# Lint code
ruff check .

# Type checking (if mypy is installed)
mypy src/vista_sdk
```

## 🐛 Troubleshooting

### Common Issues

**Q: "ModuleNotFoundError: No module named 'vista_sdk'"**
```bash
# Make sure you've installed the package
pip install -e .

# Or check your Python path
python -c "import sys; print(sys.path)"
```

**Q: "KeyError when accessing GMOD node"**
```python
# Use try_get_node for safe access
node = gmod.try_get_node("invalid.code")
if node is None:
    print("Node not found")
```

**Q: "ValueError when building Local ID"**
```python
# Validate components first
path = GmodPath.try_parse("411.1/C101.31", VisVersion.v3_4a)
if path is None:
    print("Invalid GMOD path")
else:
    # Proceed with Local ID building
    pass
```

**Q: "Performance issues with large datasets"**
```python
# Cache VIS instances and reuse them
class DataProcessor:
    def __init__(self):
        self.vis = VIS()  # Cache this
        self.gmod = self.vis.get_gmod(VisVersion.v3_4a)
        self.codebooks = self.vis.get_codebooks(VisVersion.v3_4a)
```

### Getting Help

- **Documentation**: Check the [API Reference](docs/API.md) and [Tutorials](docs/tutorials.md)
- **Examples**: See the [samples/](samples/) directory
- **Issues**: Report bugs on [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)
- **Discussions**: Join conversations on [GitHub Discussions](https://github.com/dnv-opensource/vista-sdk/discussions)

## 🎓 Learning Path

1. **Start Here**: Complete this getting started guide
2. **Try Examples**: Work through [samples/](samples/) directory
3. **Deep Dive**: Read the [tutorials](docs/tutorials.md) for advanced patterns
4. **Reference**: Use [API documentation](docs/API.md) for detailed method info
5. **Contribute**: Help improve the SDK by reporting issues or contributing code

## 🔗 Related Resources

- [VIS Documentation](https://www.dnv.com/services/vessel-information-structure-vis-120226) - Official VIS specification
- [ISO 19847](https://www.iso.org/standard/66356.html) - Ships and marine technology standard
- [Vista SDK C#](../csharp/) - C# implementation of the same SDK
- [Vista SDK JavaScript](../js/) - JavaScript/TypeScript implementation

## 🎉 What's Next?

Now that you have the basics:

1. **Explore the [samples](samples/)** - See practical examples
2. **Read the [tutorials](docs/tutorials.md)** - Learn advanced patterns
3. **Build something cool** - Create your vessel data integration
4. **Share your experience** - Help others by contributing examples or documentation

Welcome to the Vista SDK community! 🚢✨
