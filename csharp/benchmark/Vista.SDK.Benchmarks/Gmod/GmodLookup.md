```

BenchmarkDotNet v0.13.10, Windows 10 (10.0.19045.3570/22H2/2022Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK 8.0.100
  [Host]     : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2
  DefaultJob : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2


```
| Method     | Mean     | Error    | StdDev   | Ratio        | RatioSD | Rank | Allocated | Alloc Ratio |
|----------- |---------:|---------:|---------:|-------------:|--------:|-----:|----------:|------------:|
| Dict       | 41.68 ns | 0.856 ns | 1.019 ns |     baseline |         |    3 |         - |          NA |
| Gmod       | 21.65 ns | 0.428 ns | 0.458 ns | 1.92x faster |   0.06x |    2 |         - |          NA |
| FrozenDict | 19.81 ns | 0.365 ns | 0.341 ns | 2.09x faster |   0.06x |    1 |         - |          NA |
