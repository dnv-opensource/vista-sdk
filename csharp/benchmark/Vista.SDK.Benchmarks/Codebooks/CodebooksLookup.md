```

BenchmarkDotNet v0.13.10, Windows 10 (10.0.19045.3570/22H2/2022Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK 8.0.100
  [Host]     : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2
  DefaultJob : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2


```
| Method     | Mean     | Error     | StdDev    | Ratio        | RatioSD | Rank | Allocated | Alloc Ratio |
|----------- |---------:|----------:|----------:|-------------:|--------:|-----:|----------:|------------:|
| FrozenDict | 8.458 ns | 0.2027 ns | 0.5911 ns | 1.09x slower |   0.13x |    3 |         - |          NA |
| Dict       | 7.837 ns | 0.3947 ns | 1.1451 ns |     baseline |         |    2 |         - |          NA |
| Codebooks  | 1.497 ns | 0.1007 ns | 0.2822 ns | 5.48x faster |   1.49x |    1 |         - |          NA |
