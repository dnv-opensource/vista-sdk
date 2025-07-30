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
            print(f"Loading GMOD for version {VisVersion.v3_4a}")
            gmod = vis.get_gmod(VisVersion.v3_4a)
            if gmod is None:
                raise ValueError("Failed to load GMOD")
            print("GMOD loaded successfully")

            print(f"Loading Locations for version {VisVersion.v3_4a}")
            locations = vis.get_locations(VisVersion.v3_4a)
            if locations is None:
                raise ValueError("Failed to load Locations")
            print("Locations loaded successfully")

            return {"gmod": gmod, "locations": locations}
        except Exception as e:
            print(f"Error during setup: {e!s}")
            raise

    def test_try_parse(
        self, benchmark: BenchmarkFixture, setup_components: dict
    ) -> None:
        """Mirror of C#'s TryParse benchmark method - No location category."""
        gmod = setup_components["gmod"]
        locations = setup_components["locations"]

        def try_parse() -> bool:
            success, _ = GmodPath.try_parse("411.1/C101.72/I101", locations, gmod)
            return success

        config = BenchmarkConfig(
            group="GmodPathParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="No location",
        )
        result = run_benchmark(benchmark, try_parse, config)
        assert result is True

    def test_try_parse_full_path(
        self, benchmark: BenchmarkFixture, setup_components: dict
    ) -> None:
        """Mirror of C#'s TryParseFullPath benchmark method - No location category."""
        gmod = setup_components["gmod"]
        locations = setup_components["locations"]

        def try_parse_full_path() -> bool:
            success, _ = GmodPath.try_parse_full_path(
                "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101",
                gmod,
                locations,
            )
            return success

        config = BenchmarkConfig(
            group="GmodPathParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="No location",
        )
        result = run_benchmark(benchmark, try_parse_full_path, config)
        assert result is True

    def test_try_parse_individualized(
        self, benchmark: BenchmarkFixture, setup_components: dict
    ) -> None:
        """Mirror of C#'s TryParseIndividualized benchmark method - With location."""
        gmod = setup_components["gmod"]
        locations = setup_components["locations"]

        def try_parse_individualized() -> bool:
            success, _ = GmodPath.try_parse("612.21-1/C701.13/S93", locations, gmod)
            return success

        config = BenchmarkConfig(
            group="GmodPathParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="With location",
        )
        result = run_benchmark(benchmark, try_parse_individualized, config)
        assert result is True

    def test_try_parse_full_path_individualized(
        self, benchmark: BenchmarkFixture, setup_components: dict
    ) -> None:
        """Mirror of C#'s TryParseFullPathIndividualized benchmark method."""
        gmod = setup_components["gmod"]
        locations = setup_components["locations"]

        def try_parse_full_path_individualized() -> bool:
            success, _ = GmodPath.try_parse_full_path(
                "VE/600a/610/612/612.2/612.2i/612.21-1/CS10/C701/C701.1/C701.13/S93",
                gmod,
                locations,
            )
            return success

        config = BenchmarkConfig(
            group="GmodPathParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="With location",
        )
        result = run_benchmark(benchmark, try_parse_full_path_individualized, config)
        assert result is True
