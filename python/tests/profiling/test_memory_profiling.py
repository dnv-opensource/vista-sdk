"""Memory profiling tests for Vista SDK."""

import gc
import time

import memory_profiler
import psutil
import pytest

from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


class TestMemoryProfiling:
    """Memory profiling tests for various Vista SDK operations."""

    @pytest.mark.parametrize(
        "vis_version",
        [
            VisVersion.v3_5a,
            VisVersion.v3_6a,
            VisVersion.v3_7a,
            VisVersion.v3_8a,
        ],
    )
    # @pytest.mark.skip(reason="Long running memory test")
    def test_gmod_traversal_memory_usage(self, vis_version: VisVersion) -> None:
        """Test mamory usage while traversing through GMOD tree."""
        max_paths: int = 1000
        iterations: int = 100
        timeout: int = 60

        print(f"\nTesting memory usage for {vis_version.value}")
        start_time = time.time()
        vis: VIS = VIS()
        paths: list[GmodPath] = []

        def handler(p, n) -> TraversalHandlerResult:  # noqa : ANN001
            if len(paths) > max_paths:
                return TraversalHandlerResult.STOP
            paths.append(GmodPath(p, n))
            return TraversalHandlerResult.CONTINUE

        @memory_profiler.profile
        def memory_test() -> float | None:
            try:
                gmod = vis.get_gmod(vis_version)
                baseline_memory = memory_profiler.memory_usage()[0]
                print(f"Baseline memory: {baseline_memory:.2f} MiB")

                for i in range(iterations):
                    if time.time() - start_time > timeout:
                        print(f"Timeout reached after {i} iterations")
                        return None

                    paths.clear()
                    gmod.traverse(handler)

                    current_memory = memory_profiler.memory_usage()[0]
                    delta = current_memory - baseline_memory
                    if delta != 0 and i >= 1:
                        print(
                            f"Iteration {i + 1}/{iterations}: {current_memory:.2f} MiB (Δ {delta:+.2f} MiB)"  # noqa : E501
                        )

                end_time = time.time()
                print(f"Test completed in {end_time - start_time:.1f} seconds")
                return current_memory

            except Exception as e:
                print(f"Error during memory test: {e}")
                return None

        final_memory = memory_test()
        assert final_memory is not None, "Memory test failed to complete"

    def test_memory_usage_large_dataset(self) -> None:
        """Test memory usage with large dataset operations."""
        initial_memory = psutil.Process().memory_info().rss / 1024 / 1024  # B to MB
        start_time = time.perf_counter()

        # Maximum acceptable time in seconds
        max_execution_time = 60
        target_path_count = 10000

        vis = VIS()
        gmod = vis.get_gmod(VisVersion.v3_6a)
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
