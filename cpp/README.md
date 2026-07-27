# Vista SDK - C++ Implementation

[![C++ Workflow Status](https://img.shields.io/github/actions/workflow/status/dnv-opensource/vista-sdk/cpp-build-and-test.yml?branch=main&label=C%2B%2B+Build+%26+Test)](https://github.com/dnv-opensource/vista-sdk/actions)
[![GitHub](https://img.shields.io/github/license/dnv-opensource/vista-sdk?style=flat-square)](../LICENSE)

The C++ implementation of the Vista SDK. For an overview of the SDK and its concepts, see the [main README](https://github.com/dnv-opensource/vista-sdk/blob/main/README.md).

> **Standalone**: this SDK has no runtime dependencies. A C++20 compiler and the standard library are the only requirements.

## Table of Contents

- [Installation](#-installation)
- [Types](#-types)
- [StringBuilder](#️-stringbuilder)
- [Quick Start](#-quick-start)
- [Core Components](#-core-components)
- [Query API](#-query-api)
- [Advanced Usage](#-advanced-usage)
- [ISO 19848 Transport](#-iso-19848-transport)
- [Build Options](#-build-options)
- [Build Pipeline](#️-build-pipeline)
- [Testing](#-testing)
- [Performance](#-performance)
- [Tools](#-tools)
- [Error Handling](#️-error-handling)
- [Development](#️-development)
- [For Maintainers](#-for-maintainers)
- [Contributing](#-contributing)
- [License](#-license)
- [Links](#-links)
- [Support](#-support)

## 📦 Installation

### Supported platforms

| OS      | Architecture  | Compiler                     |
| ------- | ------------- | ---------------------------- |
| Linux   | x86_64, ARM64 | GCC 10+, Clang 10+           |
| Windows | x86_64        | MSVC 19.26+, MinGW GCC 14.2+ |

### Prerequisites

- **CMake** 3.25 or later
- **C++20 compiler** with concepts support:
  - GCC 10+ (tested: GCC 12.2.0, GCC 14.2.0)
  - Clang 10+ (tested: Clang 16.0.6, Clang 19.1.7)
  - MSVC 19.26+ (tested: MSVC 19.44)
  - MinGW GCC 14.2.0 (Windows)

### Build from Source

**Linux (GCC, Clang, single-config generators)**

```bash
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk

# Configure
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release

# Build the library
cmake --build build -j$(nproc)
```

**Windows (MSVC, multi-config generator)**

```bash
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk

# Configure
cmake -B build -S cpp

# Build the library
cmake --build build --config Release
```

The SDK builds as a static library by default. Pass `-DBUILD_SHARED_LIBS=ON` to build a shared library (`.so` / `.dll`) instead.

### Consuming via FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    dnv-vista-sdk
    GIT_REPOSITORY https://github.com/dnv-opensource/vista-sdk.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  cpp
)

FetchContent_MakeAvailable(dnv-vista-sdk)

target_link_libraries(your_app PRIVATE dnv::vista::sdk)
```

### Consuming via git submodule

```bash
git submodule add --depth 1 https://github.com/dnv-opensource/vista-sdk.git 3rdParty/vista-sdk
```

```cmake
add_subdirectory(3rdParty/vista-sdk/cpp)
target_link_libraries(your_app PRIVATE dnv::vista::sdk)
```

### Installing and consuming via CMake

The SDK supports the standard CMake install/export workflow (Linux shown below. On Windows/MSVC, drop `-DCMAKE_BUILD_TYPE` and add `--config Release` to both the build and install steps, per the multi-config note above):

```bash
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build -j$(nproc)
cmake --install build
```

> Omitting `-DCMAKE_INSTALL_PREFIX` installs to the system default (`/usr/local` on Linux), which usually needs `sudo`.

Then, from a downstream CMake project:

```cmake
find_package(dnv-vista-sdk REQUIRED)
target_link_libraries(your_app PRIVATE dnv::vista::sdk)
```
The installed/linked version can also be queried at compile time or runtime:

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

// Compile-time
static_assert(
    DNV_VISTA_SDK_VERSION_MAJOR > 0 || DNV_VISTA_SDK_VERSION_MINOR >= 3, "requires vista-sdk-cpp >= 0.3");

// Runtime
std::cout << dnv::vista::sdk::version() << "\n"; // e.g. "0.3.3"
```

#### Uninstalling

```bash
cmake --build build --target uninstall
```

## 🔢 Types

The SDK provides a handful of standalone value types, usable independently of the rest of the SDK. See the [types samples](samples/README.md#types) for more usage examples.

### DateTime / DateTimeOffset

`DateTime` (UTC-only) and `DateTimeOffset` (timezone-aware) represent timestamps with 100-nanosecond precision and ISO 8601 parsing/formatting (ISO 19848 Table 8). Both support a range of January 1, 0001 to December 31, 9999.

```cpp
auto now = DateTimeOffset::now();
std::cout << now.toString(DateTime::Format::Iso8601) << "\n"; // "2026-08-23T19:11:23+02:00"

auto parsed = DateTimeOffset::fromString("2026-08-23T19:11:23Z");
if (parsed.has_value())
{
    std::cout << parsed.value().toString() << "\n"; // 2026-08-23T19:11:23Z
}
```

### TimeSpan

Represents a duration (not a point in time) with 100-nanosecond precision, formatted per the ISO 8601 duration grammar (`P[n]DT[n]H[n]M[n]S`), used as the `TimeSpan` type in ISO 19848 TimeSeriesData (Table 25).

```cpp
auto duration = TimeSpan::fromHours(1.5);
std::cout << duration.toString() << "\n"; // "PT1H30M"
```

### Decimal

Exact decimal arithmetic with no floating-point rounding, implementing XSD `xs:decimal` semantics (ISO 19848 Table 2). Stores values as `mantissa / 10^scale` (a 96-bit mantissa and a scale in `[0, 28]`) instead of a binary float, so values like `0.1 + 0.2` round-trip exactly.

```cpp
Decimal fuelConsumed{ "12.450" };                               // tonnes
Decimal pricePerTonne{ "615.30" };                              // USD
std::cout << (fuelConsumed * pricePerTonne).toString() << "\n"; // "7660.485", exact
```

## ✍️ StringBuilder

`StringBuilder` is a growable character buffer used throughout the SDK to avoid the repeated small allocations that come from building strings piece by piece. It uses a fixed-size stack buffer (256 bytes) for small content, transparently falling back to a heap allocation once that capacity is exceeded. See the [StringBuilder sample](samples/README.md#utils) for more usage examples.

Every `toString()` on the SDK's core types (`GmodNode`, `GmodPath`, `MetadataTag`, `LocalId`, `LocalIdBuilder`, `UniversalId`, `UniversalIdBuilder`, `DateTime`, `DateTimeOffset`, `TimeSpan`, `Decimal`) has a `StringBuilder&`-overload alongside the convenience `std::string`-returning one:

```cpp
#include <dnv/VistaSDK.h>

using namespace dnv::vista::sdk;

StringBuilder sb;

// Reused across many calls: each toString() appends, no per-call allocation
// as long as content stays under the stack buffer's capacity.
for (const auto& node : someNodes)
{
    node.toString(sb);
    // ... consume sb.view() ...
    sb.clear(); // keeps capacity, resets length to 0
}
```

## 🚀 Quick Start

> 💡 For more complete examples, see the [samples](samples/) directory and [samples/README.md](samples/README.md).

### Basic Usage

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    std::cout << "SDK version: " << version() << "\n";

    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(vis.latest());
    const auto& codebooks = vis.codebooks(vis.latest());
    const auto& locations = vis.locations(vis.latest());

    // Look up a node
    const auto& node = gmod["411.1"];
    std::cout << "Node       : " << node.code() << " - " << node.metadata().commonName().value_or("") << "\n";

    // Parse a Gmod path
    auto path = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
    if (path.has_value())
    {
        std::cout << "Full path  : " << path->toFullPathString() << "\n";
    }

    // Build a Local ID
    auto quantityTag = codebooks[CodebookName::Quantity].createTag("temperature");
    auto contentTag  = codebooks[CodebookName::Content].createTag("exhaust.gas");
    auto positionTag = codebooks[CodebookName::Position].createTag("inlet");

    auto localId = LocalIdBuilder::create(vis.latest())
                       .withPrimaryItem(*path)
                       .withMetadataTag(quantityTag)
                       .withMetadataTag(contentTag)
                       .withMetadataTag(positionTag)
                       .build();

    std::cout << "LocalId    : " << localId.toString() << "\n";

    auto verboseBuilder = localId.builder().withVerboseMode(true);
    std::cout << "Verbose    : " << verboseBuilder.toString() << "\n";

    return 0;
}
```
Output:
```
SDK version: 0.3.3
Node       : 411.1 - Propulsion driver
Full path  : VE/400a/410/411/411i/411.1/CS1/C101/C101.3/C101.3i-2/C101.31-2
LocalId    : /dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet
Verbose    : /dnv-v2/vis-3-11a/411.1/C101.31-2/~propulsion.engine/~cylinder.2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet
```

### Working with Codebooks

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& codebooks = VIS::instance().codebooks(VisVersion::v3_11a);

    const auto& qty = codebooks[CodebookName::Quantity];
    auto tag        = qty.createTag("temperature");
    if (tag.has_value())
    {
        std::cout << "Tag                               : " << tag->toString() << "\n";
    }
    std::cout << std::boolalpha;
    std::cout << "Valid standard value 'temperature': " << qty.hasStandardValue("temperature") << "\n";

    const auto& pos = codebooks[CodebookName::Position];
    std::cout << "Has group 'centre'                : " << pos.hasGroup("centre") << "\n";

    return 0;
}
```

Output:
```
Tag                               : qty-temperature
Valid standard value 'temperature': true
Has group 'centre'                : false
```

### Gmod Path Operations

```cpp
#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(VisVersion::v3_11a);
    const auto& locations = vis.locations(VisVersion::v3_11a);

    auto path = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
    if (path.has_value())
    {
        std::cout << "Short   : " << path->toString() << "\n";
        std::cout << "Full    : " << path->toFullPathString() << "\n";
        std::cout << "Depth   : " << path->length() << "\n";

        for (auto [depth, node] : path->fullPath())
        {
            std::cout << "Depth " << std::setw(2) << depth << ": " << node.code() << " - " << node.metadata().name()
                      << "\n";
        }
    }

    return 0;
}
```

Output:
```
Short   : 411.1/C101.31-2
Full    : VE/400a/410/411/411i/411.1/CS1/C101/C101.3/C101.3i-2/C101.31-2
Depth   : 11
Depth  0: VE - Vessel
Depth  1: 400a - Propulsion and steering
Depth  2: 410 - Propulsion, conventional
Depth  3: 411 - Providing rotational power to the conventional propulsion lines
Depth  4: 411i - Providing rotational power to a conventional propulsion line and supporting the driver
Depth  5: 411.1 - Conventional propulsion line driving
Depth  6: CS1 - Driver
Depth  7: C101 - Reciprocating internal combustion engine
Depth  8: C101.3 - Cylinder and piston units
Depth  9: C101.3i - Cylinder and piston unit
Depth 10: C101.31 - Piston guiding
```

### Version Conversion

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& vis         = VIS::instance();
    const auto& gmod34      = vis.gmod(VisVersion::v3_4a);
    const auto& locations34 = vis.locations(VisVersion::v3_4a);

    auto path34    = GmodPath::fromShortPath("323.51/H362.1", gmod34, locations34);
    auto converted = vis.convertPath(VisVersion::v3_4a, *path34, VisVersion::v3_11a);
    if (converted.has_value())
    {
        std::cout << "Converted: " << converted->toString() << "\n";
    }

    return 0;
}
```

Output:
```
Converted: 323.61/H362.1
```

## 📚 Core Components

For a detailed overview of VIS concepts (Gmod, Codebooks, Locations, etc.), see the [main README](https://github.com/dnv-opensource/vista-sdk/blob/main/README.md).

### VIS (Vessel Information Structure)

Singleton entry point providing thread-safe lazy-loaded access to versioned VIS data.

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(VisVersion::v3_11a);
    const auto& locations = vis.locations(VisVersion::v3_11a);

    std::cout << "Latest version    : " << VisVersions::toString(vis.latest()) << "\n";
    std::cout << "Gmod nodes        : " << std::distance(gmod.begin(), gmod.end()) << "\n";
    std::cout << "Relative locations: " << locations.relativeLocations().size() << "\n";

    return 0;
}
```

Output:
```
Latest version    : 3-11a
Gmod nodes        : 6593
Relative locations: 13
```

### Local ID Builder

Construct standardized local identifiers:

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& vis       = VIS::instance();
    auto version          = vis.latest();
    const auto& gmod      = vis.gmod(version);
    const auto& codebooks = vis.codebooks(version);
    const auto& locations = vis.locations(version);

    auto path        = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
    auto quantityTag = codebooks[CodebookName::Quantity].createTag("temperature");
    auto contentTag  = codebooks[CodebookName::Content].createTag("exhaust.gas");

    auto localId = LocalIdBuilder::create(version)
                       .withPrimaryItem(*path)
                       .withMetadataTag(quantityTag)
                       .withMetadataTag(contentTag)
                       .build();

    std::cout << "LocalId: " << localId.toString() << "\n";

    return 0;
}
```

Output:
```
LocalId: /dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas
```

### Locations

```cpp
#include <dnv/VistaSDK.h>

#include <iomanip>
#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& locations = VIS::instance().locations(VisVersion::v3_11a);

    std::cout << "Version           : " << VisVersions::toString(locations.version()) << "\n";
    std::cout << "Relative locations: " << locations.relativeLocations().size() << "\n";
    std::cout << "Location groups   : " << locations.groups().size() << "\n";

    std::vector<std::string> samples = {
        "1",   // number only
        "1P",  // number + side (Port)
        "2CF", // number + side (Centre) + longitudinal (Forward)
        "3US"  // invalid: wrong order (U before S, must be alphabetical)
    };

    for (const auto& s : samples)
    {
        auto loc = locations.fromString(s);
        std::cout << "  '" << std::left << std::setw(4) << s << "' -> " << (loc.has_value() ? "Valid" : "Invalid")
                  << "\n";
    }

    return 0;
}
```

Output:
```
Version           : 3-11a
Relative locations: 13
Location groups   : 4
  '1   ' -> Valid
  '1P  ' -> Valid
  '2CF ' -> Valid
  '3US ' -> Invalid
```

### Universal ID Builder

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(vis.latest());
    const auto& locations = vis.locations(vis.latest());
    const auto& codebooks = vis.codebooks(vis.latest());

    auto path   = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations);
    auto qtyTag = codebooks[CodebookName::Quantity].createTag("temperature");

    auto universalId =
        UniversalIdBuilder::create(vis.latest())
            .withImoNumber(ImoNumber{ 1234567 })
            .withLocalId(LocalIdBuilder::create(vis.latest()).withPrimaryItem(*path).withMetadataTag(*qtyTag))
            .build();

    std::cout << "UniversalId: " << universalId.toString() << "\n";
    std::cout << "IMO Number : " << universalId.imoNumber().toString() << "\n";
    std::cout << "LocalId    : " << universalId.localId().toString() << "\n";

    auto parsed = UniversalId::fromString(
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas");
    if (parsed.has_value())
    {
        std::cout << "Parsed     : " << parsed->toString() << "\n";
        std::cout << "VIS Version: " << VisVersions::toString(parsed->localId().version()) << "\n";
    }

    return 0;
}
```

Output:
```
UniversalId: data.dnv.com/IMO1234567/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature
IMO Number : IMO1234567
LocalId    : /dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature
Parsed     : data.dnv.com/IMO1234567/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas
VIS Version: 3-11a
```

## 🔎 Query API

`GmodPathQuery`, `MetadataTagsQuery`, and `LocalIdQuery` provide a fluent, immutable builder API for
matching `GmodPath` / `LocalId` instances against structural and metadata criteria.
Useful for filtering incoming sensor data or LocalIds against a set of rules without re-parsing them.

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(vis.latest());
    const auto& locations = vis.locations(vis.latest());

    // Match any LocalId whose primary item is under 411.1/C101, regardless of location,
    // and that carries a qty-temperature tag
    auto basePath = GmodPath::fromShortPath("411.1/C101", gmod, locations);

    auto query =
        LocalIdQueryBuilder::create()
            .withPrimaryItem(*basePath, [](GmodPathQueryBuilder::Path p) { return p.withoutLocations().build(); })
            .withTags([](MetadataTagsQueryBuilder& tags) {
                return tags.withTag(CodebookName::Quantity, "temperature").build();
            })
            .build();

    std::cout << std::boolalpha;
    std::cout << "Matches (same subtree, qty-temperature): "
              << query.match("/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature") << "\n";
    std::cout << "Matches (different quantity)           : "
              << query.match("/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-pressure") << "\n";

    return 0;
}
```

Output:
```
Matches (same subtree, qty-temperature): true
Matches (different quantity)           : false
```

## 🔬 Advanced Usage

### Parsing Local IDs

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    auto parsed = LocalId::fromString("/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas");
    if (parsed.has_value())
    {
        std::cout << "Primary item: " << parsed->primaryItem().toString() << "\n";
        if (parsed->quantity().has_value())
        {
            std::cout << "Quantity    : " << parsed->quantity()->toString() << "\n";
        }
        if (parsed->content().has_value())
        {
            std::cout << "Content     : " << parsed->content()->toString() << "\n";
        }
    }

    std::cout << "LocalId     : " << parsed->toString() << "\n";

    return 0;
}
```

Output:
```
Primary item: 411.1/C101.31-2
Quantity    : qty-temperature
Content     : cnt-exhaust.gas
LocalId     : /dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas
```

### Gmod Traversal

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;

int main()
{
    const auto& gmod = VIS::instance().gmod(VisVersion::v3_11a);

    int count = 0;
    gmod.traverse([&count](const TraversalPath& parents, const GmodNode& node) {
        if (count >= 20)
        {
            return TraversalHandlerResult::Stop;
        }
        const auto& name = node.metadata().commonName();
        std::cout << std::string(parents.size() * 2, ' ') << node.code() << ": " << (name.has_value() ? *name : "")
                  << "\n";
        ++count;
        return TraversalHandlerResult::Continue;
    });

    return 0;
}
```

Output:
```
VE: 
  000a: General
    000: Vessel general
      001: Vessel identification
      002: Vessel design and construction
        002.1: Vessel design - precontract
        002.2: Vessel design - newbuilding
        002.3: Vessel design - conversion
        002.4: Vessel construction - newbuilding
        002.5: Vessel construction - conversion
        002.6: Vessel construction - equipment supply
      003: Vessel ownership
      004: Vessel management
        004.1: Vessel management, overall
        004.2: Vessel management, DOC holder
        004.3: Vessel management, invoice handling
        004.4: Vessel management , insurance
      005: Vessel employment
        005.1: Vessel employment, charterer
        005.2: Vessel employment, offshore field operator
// Stop
```

## 🚢 ISO 19848 Transport

### DataChannel list

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;
using namespace dnv::vista::sdk::transport;
using namespace dnv::vista::sdk::transport::datachannel;
namespace json_dc = serialization::json::datachannel;

int main()
{
    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(vis.latest());
    const auto& locations = vis.locations(vis.latest());
    const auto& codebooks = vis.codebooks(vis.latest());

    auto path    = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations).value();
    auto qtyTag  = codebooks[CodebookName::Quantity].createTag("temperature").value();
    auto localId = LocalIdBuilder::create(vis.latest())
                       .withPrimaryItem(path)
                       .withMetadataTag(qtyTag)
                       .build();

    auto dcId      = DataChannelId{ localId, "TEMP001" };
    auto property  = Property{
        DataChannelType{ "Inst", 1.0 }, Format{ "Decimal" }, Range{ 0.0, 600.0 }, Unit{ "°C", "temperature" }
    };
    auto channel   = DataChannel{ std::move(dcId), std::move(property) };

    auto configRef = ConfigurationReference{ "dcl-001", DateTimeOffset{ "2026-08-23T19:11:23Z" } };
    auto shipId    = ShipId::fromString("IMO1234567").value();
    auto header    = Header{ shipId, std::move(configRef) };
    auto dcList    = DataChannelList{};
    dcList.add(std::move(channel));
    auto pkg       = DataChannelListPackage{ Package{ std::move(header), std::move(dcList) } };

    StringBuilder sb;
    json_dc::toJsonString(sb, pkg, true);
    std::cout << sb.view() << "\n";

    return 0;
}
```

Output:
```json
{
  "Package": {
    "Header": {
      "ShipID": "IMO1234567",
      "DataChannelListID": {
        "ID": "dcl-001",
        "TimeStamp": "2026-08-23T19:11:23Z"
      }
    },
    "DataChannelList": {
      "DataChannel": [
        {
          "DataChannelID": {
            "LocalID": "/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature",
            "ShortID": "TEMP001"
          },
          "Property": {
            "DataChannelType": {
              "Type": "Inst",
              "UpdateCycle": 1
            },
            "Format": {
              "Type": "Decimal"
            },
            "Range": {
              "High": 600,
              "Low": 0
            },
            "Unit": {
              "UnitSymbol": "°C",
              "QuantityName": "temperature"
            }
          }
        }
      ]
    }
  }
}
```

### Time series data

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

using namespace dnv::vista::sdk;
using namespace dnv::vista::sdk::transport;
using namespace dnv::vista::sdk::transport::timeseries;
namespace json_ts = dnv::vista::sdk::transport::serialization::json::timeseries;

int main()
{
    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(vis.latest());
    const auto& locations = vis.locations(vis.latest());
    const auto& codebooks = vis.codebooks(vis.latest());

    auto path    = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations).value();
    auto qtyTag  = codebooks[CodebookName::Quantity].createTag("temperature").value();
    auto localId = LocalIdBuilder::create(vis.latest()).withPrimaryItem(path).withMetadataTag(qtyTag).build();

    auto shipId = ShipId::fromString("IMO1234567").value();
    auto start  = DateTimeOffset{ "2026-08-23T20:00:00Z" };
    auto end    = DateTimeOffset{ "2026-08-23T22:00:00Z" };

    auto header =
        timeseries::Header{ shipId,
                            timeseries::TimeSpan{ start, end },
                            start,
                            end,
                            "Vista SDK Sample",
                            std::vector<timeseries::ConfigurationReference>{ timeseries::ConfigurationReference{
                                "dcl-001", DateTimeOffset{ "2026-08-23T19:11:23Z" } } } };

    auto channelId = timeseries::DataChannelId::fromString(localId.toString()).value();

    std::vector<timeseries::DataChannelId> channels{ channelId };
    std::vector<timeseries::TabularDataSet> dataSets{
        timeseries::TabularDataSet{ DateTimeOffset{ "2026-08-23T20:00:00Z" }, { "350.4" }, std::nullopt },
        timeseries::TabularDataSet{ DateTimeOffset{ "2026-08-23T21:00:00Z" }, { "361.2" }, std::nullopt }
    };

    auto dataConfig = timeseries::ConfigurationReference{ "dcl-001", DateTimeOffset{ "2026-08-23T19:11:23Z" } };
    auto tabular    = timeseries::TabularData{ std::move(channels), std::move(dataSets) };
    auto tsd        = timeseries::TimeSeriesData{
        std::move(dataConfig), std::vector<timeseries::TabularData>{ std::move(tabular) }, std::nullopt, std::nullopt
    };
    auto pkg        = timeseries::TimeSeriesDataPackage{ timeseries::Package{
        std::move(header), std::vector<timeseries::TimeSeriesData>{ std::move(tsd) } } };

    StringBuilder sb;
    json_ts::toJsonString(sb, pkg, true);
    std::cout << sb.view() << "\n";

    return 0;
}
```

Output:
```json
{
  "Package": {
    "Header": {
      "ShipID": "IMO1234567",
      "TimeSpan": {
        "Start": "2026-08-23T20:00:00Z",
        "End": "2026-08-23T22:00:00Z"
      },
      "DateCreated": "2026-08-23T20:00:00Z",
      "DateModified": "2026-08-23T22:00:00Z",
      "Author": "Vista SDK Sample",
      "SystemConfiguration": [
        {
          "ID": "dcl-001",
          "TimeStamp": "2026-08-23T19:11:23Z"
        }
      ]
    },
    "TimeSeriesData": [
      {
        "DataConfiguration": {
          "ID": "dcl-001",
          "TimeStamp": "2026-08-23T19:11:23Z"
        },
        "TabularData": [
          {
            "NumberOfDataSet": 2,
            "NumberOfDataChannel": 1,
            "DataChannelID": [
              "/dnv-v2/vis-3-11a/411.1/C101.31-2/meta/qty-temperature"
            ],
            "DataSet": [
              {
                "TimeStamp": "2026-08-23T20:00:00Z",
                "Value": [
                  "350.4"
                ]
              },
              {
                "TimeStamp": "2026-08-23T21:00:00Z",
                "Value": [
                  "361.2"
                ]
              }
            ]
          }
        ]
      }
    ]
  }
}
```

### Cross-validation with domain rules

`TimeSeriesData::validate` checks structural conformance against a `DataChannelListPackage`
(channel IDs exist, values match their declared `Format`, `Range`, and `Restriction`), but it
also accepts two callbacks (`onTabularData` and `onEventData`) invoked once per data point
**after** that standard validation passes. This is the place to plug in business rules ISO 19848
doesn't know about: dynamic thresholds, cross-channel correlation, alerting, audit logging, etc.
Each callback receives the timestamp, the matched `DataChannel`, the parsed `Value`, and the
optional quality code, and returns a `ValidateResult<>`: returning `invalid(...)` fails that
data point (its message is folded into the overall validation result) without throwing.

```cpp
#include <dnv/VistaSDK.h>

#include <iostream>

int main()
{
    using namespace dnv::vista::sdk;
    using namespace dnv::vista::sdk::transport;

    const auto& vis       = VIS::instance();
    const auto& gmod      = vis.gmod(vis.latest());
    const auto& locations = vis.locations(vis.latest());
    const auto& codebooks = vis.codebooks(vis.latest());

    auto path    = GmodPath::fromShortPath("411.1/C101.31-2", gmod, locations).value();
    auto qtyTag  = codebooks[CodebookName::Quantity].createTag("temperature").value();
    auto localId = LocalIdBuilder::create(vis.latest()).withPrimaryItem(path).withMetadataTag(qtyTag).build();

    // DataChannelListPackage
    auto shipId    = ShipId::fromString("IMO1234567").value();
    auto configRef = datachannel::ConfigurationReference{ "dcl-001", DateTimeOffset{ "2026-08-23T00:00:00Z" } };
    auto header    = datachannel::Header{ shipId, configRef };

    auto dcId     = datachannel::DataChannelId{ localId, "TEMP001" };
    auto range    = datachannel::Range{ 0.0, 200.0 };
    auto property = datachannel::Property{ datachannel::DataChannelType{ "Inst" },
                                           datachannel::Format{ "Decimal" },
                                           range,
                                           datachannel::Unit{ "°C" },
                                           std::nullopt,
                                           "High", // alertPriority
                                           std::nullopt,
                                           std::nullopt,
                                           std::nullopt };

    auto channel = datachannel::DataChannel{ dcId, property };
    auto dcList  = datachannel::DataChannelList{ std::vector<datachannel::DataChannel>{ channel } };
    auto pkg     = datachannel::DataChannelListPackage{ datachannel::Package{ header, dcList } };

    // TimeSeriesData
    auto tsConfig    = timeseries::ConfigurationReference{ "dcl-001", DateTimeOffset{ "2026-08-23T00:00:00Z" } };
    auto tsChannelId = timeseries::DataChannelId::fromString("TEMP001").value();

    auto tabular =
        timeseries::TabularData{ std::vector<timeseries::DataChannelId>{ tsChannelId },
                                 std::vector<timeseries::TabularDataSet>{ timeseries::TabularDataSet{
                                     DateTimeOffset{ "2026-08-23T10:00:00Z" }, { "185.0" }, std::nullopt } } };
    auto tsd = timeseries::TimeSeriesData{
        tsConfig, std::vector<timeseries::TabularData>{ tabular }, std::nullopt, std::nullopt
    };

    // Custom validation callback
    auto onTabularData = [](const DateTimeOffset& /*timestamp*/,
                            const datachannel::DataChannel& channel,
                            const Value& value,
                            const std::optional<std::string>& /*qualityCode*/) {
        if (auto dec = value.decimal(); dec && channel.property().range())
        {
            if (*dec > channel.property().range()->high() * 0.9)
            {
                const auto& priority = channel.property().alertPriority();
                return ValidateResult<>::invalid(
                    "Value " + value.toString() + " triggered alarm '" + priority.value_or("unclassified") + "'");
            }
        }
        return ValidateResult<>::ok();
    };

    auto onEventData =
        [](const DateTimeOffset&, const datachannel::DataChannel&, const Value&, const std::optional<std::string>&) {
            return ValidateResult<>::ok();
        };

    auto result = tsd.validate(pkg, onTabularData, onEventData);

    if (!result)
    {
        for (const auto& error : result.errors())
        {
            std::cout << error << "\n";
        }
    }

    return result ? 0 : 1;
}
```

Output:
```
DataChannel TEMP001 validation failed: Value 185 triggered alarm 'High'
```

See the [`timeseriesdata` sample](samples/transport/timeseriesdata.cpp) (section 8) for a
complete, runnable example.

### JSON interoperability

The SDK ships its own minimal JSON implementation (`src/JSON/`) and has **no
runtime dependency on any third-party JSON library**. ISO 19848 defines several
`xs:any` extension points (`CustomHeaders`, `CustomNameObjects`,
`CustomElements`, `CustomProperties`, `CustomData`) that carry arbitrary
user-defined JSON content. Rather
than forcing a specific library on consumers, the SDK exposes a
**Customization Point Object (CPO)** boundary via `SerializableDocument`
(`transport/serialization/json/SerializableDocument.h`).

To plug in your own JSON type (nlohmann, rapidjson, simdjson, boost.json, ...),
provide two `tag_invoke` overloads **in your type's own namespace** so ADL can
find them:

```cpp
// In your namespace (ADL must be able to find this)
namespace my_app
{
    // Your JSON type -> SerializableDocument
    SerializableDocument tag_invoke(
        dnv::vista::sdk::transport::serialization::json::ToSerializableDocumentTag,
        const MyJsonValue& v)
    {
        // walk v and build a SerializableDocument
        if (v.is_string())
        {
            return SerializableDocument{ v.get_string() };
        }
        // ... etc.
    }

    // SerializableDocument -> your JSON type
    void tag_invoke(
        dnv::vista::sdk::transport::serialization::json::FromSerializableDocumentTag,
        const SerializableDocument& doc,
        MyJsonValue& out)
    {
        // walk doc and populate out
        if (doc.isString())
        {
            out = MyJsonValue{ doc.asString() };
        }
        // ... etc.
    }
}
```

Once adapted, pass your JSON value through the SDK's extension points via
`SerializableDocument::from<MyJsonValue>(v)` and `doc.to<MyJsonValue>()`.

The CPO is only needed for `xs:any` extension content. The SDK's own
serialization path (`toJsonString`, shown above) speaks `SerializableDocument`
internally and requires no adaptation.

## 🔧 Build Options

```cmake
option(DNV_VISTA_SDK_BUILD_TOOLS        "Build CLI tools"                                  OFF)
option(DNV_VISTA_SDK_BUILD_TESTS        "Build tests"                                      OFF)
option(DNV_VISTA_SDK_BUILD_SAMPLES      "Build samples"                                    OFF)
option(DNV_VISTA_SDK_BUILD_BENCHMARKS   "Build benchmarks"                                 OFF)
option(DNV_VISTA_SDK_BUILD_DOCS         "Build API documentation (requires Doxygen)"       OFF)
option(DNV_VISTA_SDK_WARNINGS           "Enable strict compiler warnings"                  OFF)
option(DNV_VISTA_SDK_WARNINGS_AS_ERRORS "Treat compiler and Doxygen warnings as errors"    OFF)
option(DNV_VISTA_SDK_ENABLE_SIMD        "Enable hardware SIMD acceleration (non-portable)" OFF)
```

> **API documentation**: pass `-DDNV_VISTA_SDK_BUILD_DOCS=ON` to enable the `dnv-vista-sdk-docs` target (requires Doxygen).
Graphviz (`dot`) is optional: if found, dependency graphs are included, otherwise they're silently skipped.
The doc is not built automatically:
> ```bash
> cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_BUILD_DOCS=ON
> cmake --build build --target dnv-vista-sdk-docs
> ```
> Output is written to `build/docs/html/`.

> **Hardware acceleration**: the library uses a portable software CRC32-C implementation by default. To enable hardware acceleration (SSE4.2 on GCC/Clang, AVX on MSVC), pass `-DDNV_VISTA_SDK_ENABLE_SIMD=ON` when building the library:
> ```bash
> cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_ENABLE_SIMD=ON
> ```
> This adds `-march=native` (GCC/Clang) or `/arch:AVX2` (MSVC) to the library target. The resulting binary is tied to the build machine's CPU and should not be distributed as a generic artifact.

> All `cmake --build build` commands below assume Linux. **On Windows/MSVC**, drop `-DCMAKE_BUILD_TYPE=Release` from the configure step and append `--config Release` to every `cmake --build build` command instead, per the multi-config note above.

### Build Everything

```bash
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk

cmake -B build -S cpp \
    -DCMAKE_BUILD_TYPE=Release \
    -DDNV_VISTA_SDK_BUILD_SAMPLES=ON \
    -DDNV_VISTA_SDK_BUILD_TESTS=ON \
    -DDNV_VISTA_SDK_BUILD_TOOLS=ON \
    -DDNV_VISTA_SDK_BUILD_BENCHMARKS=ON

cmake --build build -j$(nproc)
```

## 🏗️ Build Pipeline

All VIS data (`resources/*.json.gz` at the repository root, shared across all SDK
implementations) is embedded at compile time into the binary. The resulting
`.a`/`.so`/`.dll` is self-contained: no data files to deploy or manage at runtime,
and all VIS versions coexist in the same binary (`VIS::instance().gmod(VisVersion::v3_4a)`
and `VIS::instance().gmod(VisVersion::v3_11a)` are both available without any extra step).

To update VIS data without pulling new SDK code (e.g. a new VIS version was published
upstream but the C++ code is stable):

```bash
git checkout origin/main -- resources/
cmake --build build -j$(nproc)  # re-embeds the updated resources
```

The SDK uses a multi-stage code generation pipeline that runs automatically during the build:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ Stage 1: Resource Embedding                                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│              resources/*.json.gz → dnv_vista_sdk_embed_blobs()              │
│                                      ↓                                      │
│                            dnv-vista-sdk-resources                          │
│                            (CMake OBJECT library)                           │
│                                                                             │
│     Embeds compressed JSON resources as C++ byte arrays in object files     │
└─────────────────────────────────────────────────────────────────────────────┘
                   ┌───────────────────┴───────────────────┐
                   ↓                                       ↓
┌─────────────────────────────────────┐ ┌─────────────────────────────────────┐
│ Stage 2A: VIS Versions Generation   │ │ Stage 2B: ISO Versions Generation   │
├─────────────────────────────────────┤ ├─────────────────────────────────────┤
│       dnv-vista-sdk-resources       │ │       dnv-vista-sdk-resources       │
│                  ↓                  │ │                  ↓                  │
│     visversionsgenerator (exe)      │ │   iso19848versionsgenerator (exe)   │
│                  ↓                  │ │                  ↓                  │
│         VisVersions.h               │ │      ISO19848Versions.h             │
│         VisVersionsExtensions.h     │ │      ISO19848VersionsExtensions.h   │
│                                     │ │                                     │
│  Generates VIS versions enums and   │ │ Generates ISO 19848 versions enums  │
│  utilities from embedded VIS data   │ │ and utilities from embedded data    │
└─────────────────────────────────────┘ └─────────────────────────────────────┘
                   └───────────────────┬───────────────────┘
                                       ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│ Stage 3: SDK Library Compilation                                            │
├─────────────────────────────────────────────────────────────────────────────┤
│      Generated headers + SDK sources + dnv-vista-sdk-resources objects      │
│                                      │                                      │
│    BUILD_SHARED_LIBS=OFF (default)   │        BUILD_SHARED_LIBS=ON          │
│                  ↓                   │                   ↓                  │
│      libdnv-vista-sdk.a (Linux)      │  libdnv-vista-sdk.so.X.Y.Z (Linux)   │
│      dnv-vista-sdk.lib  (Windows)    │  dnv-vista-sdk.dll         (Windows) │
└─────────────────────────────────────────────────────────────────────────────┘
```

## 🧪 Testing

### Running Tests

```bash
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_BUILD_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build
```

### Running Samples

```bash
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_BUILD_SAMPLES=ON
cmake --build build -j$(nproc)

# Core
./build/bin/dnv-vista-sdk-sample-core-codebooks
./build/bin/dnv-vista-sdk-sample-core-gmod
./build/bin/dnv-vista-sdk-sample-core-gmodtraversal
./build/bin/dnv-vista-sdk-sample-core-gmodpath
./build/bin/dnv-vista-sdk-sample-core-gmodsubset
./build/bin/dnv-vista-sdk-sample-core-gmodversioning
./build/bin/dnv-vista-sdk-sample-core-imonumber
./build/bin/dnv-vista-sdk-sample-core-localid
./build/bin/dnv-vista-sdk-sample-core-locations
./build/bin/dnv-vista-sdk-sample-core-universalid

# Queries
./build/bin/dnv-vista-sdk-sample-query-gmodpathquery
./build/bin/dnv-vista-sdk-sample-query-localidquery
./build/bin/dnv-vista-sdk-sample-query-metadatatagsquery

# Transport
./build/bin/dnv-vista-sdk-sample-transport-datachannellist
./build/bin/dnv-vista-sdk-sample-transport-iso19848
./build/bin/dnv-vista-sdk-sample-transport-sensorsdataflow
./build/bin/dnv-vista-sdk-sample-transport-shipid
./build/bin/dnv-vista-sdk-sample-transport-timeseriesdata

# Types
./build/bin/dnv-vista-sdk-sample-type-datetime
./build/bin/dnv-vista-sdk-sample-type-decimal

# Utilities
./build/bin/dnv-vista-sdk-sample-utils-stringbuilder
```

`samples/wasm/` is a standalone project (its own `CMakeLists.txt`, configured with `emcmake` instead of the commands above) - an interactive Gmod tree explorer compiled to WebAssembly with ImGui + GLFW. See [`samples/wasm/README.md`](samples/wasm/README.md) for build instructions.

## 📈 Performance

The C++ implementation includes benchmarks covering all core operations. See [benchmarks/README.md](benchmarks/README.md) for details.

### Running Benchmarks

```bash
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_BUILD_BENCHMARKS=ON
cmake --build build -j$(nproc)

./build/bin/dnv-vista-sdk-benchmark-codebookmethods
./build/bin/dnv-vista-sdk-benchmark-codebookslookup
./build/bin/dnv-vista-sdk-benchmark-containers
./build/bin/dnv-vista-sdk-benchmark-datachannellistlookup
./build/bin/dnv-vista-sdk-benchmark-datachannellistserialization
./build/bin/dnv-vista-sdk-benchmark-gmodload
./build/bin/dnv-vista-sdk-benchmark-gmodlookup
./build/bin/dnv-vista-sdk-benchmark-gmodpathparse
./build/bin/dnv-vista-sdk-benchmark-gmodtraversal
./build/bin/dnv-vista-sdk-benchmark-gmodversioningconvertpath
./build/bin/dnv-vista-sdk-benchmark-localidparse
./build/bin/dnv-vista-sdk-benchmark-timeseriesdataserialization
```

## 🔍 Tools

Pass `-DDNV_VISTA_SDK_BUILD_TOOLS=ON` to build the CLI utilities:

```bash
cmake -B build -S cpp -DCMAKE_BUILD_TYPE=Release -DDNV_VISTA_SDK_BUILD_TOOLS=ON
cmake --build build -j$(nproc)
```

| Binary                             | Purpose                                                                                                                                      |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------- |
| `dnv-vista-sdk-codebooks-cli`      | Browse and search VIS codebook standard values                                                                                               |
| `dnv-vista-sdk-locations-cli`      | Explore VIS location codes and definitions                                                                                                   |
| `dnv-vista-sdk-localid-cli`        | Parse, validate, and query Local ID strings                                                                                                  |
| `dnv-vista-sdk-gmod-cli`           | Navigate and search the Gmod tree, validate paths, export node map                                                                           |
| `dnv-vista-sdk-json-validator-cli` | Validate ISO 19848 `DataChannelList` and `TimeSeriesData` files against JSON Schema Draft 2020-12 (with `cross` referential integrity check) |

> For usage details and examples, see [tools/README.md](tools/README.md).

## ⚠️ Error Handling

The SDK uses three distinct conventions depending on the nature of the failure:

| Pattern                       | When                                                                      | Example                                                      |
| ----------------------------- | ------------------------------------------------------------------------- | ------------------------------------------------------------ |
| `throw std::invalid_argument` | Programming error: caller passed invalid input that violates the contract | `ImoNumber{1234507}`, `CodebookNames::fromPrefix("unknown")` |
| `std::optional<T>` return     | Expected failure: input may legitimately be unparseable                   | `LocalId::fromString(...)`, `GmodPath::fromShortPath(...)`   |
| `ValidateResult<T>` return    | Structured validation: caller needs the error detail                      | `FormatDataType::validate(...)`                              |

**`noexcept` convention**: `noexcept` on a function means "does not fail with a business-logic error". It signals that the function returns a result or `nullopt` rather than throwing to report a parse or validation failure.
It does not guarantee zero exceptions in the theoretical sense: like any C++ code that allocates, an out-of-memory condition can still surface as `std::bad_alloc`, which is unrecoverable at this layer regardless of `noexcept`.

## 🛠️ Development

### Setting up Development Environment

```bash
# Clone the repository
git clone https://github.com/dnv-opensource/vista-sdk.git
cd vista-sdk/cpp

# Build the library
cmake -B build -S .
cmake --build build -j$(nproc)

# Run tests
cmake -B build -S . -DDNV_VISTA_SDK_BUILD_TESTS=ON
cmake --build build -j$(nproc)
ctest --test-dir build
```

### Symbol Export

New publicly consumable classes must be annotated with `DNV_VISTA_SDK_CPP_API` (shared-build symbol export), or they won't be linkable from a shared build.

### Project Structure

```
cpp/
├── benchmarks/             # Performance benchmarks
├── cmake/                  # CMake modules and build configuration
├── include/
│   └── dnv/
│       ├── vista/sdk/      # Public API headers
│       └── VistaSDK.h      # Umbrella header
├── samples/                # Usage examples
├── src/                    # Implementation files
├── tests/                  # Unit and integration tests
├── tools/                  # Build tools and CLI utilities
├── CMakeLists.txt
└── README.md
```

## 🔖 For Maintainers

### Version Numbering

The SDK follows [Semantic Versioning](https://semver.org/) (MAJOR.MINOR.PATCH):

- **MAJOR**: Breaking API changes
- **MINOR**: New features, backward compatible
- **PATCH**: Bug fixes, backward compatible

### Creating a Release

Update the version in [CMakeLists.txt](CMakeLists.txt):

```cmake
project(dnv-vista-sdk
    VERSION X.Y.Z  # Update this line
    DESCRIPTION "C++ implementation of DNV Vista SDK for VIS and ISO 19848 standard"
    HOMEPAGE_URL "https://github.com/dnv-opensource/vista-sdk"
    LANGUAGES CXX
)
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes with tests
4. Run the test suite (`ctest --test-dir build`)
5. Commit your changes (`git commit -m 'feat(cpp): add amazing feature'`)
6. Push to the branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

### Development dependencies licenses

- **[doctest](https://github.com/doctest/doctest)**: Testing framework (MIT License) - Development only
- **[nanobench](https://github.com/martinus/nanobench)**: Performance benchmarking framework (MIT License) - Development only

### Sample dependencies licenses

- **[Dear ImGui](https://github.com/ocornut/imgui)**: Immediate-mode GUI library (MIT License) - WASM sample only
- **[emscripten-glfw](https://github.com/pongasoft/emscripten-glfw)**: Emscripten GLFW port by pongasoft (Apache License 2.0) - WASM sample only
- **[GLFW](https://www.glfw.org)**: OpenGL windowing library, headers bundled in emscripten-glfw (zlib License) - WASM sample only

## 🔗 Links

- **Documentation**: [docs.vista.dnv.com](https://docs.vista.dnv.com)
- **GitHub**: [dnv-opensource/vista-sdk](https://github.com/dnv-opensource/vista-sdk)
- **Issues**: [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)

## 📞 Support

For questions and support:

- Create an issue on [GitHub Issues](https://github.com/dnv-opensource/vista-sdk/issues)
- Check the [documentation](https://docs.vista.dnv.com)
- Review the [samples](samples/) directory for examples
