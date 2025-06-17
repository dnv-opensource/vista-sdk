"""Test data generator for load testing."""

from dataclasses import dataclass
from types import NoneType

from vista_sdk.gmod import Gmod


from tests.testdata import TestData

from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion, VisVersionExtension


@dataclass
class TestDataConfig:
    """Configuration for testdata generation."""

    base_version: VisVersion = VisVersion.v3_4a
    batch_size: int = 100


class TestDataGenerator:
    """Generator for test data sets using existing test data."""

    """vis: VIS | None = None
    gmod: Gmod | None = None
    _test_paths: list[str] | None = None

    @classmethod
    def create(cls, vis: VIS) -> "TestDataGenerator":
        generator = cls()
        generator.setup(vis)
        return generator"""

    def setup(self, vis: VIS) -> None:
        """Initializing the test data generator."""
        self.vis = vis
        self.gmod = vis.get_gmod(vis_version=VisVersion.v3_4a)
        self._test_paths = self._load_test_paths()

    def _load_test_paths(self) -> list[str]:
        """Load base test paths from existing test data."""
        test_data = TestData.get_gmodpath_data("GmodPaths")
        return [item.path for item in test_data.valid]

    def generate_large_dataset(self, size: int = 1000) -> list[GmodPath]:
        """Generate large dataset by replicating testpaths."""
        paths: list[GmodPath] = []
        base_paths = self._load_test_paths()

        print(f"Loaded {len(base_paths)} base paths")

        while len(paths) < size:
            for path_str in base_paths:
                if len(paths) >= size:
                    break
                try:
                    path = self.gmod.parse_path(path_str)
                    if not path:
                        print(f"Failed to parse path: {path_str}")
                        continue
                    paths.append(path)
                except Exception as e:
                    print(f"Error parsing path {path_str}: {e!s}")
                    continue

        print(f"Generated dataset with {len(paths)} paths")
        return paths
