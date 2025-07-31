# Vista SDK - Python Implementation

[![PyPI current](https://img.shields.io/pypi/v/vista-sdk?label=PyPI%20vista-sdk)](https://pypi.org/project/vista-sdk/)
[![Python Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/build-python.yml?branch=main&label=Python)](https://github.com/dnv-opensource/vista-sdk/actions)
[![GitHub](https://img.shields.io/github/license/dnv-opensource/vista-sdk?style=flat-square)](https://github.com/dnv-opensource/vista-sdk/blob/main/LICENSE)

The Vista SDK Python implementation provides comprehensive tools for working with:
- **DNV Vessel Information Structure (VIS)**
- **ISO 19847** - Ships and marine technology — Shipboard data servers to share field data at sea
- **ISO 19848** - Ships and marine technology — Standard data for shipboard machinery and equipment

## 📦 Installation

### PyPI Installation
```bash
pip install vista-sdk
```

### Development Installation
```bash
# Clone the repository
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/python

# Install with uv (recommended)
uv sync
uv run pre-commit install

# Or install with pip
pip install -e .
```

## 🚀 Quick Start

### Basic Usage

```python
from vista_sdk import VIS, VisVersion, LocalIdBuilder, GmodPath

# Initialize VIS instance
vis = VIS()

# Get GMOD data for a specific VIS version
gmod = vis.get_gmod(VisVersion.v3_4a)
codebooks = vis.get_codebooks(VisVersion.v3_4a)
locations = vis.get_locations(VisVersion.v3_4a)

# Parse a GMOD path
path = GmodPath.parse("411.1/C101.31-2", VisVersion.v3_4a)
print(f"Parsed path: {path}")

# Build a Local ID
local_id = (LocalIdBuilder.create(VisVersion.v3_4a)
    .with_primary_item(path)
    .with_quantity_tag("temperature")
    .with_content_tag("exhaust.gas")
    .with_position_tag("inlet")
    .build())

print(f"Local ID: {local_id}")
```

### Working with Codebooks

```python
from vista_sdk import VIS, VisVersion
from vista_sdk.codebook_names import CodebookName

vis = VIS()
codebooks = vis.get_codebooks(VisVersion.v3_4a)

# Get a specific codebook
position_codebook = codebooks[CodebookName.Position]

# Create metadata tags
position_tag = position_codebook.create_tag("centre")
quantity_tag = codebooks.create_tag(CodebookName.Quantity, "temperature")

print(f"Position tag: {position_tag}")
print(f"Quantity tag: {quantity_tag}")

# Check if values are valid
print(f"Is 'centre' valid position? {position_codebook.has_standard_value('centre')}")
```

### GMOD Path Operations

```python
from vista_sdk import VIS, VisVersion, GmodPath

vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)

# Parse a path
path = GmodPath.parse("411.1/C101.31-2/meta", VisVersion.v3_4a)

# Get path information
print(f"Path depth: {len(path)}")
print(f"Node at depth 1: {path.node}")
print(f"Full path string: {path}")

# Traverse the path
for depth, node in path.get_full_path():
    print(f"Depth {depth}: {node.code} - {node.common_name}")
```

### Version Conversion

```python
from vista_sdk import VIS, VisVersion, GmodPath

vis = VIS()

# Convert paths between VIS versions
old_path = "411.1/C101.72/I101"
gmod_versioning = vis.get_gmod_versioning(VisVersion.v3_4a)

try:
    new_path = gmod_versioning.convert_path(old_path, VisVersion.v3_5a)
    print(f"Converted path: {old_path} -> {new_path}")
except Exception as e:
    print(f"Conversion failed: {e}")
```

## 📚 Core Components

### VIS (Vessel Information Structure)
The main entry point for accessing VIS data:
- **GMOD** (General Model of Data) - Equipment and system hierarchy
- **Codebooks** - Standardized metadata tags
- **Locations** - Physical positioning information
- **Versioning** - Path conversion between VIS versions

### Local ID Builder
Construct standardized local identifiers:
```python
from vista_sdk import LocalIdBuilder, VisVersion

builder = LocalIdBuilder.create(VisVersion.v3_4a)
local_id = (builder
    .with_primary_item_from_path("411.1/C101.31")
    .with_quantity_tag("temperature")
    .with_content_tag("cooling.water")
    .with_state_tag("high")
    .build())
```

### Builder Pattern Support
The SDK follows a fluent builder pattern:
- **`with_*()`** - Add or set values (throws on invalid input)
- **`try_with_*()`** - Add values safely (returns success status)
- **`without_*()`** - Remove specific components

## 🔧 Advanced Usage

### Custom Error Handling

```python
from vista_sdk import LocalIdBuilder, VisVersion
from vista_sdk.local_id_parsing_error_builder import LocalIdParsingErrorBuilder

vis = VIS()
gmod = vis.get_gmod(VisVersion.v3_4a)
codebooks = vis.get_codebooks(VisVersion.v3_4a)
locations = vis.get_locations(VisVersion.v3_4a)

error_builder = LocalIdParsingErrorBuilder()
local_id_str = "/dnv-v2/vis-3-4a/invalid/path/meta/qty-temperature"

try:
    local_id = LocalIdBuilder.try_parse(
        local_id_str, gmod, codebooks, locations, error_builder
    )
    if error_builder.has_errors:
        print("Parsing errors:")
        for error in error_builder.errors:
            print(f"  - {error.message}")
except Exception as e:
    print(f"Parsing failed: {e}")
```

### Working with Different VIS Versions

```python
from vista_sdk import VIS, VisVersion

vis = VIS()

# Get all available versions
available_versions = [VisVersion.v3_4a, VisVersion.v3_5a, VisVersion.v3_6a,
                     VisVersion.v3_7a, VisVersion.v3_8a, VisVersion.v3_9a]

# Load data for multiple versions
version_data = {}
for version in available_versions:
    version_data[version] = {
        'gmod': vis.get_gmod(version),
        'codebooks': vis.get_codebooks(version),
        'locations': vis.get_locations(version)
    }

print(f"Loaded data for {len(version_data)} VIS versions")
```

### MQTT Integration

```python
from vista_sdk.mqtt import MqttLocalId
from vista_sdk import LocalIdBuilder, VisVersion

# Create a Local ID
builder = LocalIdBuilder.create(VisVersion.v3_4a)
local_id = (builder
    .with_primary_item_from_path("411.1/C101.31")
    .with_quantity_tag("temperature")
    .build())

# Convert to MQTT format
mqtt_id = MqttLocalId(local_id.builder)
print(f"MQTT Topic: {mqtt_id}")  # Uses underscores instead of slashes
```

## 🧪 Testing

### Running Tests
```bash
# Run all tests
uv run pytest

# Run with coverage
uv run pytest --cov=vista_sdk --cov-report=html

# Run specific test categories
uv run pytest tests/test_gmod.py -v
uv run pytest tests/test_local_id.py -v
```

### Running Benchmarks
```bash
# Run all benchmarks
python run_benchmarks.py

# Run specific benchmark group
python run_benchmarks.py --group gmod

# Run with memory profiling
python run_benchmarks.py --memory --save-data

# Unix-style runner
./benchmark.sh run all
```

## 📈 Performance

The Python implementation includes comprehensive benchmarks that mirror the C# implementation:

- **GMOD Operations**: Path parsing, traversal, versioning
- **Codebook Lookups**: Tag creation and validation
- **Local ID Processing**: Parsing and building
- **Transport**: JSON serialization performance

See [BENCHMARK_IMPLEMENTATION_SUMMARY.md](BENCHMARK_IMPLEMENTATION_SUMMARY.md) for detailed performance analysis.

## 🛠️ Development

### Setting up Development Environment

```bash
# Clone and setup
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/python

# Install development dependencies
uv sync

# Install pre-commit hooks
uv run pre-commit install

# Run tests
uv run pytest

# Run type checking
uv run mypy src/vista_sdk

# Run linting
uv run ruff check src/vista_sdk
```

### Project Structure

```
python/
├── src/vista_sdk/          # Main package
│   ├── __init__.py         # Public API exports
│   ├── vis.py             # Main VIS class
│   ├── gmod.py            # GMOD implementation
│   ├── codebooks.py       # Codebook handling
│   ├── local_id.py        # Local ID implementation
│   ├── mqtt/              # MQTT-specific modules
│   └── system_text_json/  # JSON serialization
├── tests/                  # Test suite
├── samples/               # Usage examples
└── docs/                  # Additional documentation
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests for new functionality
5. Run the test suite (`uv run pytest`)
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🔗 Links

- **Documentation**: [docs.vista.dnv.com](https://docs.vista.dnv.com)
- **GitHub Repository**: [vista-sdk](https://github.com/dnv-opensource/vista-sdk)
- **PyPI Package**: [vista-sdk](https://pypi.org/project/vista-sdk/)
- **Issues**: [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)

## 📞 Support

For questions and support:
- Create an issue on [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)
- Check the [documentation](https://docs.vista.dnv.com)
- Review the [samples](samples/) directory for examples
