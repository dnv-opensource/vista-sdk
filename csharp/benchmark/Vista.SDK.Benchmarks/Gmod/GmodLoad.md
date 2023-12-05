```

BenchmarkDotNet v0.13.10, Windows 10 (10.0.19045.3570/22H2/2022Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK 8.0.100
  [Host]     : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2
  DefaultJob : .NET 8.0.0 (8.0.23.53103), X64 RyuJIT AVX2


```
| Method | Mean     | Error    | StdDev   | Rank | Gen0      | Gen1      | Gen2     | Allocated |
|------- |---------:|---------:|---------:|-----:|----------:|----------:|---------:|----------:|
| Load   | 39.44 ms | 0.780 ms | 1.367 ms |    1 | 1538.4615 | 1461.5385 | 538.4615 |  15.38 MB |
