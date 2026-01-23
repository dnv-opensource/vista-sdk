"""Performance benchmarks for Vista SDK."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture  # type: ignore

from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.benchmark(group="parsing")
def test_path_parsing_benchmark(benchmark: BenchmarkFixture) -> None:
    """Benchmark path parsing performance."""
    vis = VIS()
    gmod = vis.get_gmod(VisVersion.v3_6a)
    path_str = "411.1/C101.63/S206"

    def parse_path() -> GmodPath:
        return gmod.parse_path(path_str)

    result = benchmark(parse_path)
    assert result is not None
    assert isinstance(result, GmodPath)


@pytest.mark.benchmark(group="versioning")
def test_version_conversion_benchmark(benchmark: BenchmarkFixture) -> None:
    """Version conversion performance."""
    vis = VIS()
    source_gmod = vis.get_gmod(VisVersion.v3_6a)
    path = source_gmod.parse_path("411.1/C101.63/S206")

    def convert_path() -> GmodPath | None:
        try:
            # Get versioning
            versioning = vis.get_gmod_versioning()

            # Try conversion
            return versioning.convert_path(VisVersion.v3_6a, path, VisVersion.v3_7a)
        except Exception as e:
            print(f"Conversion error: {e!s}")
            return None

    result = benchmark(convert_path)
    assert result is not None
    assert isinstance(result, GmodPath)
