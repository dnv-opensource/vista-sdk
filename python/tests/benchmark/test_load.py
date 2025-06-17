"""Load testing for Vista SDK."""

import asyncio
import os
from concurrent.futures import ThreadPoolExecutor

import psutil
import pytest
from pytest_benchmark.fixture import BenchmarkFixture  # type: ignore
from testdata_generator import TestDataGenerator  # type: ignore

from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.fixture(scope="module")
def test_data_generator() -> TestDataGenerator:  # noqa : D103
    vis = VIS().instance
    generator = TestDataGenerator()
    generator.setup(vis)
    return generator


def get_memory_usage() -> float:
    """Get current memory usage in MB."""
    process = psutil.Process(os.getpid())
    return process.memory_info().rss / 1024 / 1024


@pytest.mark.load_test
def test_large_path_dataset(test_data_generator: TestDataGenerator) -> None:
    """Test handling of large path datasets."""
    paths = test_data_generator.generate_large_dataset(size=1)
    # Test memory usage pattern
    memory_sizes: list[float] = []
    for i in range(0, len(paths), 10):
        start_memory: float = get_memory_usage()
        subset = paths[: i + 10]
        # Process subset and measure memory
        result = process_paths(subset)
        end_memory: float = get_memory_usage()

        memory_sizes.append(end_memory - start_memory)

        assert result is not None

    # Chack memory growth is not exessive
    assert max(memory_sizes) < 1000  # Max 1GB memory usage


@pytest.mark.benchmark(group="concurrent")
def test_concurrent_large_dataset(
    test_data_generator: TestDataGenerator, benchmark: BenchmarkFixture
) -> None:
    """Test concurrent processing of large datasets."""
    paths = test_data_generator.generate_large_dataset(size=1)
    chunk_size = 100
    chunks = [paths[i : i + chunk_size] for i in range(0, len(paths), chunk_size)]

    def run_concurrent():  # noqa : ANN202
        with ThreadPoolExecutor(max_workers=4) as executor:
            return list(executor.map(process_paths, chunks))

    results = benchmark.pedantic(run_concurrent, iterations=3, rounds=100)

    assert all(result is not None for result in results)
    assert len(results) == len(chunks)


@pytest.mark.asyncio
@pytest.mark.benchmark(group="async")
async def test_async_large_dataset(
    test_data_generator: TestDataGenerator, benchmark: BenchmarkFixture
) -> None:
    """Test asynchronous processing of large datasets."""
    paths = test_data_generator.generate_large_dataset(size=1)
    chunk_size = 100
    chunks = [paths[i : i + chunk_size] for i in range(0, len(paths), chunk_size)]

    async def run_benchmark():  # noqa : ANN202
        async def run_async() -> list[list[GmodPath]]:
            tasks = [process_paths_async(chunk) for chunk in chunks]
            return await asyncio.gather(*tasks)

        return await benchmark.pedantic(run_async, iterations=1, rounds=1)

    results = await run_benchmark()
    assert all(result is not None for result in results)
    assert len(results) == len(chunks)


def process_paths(paths: list[GmodPath]) -> list[GmodPath]:
    """Helper function to process a chunk of paths."""
    vis = VIS().instance
    results = []
    for path in paths:
        try:
            converted = vis.convert_path(VisVersion.v3_4a, path, VisVersion.v3_5a)
            if converted:
                results.append(converted)
        except Exception as e:
            print(f"Error processing path {path}: {e!s}")
            continue
    return results


async def process_paths_async(paths: list[GmodPath]) -> list[GmodPath]:
    """Helper function to process a chunk of paths asynchronously."""
    vis = VIS().instance
    results = []
    for path in paths:
        try:
            # Use asyncio.to_thread for CPU-bound operations
            converted = await asyncio.to_thread(
                vis.convert_path, VisVersion.v3_4a, path, VisVersion.v3_5a
            )
            if converted:
                results.append(converted)
        except Exception as e:
            print(f"Error processing path {path}: {e!s}")
            continue
    return results
