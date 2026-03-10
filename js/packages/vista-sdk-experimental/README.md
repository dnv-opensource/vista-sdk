# dnv-vista-sdk-experimental

[![NPM current](https://img.shields.io/npm/v/dnv-vista-sdk-experimental?label=NPM%20dnv-vista-sdk-experimental)](https://www.npmjs.com/package/dnv-vista-sdk-experimental)

Experimental extensions for the Vista SDK — PMS Local IDs and ISO 19848/19847 inspired transport models.

> **⚠️ Experimental**: This package contains features that are under active development. APIs may change between versions without following semver conventions. For stable functionality, use the core [`dnv-vista-sdk`](../vista-sdk/) package.

For full SDK documentation, see the [JS SDK README](../../README.md).

## Installation

```bash
npm install dnv-vista-sdk-experimental
```

This package requires `dnv-vista-sdk` as a peer dependency:

```bash
npm install dnv-vista-sdk dnv-vista-sdk-experimental
```

## What's Included

| Export                         | Description                                              |
| ------------------------------ | -------------------------------------------------------- |
| `PMSLocalId`                   | PMS (Planned Maintenance System) Local ID representation |
| `PMSLocalIdBuilder`            | Builder for constructing PMS Local IDs                   |
| `AssetIdentifier`              | Asset identification for transport models                |
| `DataId`                       | Data identifier for time series entries                  |
| `DataList` / `DataListDto`     | Experimental data list domain and DTO models             |
| `TimeSeries` / `TimeSeriesDto` | Experimental time series domain and DTO models           |
| `JSONExtensions`               | JSON serialization/deserialization utilities             |
| `JSONSerializer`               | Serializer for experimental transport types              |

## Quick Example

```typescript
import { PMSLocalIdBuilder } from "dnv-vista-sdk-experimental";
import { VIS, VisVersion } from "dnv-vista-sdk";

const gmod = await VIS.instance.getGmod(VisVersion.v3_4a);
const path = gmod.parsePath("411.1/C101.31-2");

const pmsLocalId = PMSLocalIdBuilder.create(VisVersion.v3_4a)
    .withPrimaryItem(path)
    .build();

console.log(`${pmsLocalId}`);
```

## Documentation

- **[JS SDK README](../../README.md)** — Full documentation for the core SDK
- **[Core Package](../vista-sdk/)** — The stable `dnv-vista-sdk` package
- **[Samples](../../samples/)** — Runnable TypeScript examples
- **[Main README](../../../README.md)** — Overview of VIS concepts and cross-platform SDK information

## License

MIT — see [LICENSE](../../../LICENSE).
