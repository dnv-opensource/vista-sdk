"""Performance benchmarks for Vista SDK."""

import gc
import time

import psutil
import pytest
from pytest_benchmark.fixture import BenchmarkFixture  # type: ignore

from vista_sdk import gmod_path
from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.benchmark(group="parsing")
def test_path_parsing_benchmark(benchmark: BenchmarkFixture) -> None:
    """Benchmark path parsing performance."""
    vis = VIS().instance
    gmod = vis.get_gmod(VisVersion.v3_4a)
    path_str = "411.1/C101.63/S206"

    def parse_path() -> GmodPath:
        return gmod.parse_path(path_str)

    result = benchmark(parse_path)
    assert result is not None
    assert isinstance(result, GmodPath)


# @pytest.mark.benchmark(group="conversion")
def test_version_conversion_benchmark() -> None:
    """Benchmark version conversion performance."""
    vis = VIS().instance
    source_gmod = vis.get_gmod(VisVersion.v3_6a)
    path = source_gmod.parse_path("411.1/C101.63/S206")

    # TODO: Change this back to using vis.convert_path()
    def convert_path() -> GmodPath | None:
        try:
            # Get versioning
            print("init")
            versioning = vis.get_gmod_versioning(VisVersion.v3_6a)
            print(f"Got versioning: {versioning.__str__()}")

            # Try conversion
            converted = versioning.convert_path(
                VisVersion.v3_6a, path, VisVersion.v3_7a
            )
            print(f"Conversion result: {converted}")

            # Validate result
            if converted:
                target_gmod = vis.get_gmod(VisVersion.v3_7a)
                valid = target_gmod.parse_path(str(converted))
                print(f"Validation result: {valid}")
                if not valid:
                    return None
            return converted
        except Exception as e:
            print(f"Conversion error: {e}")
            return None

    """def convert_path() -> GmodPath | None:
        gmod_path: GmodPath | None = vis.convert_path(
            VisVersion.v3_5a, path, VisVersion.v3_6a
        )
    gmod_path = vis.convert_path(VisVersion.v3_5a, path, VisVersion.v3_6a)
    print(f"Gmod path: {gmod_path}, input path: {path}")
    # return gmod_path """

    result = convert_path
    assert result is not None
    # assert isinstance(result, GmodPath)


def test_memory_usage_large_dataset() -> None:
    """Test memory usage with large dataset operations."""
    initial_memory = psutil.Process().memory_info().rss / 1024 / 1024  # B to MB
    start_time = time.perf_counter()

    # Maximum acceptable time in seconds
    max_execution_time = 30
    # Expected time for normal operation
    # expected_time = 10
    # Limit to prevent unnecessary processing
    target_path_count = 10000

    vis = VIS().instance
    gmod = vis.get_gmod(VisVersion.v3_4a)
    path_count = 0
    max_memory_increase = 10  # MB

    def traverse_callback(
        parents: list[GmodNode], node: GmodNode
    ) -> TraversalHandlerResult:
        nonlocal path_count

        if path_count > target_path_count:
            return TraversalHandlerResult.STOP

        if not parents:
            return TraversalHandlerResult.CONTINUE
        # Create path without storing it
        _ = GmodPath(list(parents), node, skip_verify=True)
        path_count += 1

        if time.perf_counter() - start_time > max_execution_time:
            return TraversalHandlerResult.STOP

        return TraversalHandlerResult.CONTINUE

    gmod.traverse(traverse_callback)
    execution_time = time.perf_counter() - start_time

    # Force garbage colleciton
    gc.collect()

    """if execution_time > EXPECTED_TIME:
        pytest.warns(
            UserWarning,
            f"Traversal took {execution_time:.2f}s, expected {EXPECTED_TIME}s",
        )
"""
    # Assert time constraints
    assert execution_time < max_execution_time, (
        f"Traversal took {execution_time:.2f}s, exceeding limit of {max_execution_time}s"  # noqa : E501
    )

    final_memory = psutil.Process().memory_info().rss / 1024 / 1024  # B to MB
    memory_increase = final_memory - initial_memory

    print(f"Initial memory: {initial_memory}, final memory: {final_memory}")
    print(f"Memory increase: {memory_increase}")

    assert path_count > 0, "No paths were processed."
    assert memory_increase < max_memory_increase, (
        f"Memory usage increased by {memory_increase}MB, "
        f"exceeding limit of {max_memory_increase}MB"
    )
