```

BenchmarkDotNet v0.13.10, Windows 10 (10.0.19045.3570/22H2/2022Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK 8.0.100
  [Host]     : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2
  DefaultJob : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2


```
| Method         | Input      | Mean     | Error     | StdDev    | Ratio        | RatioSD | Rank | Allocated | Alloc Ratio |
|--------------- |----------- |---------:|----------:|----------:|-------------:|--------:|-----:|----------:|------------:|
| Larsson        | 400        | 2.672 ns | 0.0367 ns | 0.0343 ns | 1.11x slower |   0.04x |    5 |         - |          NA |
| Bcl            | 400        | 2.416 ns | 0.0736 ns | 0.0848 ns |     baseline |         |    4 |         - |          NA |
| BclOrd         | 400        | 1.798 ns | 0.0595 ns | 0.0585 ns | 1.34x faster |   0.04x |    3 |         - |          NA |
| Fnv            | 400        | 1.263 ns | 0.0505 ns | 0.0582 ns | 1.92x faster |   0.11x |    2 |         - |          NA |
| Crc32Intrinsic | 400        | 1.220 ns | 0.0453 ns | 0.0424 ns | 1.98x faster |   0.09x |    1 |         - |          NA |
|                |            |          |           |           |              |         |      |           |             |
| Bcl            | H346.11112 | 5.968 ns | 0.0913 ns | 0.0854 ns |     baseline |         |    5 |         - |          NA |
| Fnv            | H346.11112 | 3.995 ns | 0.0858 ns | 0.0761 ns | 1.49x faster |   0.04x |    4 |         - |          NA |
| Larsson        | H346.11112 | 3.825 ns | 0.0800 ns | 0.0748 ns | 1.56x faster |   0.04x |    3 |         - |          NA |
| BclOrd         | H346.11112 | 3.336 ns | 0.0859 ns | 0.0804 ns | 1.79x faster |   0.05x |    2 |         - |          NA |
| Crc32Intrinsic | H346.11112 | 2.900 ns | 0.0681 ns | 0.0569 ns | 2.06x faster |   0.04x |    1 |         - |          NA |
