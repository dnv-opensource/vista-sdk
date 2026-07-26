# Vista SDK C++ - Performance Benchmarks (Cross-Compiler)

Benchmark framework: [nanobench](https://github.com/martinus/nanobench) v4.3.11

---

## Test Environment

| Component        | Specification                 |
| ---------------- | ----------------------------- |
| **CPU**          | 12th Gen Intel Core i7-12800H |
| **Base Clock**   | 2.80 GHz                      |
| **Turbo Clock**  | 4.80 GHz                      |
| **RAM**          | 32 GB DDR5-4800               |
| **OS (Linux)**   | Linux 6.12 (Debian 13)        |
| **OS (Windows)** | Windows 10                    |

## Compilers

| OS      | Compiler    | Version       | Flags |
| ------- | ----------- | ------------- | ----- |
| Linux   | GCC         | 14.2.0        | `-O3` |
| Linux   | Clang       | 19.1.7        | `-O3` |
| Windows | GCC (MinGW) | 14.2.0        | `-O3` |
| Windows | MSVC        | 19.50.35721.0 | `/O2` |

---

## Results

### Codebook methods

| Benchmark (ns/op)                     | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ------------------------------------- | ----------: | ------------: | --------------: | -------------: |
| `Quantity_hasStandardValue_hit`       |        1.77 |          1.21 |            2.26 |           9.48 |
| `Quantity_hasStandardValue_miss`      |        7.30 |          1.06 |            8.08 |           6.38 |
| `Quantity_createTag_standard`         |       13.93 |         22.75 |           17.96 |          25.11 |
| `Quantity_createTag_custom`           |       22.97 |         26.57 |           52.81 |          60.84 |
| `State_hasGroup_hit`                  |        1.77 |          1.88 |            1.95 |           6.31 |
| `State_hasGroup_miss`                 |        2.04 |          0.63 |            2.17 |           4.42 |
| `Position_validatePosition_simple`    |       12.96 |         14.63 |           22.69 |          19.80 |
| `Position_validatePosition_composite` |      155.31 |        136.51 |          178.59 |         145.29 |

### Codebooks lookup

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `StdUnorderedMap` |        6.42 |          1.70 |            6.99 |           3.13 |
| `StdMap`          |        3.72 |          5.69 |            4.01 |           6.97 |
| `Codebooks`       |        0.49 |          0.67 |            0.54 |           1.65 |

### Gmod cache access

| Benchmark (ns/op)       | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------------- | ----------: | ------------: | --------------: | -------------: |
| `GmodCacheAccess_v3_4a` |       15.80 |         14.79 |           66.11 |          15.56 |
| `GmodCacheAccess_v3_7a` |       15.77 |         15.41 |           69.32 |          15.27 |

### Gmod node lookup

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `StdUnorderedMap` |       33.08 |         27.73 |           37.57 |          38.73 |
| `Gmod`            |        6.51 |          8.19 |            7.13 |          25.91 |

### Gmod path parsing

| Benchmark (ns/op)             | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------------------- | ----------: | ------------: | --------------: | -------------: |
| `FromShortPath`               |      503.34 |        461.59 |          696.58 |         978.68 |
| `FromFullPath`                |      376.53 |        396.93 |          608.91 |         682.37 |
| `FromShortPathIndividualized` |      897.27 |        849.43 |        1,274.23 |       1,840.43 |
| `FromFullPathIndividualized`  |      667.34 |        658.63 |          879.81 |       1,053.55 |

### Gmod traversal

| Benchmark (ns/op) |   `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ------------: | ------------: | --------------: | -------------: |
| `FullTraversal`   | 30,709,411.83 | 34,474,440.20 |   32,160,000.00 |  58,343,400.00 |

### Gmod versioning - path conversion

| Benchmark (ns/op)         | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ------------------------- | ----------: | ------------: | --------------: | -------------: |
| `ConvertPath`             |    1,594.07 |      1,498.04 |        2,131.63 |       2,241.61 |
| `ConvertPathWithLocation` |    1,585.83 |      1,456.88 |        1,908.77 |       2,171.78 |

### DataChannelList lookup

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `ByShortId`       |        5.95 |          3.49 |            6.92 |           4.86 |
| `ByLocalId`       |        25.6 |         33.18 |           33.28 |          54.29 |

### LocalId parsing

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `Simple`          |    1,206.49 |      1,274.66 |        2,060.83 |       2,191.61 |
| `Complex`         |    4,549.31 |      4,529.80 |        7,510.32 |       9,065.00 |

### DataChannelList serialization

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `Serialize`       |    5,366.96 |      5,980.71 |        7,774.88 |       8,362.39 |
| `Deserialize`     |   24,700.90 |     25,527.97 |       43,907.14 |      47,924.53 |

### TimeSeriesData serialization

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `Serialize`       |    2,029.06 |      2,377.73 |        2,692.99 |       3,434.15 |
| `Deserialize`     |   10,363.19 |     10,365.01 |       23,720.85 |      37,877.19 |

### Containers (SDK custom vs STL)

| Benchmark (ns/op)           | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| --------------------------- | ----------: | ------------: | --------------: | -------------: |
| `PerfectHashMap_hit`        |        8.22 |         14.12 |            9.46 |          20.95 |
| `StdUnorderedMap_hit`       |       32.82 |         33.37 |           40.40 |          59.84 |
| `PerfectHashMap_miss`       |        6.85 |          7.73 |            7.59 |          19.42 |
| `StdUnorderedMap_miss`      |       19.18 |         18.52 |           22.09 |          47.16 |
| `PerfectHashMap_stringview` |       18.23 |         18.72 |           21.12 |          33.01 |
| `StdUnorderedSet_hit`       |       31.28 |         34.58 |           40.90 |          30.20 |
| `StringSet_hit`             |        6.46 |          9.10 |            7.07 |          24.68 |
| `StringSet_miss`            |        4.91 |          7.30 |            5.27 |          24.15 |
| `StringSet_stringview`      |       17.22 |         15.29 |           18.78 |          33.27 |
| `StackVector_split_short`   |       10.99 |         11.19 |           14.63 |          19.37 |
| `StdVector_split_short`     |       40.20 |         39.09 |          114.22 |         119.11 |
| `StackVector_split_long`    |       27.26 |         31.39 |           37.83 |          49.36 |
| `StdVector_split_long`      |       64.19 |         65.16 |          168.14 |         243.40 |

### StringBuilder (SDK custom vs STL)

| Benchmark (ns/op)                 | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| --------------------------------- | ----------: | ------------: | --------------: | -------------: |
| `StringBuilder_jsonish`           |       43.96 |         38.23 |          114.63 |          50.68 |
| `StdString_jsonish`               |       77.10 |         78.66 |          239.15 |         189.20 |
| `StringBuilder_largeAccumulation` |    1,659.40 |      1,638.49 |        4,203.39 |       2,073.68 |
| `StdString_largeAccumulation`     |    2,021.59 |      2,045.68 |        4,681.61 |       2,825.43 |
| `StringBuilder_indent`            |        1.62 |          1.70 |            1.48 |           2.74 |
| `StdString_indent`                |        2.39 |          2.95 |            2.25 |          13.42 |

---

_Benchmarked on July 26, 2026_
