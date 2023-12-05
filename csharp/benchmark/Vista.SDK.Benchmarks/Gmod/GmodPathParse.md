```

BenchmarkDotNet v0.13.10, Windows 10 (10.0.19045.3570/22H2/2022Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK 8.0.100
  [Host]     : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2
  DefaultJob : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2


```
| Method                         | Categories    | Mean       | Error    | StdDev   | Gen0   | Gen1   | Allocated |
|------------------------------- |-------------- |-----------:|---------:|---------:|-------:|-------:|----------:|
| TryParseFullPath               | No location   |   742.5 ns | 11.02 ns | 10.31 ns | 0.0572 |      - |     728 B |
| TryParse                       | No location   | 3,850.0 ns | 53.65 ns | 47.56 ns | 0.2670 |      - |    3416 B |
|                                |               |            |          |          |        |        |           |
| TryParseFullPathIndividualized | With location |   980.8 ns | 19.39 ns | 19.04 ns | 0.0896 |      - |    1144 B |
| TryParseIndividualized         | With location | 1,952.1 ns | 38.65 ns | 46.00 ns | 0.3548 | 0.0038 |    4496 B |
