"""Tests for GMOD versioning functionality."""

import json
import unittest
from pathlib import Path
from typing import TypedDict

from tests.test_vis import TestVis
from vista_sdk.client import Client
from vista_sdk.vis_version import VisVersion, VisVersionExtension, VisVersions


class PathTestCase(TypedDict):
    """Model for the testdata."""

    path: str
    visVersion: str


def load_test_paths() -> tuple[list[PathTestCase], list[PathTestCase]]:
    """Load test paths from GmodPaths.json."""
    test_data_path = Path(__file__).parent / "testdata" / "GmodPaths.json"
    with test_data_path.open() as f:
        data = json.load(f)
    return data["Valid"], data["Invalid"]


class TestGmodVersioning(unittest.TestCase):
    """Tests for GmodVersioning functionality."""

    def setUp(cls) -> None:  # noqa : N805
        """Set up the test environment."""
        cls.valid_paths, cls.invalid_paths = load_test_paths()
        cls.vis = TestVis.get_vis()
        cls.gmod = Client.get_gmod_versioning_test(
            VisVersionExtension.to_version_string(VisVersion.v3_6a)
        )

    def test_convert_path_with_locations(self) -> None:
        """Test path conversion with location preservation."""
        from vista_sdk.gmod_path import GmodPath

        # Filter test cases with locations
        test_cases = [case for case in self.valid_paths if "-" in case["path"]]

        for case in test_cases:
            with self.subTest(path=case["path"]):
                source_version = VisVersions.parse(case["visVersion"])
                source_path = GmodPath.parse(case["path"], source_version)

                target_version = VisVersion.v3_7a
                target_path = self.vis.convert_path(
                    source_version,
                    source_path,
                    target_version,
                )

                assert target_path is not None

                # Verify location preservation
                source_locations = [
                    n[1]
                    for n in source_path.get_full_path()
                    if n[1].location is not None
                ]
                target_locations = [
                    n[1]
                    for n in target_path.get_full_path()
                    if n[1].location is not None
                ]
                assert len(source_locations) == len(target_locations)
