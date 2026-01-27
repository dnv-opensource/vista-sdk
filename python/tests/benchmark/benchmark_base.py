"""Base configuration and utilities for Vista SDK benchmarks."""

from collections.abc import Callable
from dataclasses import dataclass
from enum import Enum
from typing import Any

import psutil
from pytest_benchmark.fixture import BenchmarkFixture


class MethodOrderPolicy(Enum):
    """Mirror of BenchmarkDotNet's MethodOrderPolicy."""

    Declared = "Declared"
    Alphabetical = "Alphabetical"


class SummaryOrderPolicy(Enum):
    """Mirror of BenchmarkDotNet's SummaryOrderPolicy."""

    FastestToSlowest = "FastestToSlowest"
    SlowestToFastest = "SlowestToFastest"


@dataclass
class BenchmarkConfig:
    """Configuration for benchmarks to mirror BenchmarkDotNet style."""

    group: str
    baseline: bool = False
    memory_tracking: bool = True
    method_order: MethodOrderPolicy = MethodOrderPolicy.Declared
    summary_order: SummaryOrderPolicy = SummaryOrderPolicy.FastestToSlowest
    description: str | None = None


def run_benchmark(
    benchmark: BenchmarkFixture, func: Callable[[], Any], config: BenchmarkConfig
) -> Any:  # noqa : ANN401
    """Run benchmark with memory tracking and detailed metrics."""
    print(f"Running benchmark with config: {config}")

    # Configure benchmark using the correct pytest-benchmark API
    benchmark.group = config.group
    if config.baseline:
        benchmark.extra_info["baseline"] = True
    if config.description:
        benchmark.extra_info["description"] = config.description
    benchmark.extra_info["method_order"] = config.method_order.value
    benchmark.extra_info["summary_order"] = config.summary_order.value

    # Actually run the benchmark
    result = benchmark(func)
    print(f"Benchmark result: {result}")
    return result  # Track memory before if memory tracking is enabled
    if config.memory_tracking:
        process = psutil.Process()
        mem_before = process.memory_info().rss

    # Run benchmark
    result = benchmark(func)

    # Track memory after if memory tracking is enabled
    if config.memory_tracking:
        mem_after = process.memory_info().rss
        allocated = (mem_after - mem_before) / 1024  # Convert to KB
        # Add memory allocation info to benchmark stats
        benchmark.stats.stats["allocated"] = allocated
        benchmark.stats.stats["gen0_collections"] = 0  # Placeholder for .NET GC stats
        benchmark.stats.stats["gen1_collections"] = 0
        benchmark.stats.stats["gen2_collections"] = 0

    return result
