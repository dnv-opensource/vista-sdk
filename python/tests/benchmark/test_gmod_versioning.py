"""Gmod versioning conversion benchmarks matching C# implementation."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    run_benchmark,
    MethodOrderPolicy,
    SummaryOrderPolicy
)


@pytest.mark.benchmark(group="gmod")
class TestGmodVersioningConvertPath:
    """Mirror of C#'s GmodVersioningConvertPath benchmark class."""

    @pytest.fixture(scope="class")
    def setup_conversion(self):
        """Mirror of C#'s Setup method."""
        vis = VIS()
        gmod = vis.get_gmod(VisVersion.v3_4a)
        path = gmod.parse_path("411.1/C101.72/I101")
        return {
            'vis': vis,
            'gmod': gmod,
            'path': path
        }

    @pytest.mark.parametrize("source_version,target_version", [
        (VisVersion.v3_4a, VisVersion.v3_5a),
        (VisVersion.v3_5a, VisVersion.v3_6a),
        (VisVersion.v3_6a, VisVersion.v3_7a),
        (VisVersion.v3_7a, VisVersion.v3_8a),
        (VisVersion.v3_8a, VisVersion.v3_9a)
    ])
    def test_convert_path(self, benchmark: BenchmarkFixture, setup_conversion,
                         source_version: VisVersion, target_version: VisVersion) -> None:
        """Mirror of C#'s ConvertPath benchmark method."""
        vis = setup_conversion['vis']
        path = setup_conversion['path']

        def convert_path() -> GmodPath:
            return vis.convert_path(source_version, path, target_version)

        config = BenchmarkConfig(
            group="GmodVersioningConvertPath",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description=f"{source_version} -> {target_version}"
        )
        result = run_benchmark(benchmark, convert_path, config)
        assert isinstance(result, GmodPath)
