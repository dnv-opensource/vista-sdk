"""This module is designed to test the performance on GMOD path parsing."""

import json
from pathlib import Path

import pytest
from pytest_benchmark.fixture import BenchmarkFixture  # type: ignore

from vista_sdk.gmod_path import GmodPath
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

    @pytest.mark.benchmark(group="parsing")
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
