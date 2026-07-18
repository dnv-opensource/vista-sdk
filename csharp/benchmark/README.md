### DataChannelList serialization

See [Transport/_files/](Vista.SDK.Benchmarks/Transport/_files/) folder for the sample payload that is being serialized.
Benchmark implementation: [Transport/DataChannelListSerialization.cs](Vista.SDK.Benchmarks/Transport/DataChannelListSerialization.cs)

``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1645 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.202
  [Host]                        : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT
  DataChannelList serialization : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT

Job=DataChannelList serialization  IterationCount=3  LaunchCount=1
WarmupCount=3

```
| Method   | Categories       | CompressionLevel |            Mean |           Error |        StdDev |  Payload size |
| -------- | ---------------- | ---------------- | --------------: | --------------: | ------------: | ------------: |
| **Json** | **Uncompressed** | **?**            |    **906.4 μs** |    **106.4 μs** |   **5.83 μs** | **285.92 KB** |
| Avro     | Uncompressed     | ?                |        702.2 μs |        337.7 μs |      18.51 μs |     113.22 KB |
|          |                  |                  |                 |                 |               |               |
| Json     | Brotli           | ?                |    363,783.2 μs |    468,803.0 μs |  25,696.67 μs |      18.31 KB |
| Avro     | Brotli           | ?                |    129,235.6 μs |     35,572.9 μs |   1,949.87 μs |      18.56 KB |
|          |                  |                  |                 |                 |               |               |
| **Json** | **Bzip2**        | **5**            | **42,353.5 μs** | **16,058.2 μs** | **880.20 μs** |  **19.19 KB** |
| Avro     | Bzip2            | 5                |     12,175.2 μs |      9,095.2 μs |     498.54 μs |       19.5 KB |
| **Json** | **Bzip2**        | **9**            | **48,419.8 μs** | **16,895.3 μs** | **926.09 μs** |  **19.19 KB** |
| Avro     | Bzip2            | 9                |     13,762.6 μs |      2,310.1 μs |     126.62 μs |       19.5 KB |


### Gmod traversal

Traverses through all possible paths in gmod, avoiding cycles.
Benchmark implementation: [Gmod/GmodTraversal.cs](Vista.SDK.Benchmarks/Gmod/GmodTraversal.cs)

``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1645 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.202
  [Host]     : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT
  DefaultJob : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT


```
| Method        |     Mean |   Error |  StdDev | Allocated |
| ------------- | -------: | ------: | ------: | --------: |
| FullTraversal | 135.3 ms | 2.69 ms | 3.49 ms |      2 KB |


### Gmod path parsing

Parses the string representation of a gmod path (`411.1/C101.72/I101`) into the domain model object.
Benchmark implementation: [Gmod/GmodPathParse.cs](Vista.SDK.Benchmarks/Gmod/GmodPathParse.cs)

``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1645 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.202
  [Host]     : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT
  DefaultJob : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT


```
| Method   |     Mean |     Error |    StdDev |  Gen 0 | Allocated |
| -------- | -------: | --------: | --------: | -----: | --------: |
| TryParse | 3.771 μs | 0.0719 μs | 0.0856 μs | 0.2289 |      3 KB |
