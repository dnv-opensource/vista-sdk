# Vista SDK Python Samples

This directory contains practical examples demonstrating how to use the Vista SDK Python implementation. Each sample focuses on specific aspects of the SDK and can be run independently.

## 📁 Sample Files

### 🚀 [`basic_usage.py`](basic_usage.py)

**Perfect starting point** - Demonstrates fundamental operations:

- Initializing VIS and loading data
- Parsing GMOD paths
- Working with codebooks and metadata tags
- Building and parsing Local IDs

```bash
python samples/basic_usage.py
```

### 🔧 [`codebooks_example.py`](codebooks_example.py)

**Deep dive into codebooks** - Shows how to:

- Explore available codebooks and their content
- Create standard and custom metadata tags
- Validate tag values
- Handle position validation special cases

```bash
python samples/codebooks_example.py
```

### 🏗️ [`gmod_operations.py`](gmod_operations.py)

**GMOD (Generic Product Model) operations** - Covers:

- Parsing and analyzing GMOD paths
- Exploring node hierarchies and metadata
- Working with different VIS versions
- Path validation and error handling

```bash
python samples/gmod_operations.py
```

### 🎯 [`advanced_local_ids.py`](advanced_local_ids.py)

**Advanced Local ID operations** - Demonstrates:

- Building complex Local IDs with multiple components
- Error handling and validation
- Working with custom tags
- Builder pattern variations and safe operations

```bash
python samples/advanced_local_ids.py
```

### 🔍 [`local_id_query.py`](local_id_query.py)

**Local ID Query operations** - Shows how to:

- Use GmodPathQuery to match paths with or without locations
- Use MetadataTagsQuery to filter by metadata tags
- Combine path and tag queries with LocalIdQuery
- Use NodesConfig and PathConfig for flexible matching
- Filter sensor data in practical scenarios

```bash
python samples/local_id_query.py
```

### 🌳 [`gmod_subset.py`](gmod_subset.py)

**Asset Model / Digital Twin structure** - Demonstrates:

- Building an asset model tree from GmodPaths
- Creating hierarchical visualizations
- Extracting unique nodes across multiple paths
- Generating LocalIds for sensor data

```bash
python samples/gmod_subset.py
```

### 📊 [`iso19848_json.py`](iso19848_json.py)

**ISO19848 JSON serialization** - Shows how to:

- Load DataChannelList and TimeSeriesData from JSON
- Convert between JSON DTOs and domain models
- Look up data channels by ShortId and LocalId
- Validate TimeSeriesData against DataChannelList

```bash
python samples/iso19848_json.py
```

### 🔄 [`sensor_data_flow.py`](sensor_data_flow.py)

**Sensor data transformation** - Demonstrates:

- Transforming proprietary sensor data into ISO19848 format
- Using DataChannelList as mapping between system IDs and LocalIds
- Grouping channels by update_cycle into TabularData
- Producing ISO19848 TimeSeriesDataPackage

```bash
python samples/sensor_data_flow.py
```

## 🚀 Running the Samples

### Prerequisites

Make sure you have the Vista SDK installed:

```bash
# If running from the repository
cd python
uv sync

# If using pip
pip install vista-sdk
```

### Running Individual Samples

```bash
# Run from the python directory
python samples/basic_usage.py
python samples/codebooks_example.py
python samples/gmod_operations.py
python samples/advanced_local_ids.py
```

### Running All Samples

```bash
# Run all samples in sequence
for sample in samples/*.py; do
    echo "Running $sample..."
    python "$sample"
    echo "---"
done
```

## 📚 What You'll Learn

### From `basic_usage.py`:

- How to initialize the VIS system
- Basic GMOD path parsing
- Creating simple Local IDs
- Working with metadata tags

### From `codebooks_example.py`:

- Understanding different codebook types
- Creating and validating tags
- Working with custom values
- Position validation rules

### From `gmod_operations.py`:

- GMOD structure and navigation
- Node properties and relationships
- Version-specific operations
- Path validation techniques

### From `advanced_local_ids.py`:

- Complex Local ID construction
- Error handling strategies
- Builder pattern best practices
- Verbose mode usage

### From `local_id_query.py`:

- GmodPathQuery for path matching
- MetadataTagsQuery for tag filtering
- LocalIdQuery for combined queries
- NodesConfig and PathConfig usage
- Practical filtering scenarios

### From `gmod_subset.py`:

- Building asset models from GmodPaths
- Tree structure creation and navigation
- Path hierarchy extraction
- Digital twin foundations

### From `iso19848_json.py`:

- ISO19848 data structures
- JSON serialization/deserialization
- Domain model conversion
- Data channel lookups

### From `sensor_data_flow.py`:

- Proprietary to ISO19848 transformation
- DataChannelList as a mapping source
- TabularData grouping strategies
- TimeSeriesDataPackage creation

## 🔧 Customizing the Examples

Feel free to modify these samples to experiment with:

- **Different VIS versions**: Change `VisVersion.v3_4a` to other versions
- **Different paths**: Try your own GMOD paths
- **Custom metadata**: Create your own tag combinations
- **Error scenarios**: Test with invalid inputs to see error handling

## 🛠️ Common Patterns

All samples follow these patterns:

### 1. VIS Initialization

```python
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion

vis = VIS()
version = VisVersion.v3_4a
gmod = vis.get_gmod(version)
codebooks = vis.get_codebooks(version)
locations = vis.get_locations(version)
```

### 2. Error Handling

```python
try:
    result = some_operation()
    print(f"✓ Success: {result}")
except Exception as e:
    print(f"✗ Error: {e}")
```

### 3. Builder Pattern

```python
quantity_tag = codebooks.create_tag(CodebookName.Quantity, "temperature")

local_id = (LocalIdBuilder.create(version)
    .with_primary_item(path)
    .with_metadata_tag(quantity_tag)
    .build())
```

## 🐛 Troubleshooting

### Common Issues:

1. **Import Errors**: Make sure vista-sdk is installed
2. **Path Not Found**: Check that GMOD paths are valid for the VIS version
3. **Tag Creation Fails**: Verify tag values against codebook standards

### Getting Help:

- Check the main [README](../README.md) for installation help
- Review the [documentation](https://docs.vista.dnv.com)
- Look at the test files in `tests/` for more examples

## 🔗 Next Steps

After running these samples:

1. **Explore the API**: Look at the source code in `src/vista_sdk/`
2. **Run the tests**: Execute `uv run pytest` to see comprehensive examples
3. **Check benchmarks**: Run `python run_benchmarks.py` for performance examples
4. **Build your own**: Use these patterns in your own applications

## 📄 License

These samples are part of the Vista SDK and are licensed under the MIT License.
