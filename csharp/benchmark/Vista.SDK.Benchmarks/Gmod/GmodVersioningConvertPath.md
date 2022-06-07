``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1586 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.201
  [Host]     : .NET 6.0.3 (6.0.322.12309), X64 RyuJIT
  DefaultJob : .NET 6.0.3 (6.0.322.12309), X64 RyuJIT


```
|      Method |     Mean |    Error |   StdDev |  Gen 0 | Allocated |
|------------ |---------:|---------:|---------:|-------:|----------:|
| ConvertPath | 54.89 μs | 1.095 μs | 1.737 μs | 5.6152 |     69 KB |
