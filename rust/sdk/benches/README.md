## Running benchmarks

```bash
cargo bench --bench benchmarks

# With profiling
cargo bench --bench benchmarks -- --profile-time 30

# or cargo criterion - install: cargo install cargo-criterion
cargo criterion
```