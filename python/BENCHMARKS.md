# Vista SDK Python Benchmarks

Performance benchmarks for the Vista SDK Python implementation.

**Environment:** Python 3.13.9, pytest-benchmark 5.1.0
**Platform:** Linux
**Last Updated:** January 2026

## Summary

| Category | Operation | Mean Time | Throughput |
|----------|-----------|-----------|------------|
| Lookup | Codebooks lookup | 1.3 μs | 796K ops/s |
| Lookup | Gmod node by code | 184 ns | 5.4M ops/s |
| Lookup | DataChannel by short_id | 161 ns | 6.2M ops/s |
| Lookup | DataChannel by local_id | 2.6 μs | 378K ops/s |
| Serialization | JSON serialize (DC) | 21.8 μs | 46K ops/s |
| Serialization | JSON deserialize (DC) | 52.0 μs | 19K ops/s |
| Domain | DataChannelList to domain | 1.0 ms | 992 ops/s |
| Domain | TimeSeriesData to domain | 48.3 μs | 21K ops/s |
| Parsing | LocalId complex | 229.0 μs | 4K ops/s |
| Parsing | GmodPath full path | 30.4 μs | 33K ops/s |
| Versioning | Path conversion | 61.2 μs | 16K ops/s |
| Traversal | Full Gmod traversal | 3.15 s | 0 ops/s |

## Detailed Results

### CodebooksLookup

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_dict_lookup | 181 ns | 15 ns | 5.5M ops/s |
| test_frozen_dict_lookup | 747 ns | 193 ns | 1.3M ops/s |
| test_codebooks_lookup | 1.3 μs | 448 ns | 796K ops/s |

### DataChannelListSerialization

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_json_serialize | 21.8 μs | 1.9 μs | 46K ops/s |
| test_json_deserialize | 52.0 μs | 2.6 μs | 19K ops/s |
| test_json_bzip2[5] | 1.3 ms | 10.5 μs | 781 ops/s |
| test_json_bzip2[9] | 1.3 ms | 42.8 μs | 780 ops/s |
| test_json_brotli | 5.5 ms | 68.5 μs | 181 ops/s |

### GmodLoad

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_load | 1.4 μs | 192 ns | 736K ops/s |

### GmodLookup

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_lookup_by_code | 184 ns | 14 ns | 5.4M ops/s |
| test_lookup_deep_node | 185 ns | 21 ns | 5.4M ops/s |
| test_lookup_root_node | 192 ns | 15 ns | 5.2M ops/s |
| test_try_get_node | 231 ns | 19 ns | 4.3M ops/s |

### GmodPathParse

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_try_parse_full_path | 30.4 μs | 2.1 μs | 33K ops/s |
| test_try_parse_full_path_individualized | 52.6 μs | 4.5 μs | 19K ops/s |
| test_try_parse_individualized | 95.7 μs | 4.9 μs | 10K ops/s |
| test_try_parse | 141.6 μs | 5.3 μs | 7K ops/s |

### GmodTraversal

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_full_traversal | 3.15 s | 32.7 ms | 0 ops/s |

### GmodVersioningConvertPath

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_convert_path[3-6a-3-7a] | 58.6 μs | 9.4 μs | 17K ops/s |
| test_convert_path[3-5a-3-6a] | 58.7 μs | 2.9 μs | 17K ops/s |
| test_convert_path[3-7a-3-8a] | 60.3 μs | 2.8 μs | 17K ops/s |
| test_convert_path[3-9a-3-10a] | 61.2 μs | 5.4 μs | 16K ops/s |
| test_convert_path[3-4a-3-5a] | 62.5 μs | 5.6 μs | 16K ops/s |

### ISO19848 DataChannelList Lookup

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_lookup_by_index | 153 ns | 11 ns | 6.5M ops/s |
| test_lookup_by_short_id | 161 ns | 11 ns | 6.2M ops/s |
| test_lookup_by_local_id | 2.6 μs | 307 ns | 378K ops/s |

### ISO19848 DataChannelList ToDomain

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_data_channel_list_to_domain | 1.0 ms | 23.3 μs | 992 ops/s |

### ISO19848 DataChannelList ToJson

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_data_channel_list_to_json | 21.8 μs | 1.1 μs | 46K ops/s |

### ISO19848 TimeSeriesData ToDomain

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_time_series_data_to_domain | 48.3 μs | 2.0 μs | 21K ops/s |

### ISO19848 TimeSeriesData ToJson

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_time_series_data_to_json | 18.2 μs | 1.0 μs | 55K ops/s |

### LocalIdParse

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_parse_simple | 112.4 μs | 4.9 μs | 9K ops/s |
| test_try_parse_simple | 112.7 μs | 6.9 μs | 9K ops/s |
| test_parse_complex | 229.0 μs | 8.8 μs | 4K ops/s |
| test_try_parse_complex | 229.1 μs | 9.1 μs | 4K ops/s |
| test_parse_medium | 750.1 μs | 15.9 μs | 1K ops/s |

> **Note:** Parsing performance depends on the structural location of the target node in the GMOD tree due to depth-first search (DFS) traversal, not just the apparent path complexity. Paths where the target node appears later in the child iteration order require more traversal to find.

### parsing

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_path_parsing_benchmark[3-8a] | 3.0 μs | 302 ns | 328K ops/s |
| test_path_parsing_benchmark[3-6a] | 3.1 μs | 210 ns | 326K ops/s |
| test_path_parsing_benchmark[3-5a] | 3.1 μs | 295 ns | 324K ops/s |
| test_path_parsing_benchmark | 131.9 μs | 5.5 μs | 8K ops/s |
| test_path_parsing_benchmark[3-7a] | 228.6 μs | 6.3 μs | 4K ops/s |

### versioning

| Test | Mean | Std Dev | OPS |
|------|------|---------|-----|
| test_version_conversion_benchmark | 61.4 μs | 5.1 μs | 16K ops/s |

## Running Benchmarks

```bash
# Run all benchmarks and generate this file
python run_benchmarks.py --generate-markdown

# Run all benchmarks
python run_benchmarks.py

# Run specific benchmark group
python run_benchmarks.py --group gmod

# Run with verbose output
python run_benchmarks.py --verbose
```

## Notes

- All times are mean values across multiple iterations
- OPS = Operations Per Second
- Benchmarks run with garbage collection enabled (realistic conditions)
- Results may vary based on hardware and system load
