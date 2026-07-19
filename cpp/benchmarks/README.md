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
| `Position_validatePosition_composite` |      161.44 |        142.78 |          215.70 |         172.80 |

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
| `FromShortPath`               |      564.34 |        589.54 |        1,045.68 |       1,453.64 |
| `FromFullPath`                |      633.93 |        679.03 |        1,212.74 |       1,464.30 |
| `FromShortPathIndividualized` |    1,156.81 |      1,156.83 |        2,013.57 |       3,030.09 |
| `FromFullPathIndividualized`  |    1,164.68 |      1,129.92 |        1,741.28 |       2,259.49 |

### Gmod traversal

| Benchmark (ns/op) |   `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ------------: | ------------: | --------------: | -------------: |
| `FullTraversal`   | 34,886,852.00 | 36,897,152.00 |   36,423,666.67 |  54,102,250.00 |

### Gmod versioning - path conversion

| Benchmark (ns/op)         | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ------------------------- | ----------: | ------------: | --------------: | -------------: |
| `ConvertPath`             |    2,067.05 |      2,051.02 |        3,737.92 |       4,255.62 |
| `ConvertPathWithLocation` |    2,248.06 |      2,223.19 |        3,826.66 |       4,578.12 |

### DataChannelList lookup

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `ByShortId`       |        5.95 |          3.49 |            6.92 |           4.86 |
| `ByLocalId`       |        25.6 |         33.18 |           33.28 |          54.29 |

### LocalId parsing

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `Simple`          |    1,484.73 |      1,525.71 |        3,344.40 |       3,395.65 |
| `Complex`         |    7,509.31 |      7,612.69 |       13,748.91 |      15,830.47 |

### DataChannelList serialization

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `Serialize`       |   11,299.17 |     10,740.45 |       15,949.87 |      18,923.62 |
| `Deserialize`     |   24,700.90 |     25,527.97 |       43,907.14 |      47,924.53 |

### TimeSeriesData serialization

| Benchmark (ns/op) | `Linux GCC` | `Linux Clang` | `Windows MinGW` | `Windows MSVC` |
| ----------------- | ----------: | ------------: | --------------: | -------------: |
| `Serialize`       |    4,637.55 |      4,351.99 |        7,373.35 |       8,248.88 |
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

---

_Benchmarked on July 19, 2026_
