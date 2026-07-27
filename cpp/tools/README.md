# Vista SDK C++ - Command-Line Tools

Interactive CLI utilities for exploring and validating DNV VIS (Vessel Information Structure) data.

## codebooks-cli - VIS Codebooks Explorer

Browse and search VIS codebook standard values:

```bash
codebooks-cli                       # List all codebooks (latest version)
codebooks-cli --version 3-4a        # Use specific VIS version
codebooks-cli --codebook qty        # Show Quantity codebook values
codebooks-cli --search pressure     # Search across all codebooks
codebooks-cli --help                # Show all options
```

## locations-cli - VIS Location Codes Explorer

Explore VIS location codes and definitions:

```bash
locations-cli                      # List all location codes (latest version)
locations-cli --version 3-4a       # Use specific VIS version
locations-cli --code P             # Show details for location 'P' (Port)
locations-cli --search starboard   # Search location names/definitions
locations-cli --help               # Show all options
```

## localid-cli - Local ID Parser and Validator

Parse and validate Vista Local ID strings:

```bash
localid-cli --parse "/dnv-v2/vis-3-4a/411.1/meta/qty-temperature"   # Parse and display components
localid-cli --validate localids.txt                                 # Validate LocalIds from file
localid-cli --query localids.txt --tag qty temperature              # Filter LocalIds with query
localid-cli --help                                                  # Show all options
```

## gmod-cli - VIS GMOD Tree Explorer

Navigate and explore the Generic Product Model hierarchy:

```bash
gmod-cli                     # Show tree overview (latest version)
gmod-cli --version 3-4a      # Use specific VIS version
gmod-cli --code 411.1        # Show node details
gmod-cli --search engine     # Search nodes by name/definition
gmod-cli --tree 400a 2       # Show subtree (depth 2)
gmod-cli --path "411.1-1P"   # Parse and validate GMOD path
gmod-cli --help              # Show all options
```

## json-validator-cli - ISO 19848 JSON Validator

Validate JSON files against ISO 19848 schemas:

```bash
json-validator-cli datachannel DataChannelList.json                 # Validate DataChannelList
json-validator-cli timeseries TimeSeriesData.json                   # Validate TimeSeriesData
json-validator-cli cross DataChannelList.json TimeSeriesData.json   # Cross-validate TSD against DCL
json-validator-cli --help                                           # Show all options
```

## Usage Examples

### Example: Searching Codebooks

```bash
$ ./codebooks-cli --search pressure
Searching for 'pressure' in VIS 3-11a codebooks...

Quantity                       ('qty       ') :  7 match(es)
  - partial.pressure
  - sound.pressure
  - pressure
  - static.pressure
  - gauge.pressure
  - differential.pressure
  - relative.pressure.coefficient

Functional Services            ('funct.svc ') :  2 match(es)
  - low.pressure
  - high.pressure
```

### Example: Looking Up Location Codes

```bash
$ ./locations-cli --code S
=== VIS Location Code ===
Version: 3-11a

Code:       S
Name:       Starboard
Definition: A function / product pair located on the starboard side of other corresponding function / product pairs.
```

### Example: Parsing Local IDs

```bash
$ ./localid-cli --parse "/dnv-v2/vis-3-4a/411.1/meta/qty-temperature"
Parsing LocalId: /dnv-v2/vis-3-4a/411.1/meta/qty-temperature
================================================================================

VIS Version:    3-4a
Naming Rule:    dnv-v2
Verbose:        /dnv-v2/vis-3-4a/411.1/~propulsion.driver/meta/qty-temperature

Primary Item:
  Short path:   411.1
  Full path:    VE/400a/410/411/411i/411.1

  Path breakdown:
    VE              - Vessel
    400a            - Propulsion and steering
    410             - Propulsion, conventional
    411             - Providing rotational power to the conventional propulsion lines
    411i            - Providing rotational power to a conventional propulsion line and supporting the driver
    411.1           - Conventional propulsion line driving

Secondary Item: (none)

Metadata Tags:  1 tag(s)
  - qty             : temperature

Full String:    /dnv-v2/vis-3-4a/411.1/meta/qty-temperature
```

### Example: Exploring GMOD Nodes

```bash
$ ./gmod-cli --code 411.1
Node Details: 411.1
============================================================

Code:       411.1
Category:   ASSET FUNCTION
Type:       LEAF
Name:       Conventional propulsion line driving
Common:     Propulsion driver
Definition: Converting input energy to rotational mechanical energy for driving a conventional propulsion line.
Install:    1

Hierarchy:
Is root:    false
Parents (1):
  - 411i (Providing rotational power to a conventional propulsion line and supporting the driver)
Children (1):
  - CS1 (Driver)
Product Selection: CS1 (Driver)
```

### Example: Validating JSON

```bash
$ ./json-validator-cli datachannel DataChannelList.sample.json
Validating: "DataChannelList.sample.json"
Schema: DataChannelList.schema.json

[OK] Validation succeeded
     File conforms to DataChannelList.schema.json
     Schema: DataChannelListPackage (2020-12)
```

## Building

```bash
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_BUILD_TOOLS=ON
cmake --build build -j$(nproc)
```

## See Also

- [Main SDK Documentation](../README.md)
- [Sample Programs](../samples/README.md)
- [Benchmark Results](../benchmarks/README.md)
