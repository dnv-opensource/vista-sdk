"""This module is designed to test the performance on GMOD path parsing."""

import json
import time
from pathlib import Path

import memory_profiler  # type: ignore
import pytest
from pytest_benchmark.fixture import BenchmarkFixture  # type: ignore

from vista_sdk.gmod_path import GmodPath
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.fixture(
    params=[
        VisVersion.v3_5a,
        VisVersion.v3_6a,
        VisVersion.v3_7a,
        VisVersion.v3_8a,
    ]
)
def vis_version(request) -> VisVersion:  # noqa: ANN001
    """Fixture that provides VIS versions for testing."""
    return request.param


class TestPerformance:
    """Module to test the performance on path parsing."""

    def load_test_paths(self) -> list[tuple[str, str]]:
        """Load test paths from JSON file."""
        json_path = Path(__file__).parent.parent / "testdata" / "GmodPaths.json"
        with Path.open(json_path, "r") as f:
            data = json.load(f)

        test_cases = []
        # Include valid paths
        for case in data["Valid"]:
            test_cases.append((case["path"], case["visVersion"]))
        # Optionally include invalid paths
        # for case in data["Invalid"]:
        #     test_cases.append((case["path"], case["visVersion"]))
        return test_cases

    def test_path_parsing_benchmark(
        self, benchmark: BenchmarkFixture, vis_version: VisVersion | None
    ) -> None:
        """Test path parsing performance using paths from JSON file."""
        if vis_version is None:
            vis_version = VisVersion.v3_5a

        vis = VIS()
        gmod = vis.get_gmod(vis_version)

        # Load test paths
        test_paths = self.load_test_paths()

        def parse_paths() -> list[GmodPath]:
            results = []
            for path_str, version in test_paths:
                enum_version = f"v{version.replace('-', '_')}"
                if VisVersion[enum_version] == vis_version:
                    try:
                        path = gmod.parse_path(path_str)
                        results.append(path)
                    except Exception as e:
                        print(f"Error parsing path {path_str}: {e!s}")
            return results

        results = benchmark(parse_paths)

        # Verify results
        assert results is not None
        for result in results:
            assert isinstance(result, GmodPath)
            # Get the normalized path string for comparison
            result_str = str(result).lstrip("/")  # Remove leading slash if present
            # Verify roundtrip conversion
            assert any(result_str == p[0] for p in test_paths), (
                f"Path {result_str} not found in test cases"
            )

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
    def test_memory_usage(self, vis_version: VisVersion) -> None:
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
