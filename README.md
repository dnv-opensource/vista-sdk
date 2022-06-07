## Vista SDK
The Vista team at DNV are working on tooling related to
* DNV Vessel Information Structure (VIS)
* ISO 19847 - Ships and marine technology — Shipboard data servers to share field data at sea
* ISO 19848 - Ships and marine technology — Standard data for shipboard machinery and equipment

In this repository we codify the rules and principles of VIS and related ISO-standards to enable and support
users and implementers of the standards.

Our plan is to develop SDKs for some of the most common platforms. We are starting with .NET, Python and JavaScript.
We will be developing these SDKs as open source projects. Feel free to provide input, request changes or make contributions by creating issues in this repository.

For general documentation relating to VIS and relating standard. See [vista.dnv.com/docs](https://vista.dnv.com/docs).

> **_NOTE:_**  These SDKs are currently being used in production at DNV, and will be open sourced and released under preview initially.

### Content

Each SDK makes use of the contents of the resources and schemas folders to generate code and use the standards.

```
📦vista-sdk
 ┣ 📂resources
 ┃ ┣ 📜codebooks-vis-3-4a.json.gz
 ┃ ┗ 📜gmod-vis-3-4a.json.gz
 ┣ 📂schemas
 ┃ ┣ 📂avro
 ┃ ┃ ┣ 📜DataChannelList.avsc
 ┃ ┃ ┗ 📜TimeSeriesData.avsc
 ┃ ┣ 📂json
 ┃ ┃ ┣ 📜DataChannelList.schema.json
 ┃ ┃ ┗ 📜TimeSeriesData.schema.json
 ┃ ┗ 📂xml
 ┃ ┃ ┣ 📜DataChannelList.xsd
 ┃ ┃ ┗ 📜TimeSeriesData.xsd
 ┣ 📂csharp
 ┣ 📂python
 ┣ 📂js
 ┣ 📜LICENSE
 ┗ 📜README.md
```

### SDK outline

This section will outline the various components and modules in our SDKs.

#### Vessel Information Structure

There are two codified components of VIS in our SDKs

* Generic product model (Gmod)
* Codebooks
* Coming soon - versioning support, ability to convert gmod nodes and paths between versions of gmod

For more information on this concepts, check out [vista.dnv.com/docs](https://vista.dnv.com/docs).

#### ISO-19848 and ISO-19847

Part of these standards are the definition of datastructures used for communicating and sharing sensor data.
Note that while compression isnt explicitly mentioned in these standards, the standard doesnt prohibit use
of compression when implementing these standards, as long as the datastructures remain the same.

### Benchmarks

We are developing some benchmarks to keep track of the performance characteristics of the libraries we are creating.

#### Transport packages - DataChannelList and TimeSeriesData

The ISO-19848/10947 standards define the schema for the XML encoding of the transport packages.
The JSON encoding is only provded in example form, but we have developed [JSON schemas](https://json-schema.org/) for these packages [here](schemas/json/).

In the benchmark below we try to isolate the difference between Json and Avro encoding, and measure the effect of compression using Bzip2 and Brotli.
See the `Payload size` column below for an insight into size of ISO-19848 packages over the wire.
Avro has been included as an example binary encoding - it is not currently part of the standard.
The latency measurements in this context are less useful, as they are platform-specific.
See [csharp/benchmark](csharp/benchmark) for more details on the method of the benchmarks.

``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1645 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.202
  [Host]                        : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT
  DataChannelList serialization : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT

Job=DataChannelList serialization  IterationCount=3  LaunchCount=1
WarmupCount=3

```
| Method |   Categories | CompressionLevel |         Mean |        Error |       StdDev | Payload size |
|------- |------------- |----------------- |-------------:|-------------:|-------------:|-------------:|
|   **Json** | **Uncompressed** |                **?** |     **906.4 μs** |     **106.4 μs** |      **5.83 μs** |    **285.92 KB** |
|   Avro | Uncompressed |                ? |     702.2 μs |     337.7 μs |     18.51 μs |    113.22 KB |
|        |              |                  |              |              |              |              |
|   Json |       Brotli |                ? | 363,783.2 μs | 468,803.0 μs | 25,696.67 μs |     18.31 KB |
|   Avro |       Brotli |                ? | 129,235.6 μs |  35,572.9 μs |  1,949.87 μs |     18.56 KB |
|        |              |                  |              |              |              |              |
|   **Json** |        **Bzip2** |                **5** |  **42,353.5 μs** |  **16,058.2 μs** |    **880.20 μs** |     **19.19 KB** |
|   Avro |        Bzip2 |                5 |  12,175.2 μs |   9,095.2 μs |    498.54 μs |      19.5 KB |
|   **Json** |        **Bzip2** |                **9** |  **48,419.8 μs** |  **16,895.3 μs** |    **926.09 μs** |     **19.19 KB** |
|   Avro |        Bzip2 |                9 |  13,762.6 μs |   2,310.1 μs |    126.62 μs |      19.5 KB |

