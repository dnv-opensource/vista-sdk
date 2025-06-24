"""Test module for path conversion functionality in the Vista SDK."""

import json
import time
import unittest
from pathlib import Path
from typing import TypedDict

import pytest

from tests.test_vis import TestVis
from vista_sdk.client import Client
from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion, VisVersionExtension, VisVersions


class PathTestCase(TypedDict):
    """Model for the test data."""

    path: str
    visVersion: str


def load_test_paths() -> tuple[list[PathTestCase], list[PathTestCase]]:
    """Load test paths from GmodPaths.json."""
    test_data_path = Path(__file__).parent / "testdata" / "GmodPaths.json"
    with test_data_path.open() as f:
        data = json.load(f)
    return data["Valid"], data["Invalid"]


class TestPathConversion(unittest.TestCase):
    """."""

    def setUp(self) -> None:
        """Initializing the test data generator."""
        self.valid_paths, self.invalid_paths = load_test_paths()
        self.vis = TestVis.get_vis()
        self.gmod = Client.get_gmod_test(
            VisVersionExtension.to_version_string(VisVersion.v3_6a)
        )

    def test_convert_path_validation(self) -> None:
        """Test path validation during conversion."""
        cases = [case for case in self.valid_paths if "-" in case["path"]]

        for case in cases:
            with self.subTest(input_path=case["path"]):
                source_version = VisVersions.parse(case["visVersion"])
                source_path = GmodPath.parse(case["path"], source_version)
                result = self.vis.convert_path(
                    VisVersion.v3_5a, source_path, VisVersion.v3_6a
                )

                print(f"result: {result}")

                assert result is None
                # assert str(result)

    def test_path_conversion_performance(self) -> None:
        """Test performance of path conversion."""
        vis = VIS().instance
        source_gmod = vis.get_gmod(VisVersion.v3_6a)
        paths = []

        def collect_paths(
            parents: list[GmodNode], node: GmodNode
        ) -> TraversalHandlerResult:
            if parents:
                paths.append(GmodPath(list(parents), node, skip_verify=True))
            return TraversalHandlerResult.CONTINUE

        source_gmod.traverse(collect_paths)

        start_time = time.perf_counter()
        success_count = 0
        failed_count = 0

        for path in paths[:1000]:  # Test with 1000 paths
            try:
                result = vis.convert_path(VisVersion.v3_6a, path, VisVersion.v3_7a)
                if result:
                    success_count += 1
                else:
                    failed_count += 1
            except Exception as e:
                failed_count += 1
                print(f"Conversion failed for {path}: {e}")

        end_time = time.perf_counter()

        # Calculate success rate
        total = success_count + failed_count
        success_rate = (success_count / total) * 100

        print(f"Converted {total} paths in {end_time - start_time:.2f} seconds")
        print(f"Success rate: {success_rate:.2f}%")

        assert success_rate > 95.0  # Expect at least 95% success rate

    def test_error_handling(self) -> None:
        """Test error handling in path conversion."""
        with pytest.raises(ValueError, match="Source version cannot be None"):
            self.vis.convert_path(None, None, None)  # type: ignore[call-arg]

        with pytest.raises(ValueError, match="Path cannot be None"):
            self.vis.convert_path(VisVersion.v3_6a, None, VisVersion.v3_7a)  # type: ignore[call-arg]

        # Test invalid version combinations
        path = GmodPath.parse("411.1", VisVersion.v3_4a)
        with pytest.raises(
            ValueError, match="Source version must be lower than target version"
        ):
            self.vis.convert_path(
                VisVersion.v3_7a,  # Higher version
                path,
                VisVersion.v3_6a,  # Lower version
            )

    def test_property_preservation(self) -> None:
        """Test preservation of node properties during conversion."""
        source_path = GmodPath.parse("411.1-1/C101.71/I101", VisVersion.v3_6a)
        target_path = self.vis.convert_path(
            VisVersion.v3_6a, source_path, VisVersion.v3_7a
        )

        assert target_path is not None

        # Check property preservation
        for source_node, target_node in zip(
            source_path.get_full_path(), target_path.get_full_path(), strict=False
        ):
            # Check location
            assert source_node[1].location == target_node[1].location

            # Check metadata
            assert source_node[1].metadata.type == target_node[1].metadata.type
            assert source_node[1].metadata.category == target_node[1].metadata.category

            # Check normal assignments
            if source_node[1].product_type:
                assert target_node[1].product_type is not None
