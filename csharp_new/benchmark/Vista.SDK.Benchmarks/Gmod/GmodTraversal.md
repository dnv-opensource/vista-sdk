``` ini

BenchmarkDotNet=v0.13.1, OS=Windows 10.0.19043.1645 (21H1/May2021Update)
11th Gen Intel Core i9-11950H 2.60GHz, 1 CPU, 16 logical and 8 physical cores
.NET SDK=6.0.202
  [Host]     : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT
  DefaultJob : .NET 6.0.4 (6.0.422.16404), X64 RyuJIT


```
|        Method |     Mean |   Error |  StdDev | Allocated |
|-------------- |---------:|--------:|--------:|----------:|
| FullTraversal | 135.3 ms | 2.69 ms | 3.49 ms |      2 KB |
