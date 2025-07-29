"""Gmod path parsing benchmarks matching C# implementation."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.benchmark(group="gmod")
class TestGmodPathParse:
    """Mirror of C#'s GmodPathParse benchmark class."""

    @pytest.fixture(scope="class")
    def setup_components(self) -> dict:
        """Mirror of C#'s Setup method."""
        print("\nInitializing VIS components...")
        try:
            vis = VIS()
            print("VIS instance created successfully")

            # Load cache as in C# implementation
            print(f"Loading GMOD for version {VisVersion.v3_7a}")
            gmod = vis.get_gmod(VisVersion.v3_7a)
            if gmod is None:
                raise ValueError("Failed to load GMOD")
            print("GMOD loaded successfully")

            print(f"Loading Locations for version {VisVersion.v3_7a}")
            locations = vis.get_locations(VisVersion.v3_7a)
            if locations is None:
                raise ValueError("Failed to load Locations")
            print("Locations loaded successfully")

            return {"gmod": gmod, "locations": locations}
        except Exception as e:
            print(f"Error during setup: {e!s}")
            raise

    @pytest.mark.parametrize(
        ("category", "path_str"),
        [
            ("Short", "400"),
            ("Complex", "H346.11112"),
            ("WithLocation", "411.1/C101.72/I101"),
            ("FullPath", "411.1/C101.72/I101.12/S206.1"),
            ("WithDetail", "411.1/C101.72/I101.12/S206.1/D2"),
        ],
    )
    def test_path_parsing(
        self,
        benchmark: BenchmarkFixture,
        setup_components: dict,
        category: str,
        path_str: str,
    ) -> None:
        """Mirror of C#'s benchmark methods."""
        print(f"\nTesting path parsing for category: {category}, path: {path_str}")
        gmod = setup_components["gmod"]

        def parse_path() -> GmodPath:
            try:
                if category in ["Complex", "FullPath", "WithDetail"]:
                    ve_path = f"VE/{path_str}"
                    version = (
                        VisVersion.v3_4a if "I101.12" in path_str else gmod.vis_version
                    )
                    result = GmodPath.parse_full_path(ve_path, version)
                else:
                    result = gmod.parse_path(path_str)
                return result
            except Exception as e:
                print(f"\nError parsing path {path_str}: {e!s}")
                raise

        config = BenchmarkConfig(
            group="GmodPathParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description=category,
        )
        result = run_benchmark(benchmark, parse_path, config)
        assert isinstance(result, GmodPath)
        assert str(result) == path_str

    @pytest.mark.benchmark(group="gmod")
    def test_location_parsing(
        self, benchmark: BenchmarkFixture, setup_components: dict
    ) -> None:
        """Mirror of C#'s ParseLocation benchmark method."""
        print("\nTesting location parsing")
        locations = setup_components["locations"]
        location_str = "411.1/C101.72/I101"

        def parse_location() -> bool:
            try:
                success, result = locations.try_parse(location_str)
                return success and result is not None
            except Exception as e:
                print(f"\nError parsing location {location_str}: {e!s}")
                raise

        config = BenchmarkConfig(
            group="GmodPathParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Location",
        )
        result = run_benchmark(benchmark, parse_location, config)
        assert result is True
