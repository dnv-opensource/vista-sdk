"""Gmod versioning conversion benchmarks matching C# implementation."""

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
from vista_sdk.vis_version import VisVersion, VisVersions


@pytest.mark.benchmark(group="gmod")
class TestGmodVersioningConvertPath:
    """Mirror of C#'s GmodVersioningConvertPath benchmark class."""

    @pytest.fixture(scope="class")
    def setup_conversion(self) -> dict:
        """Mirror of C#'s Setup method."""
        vis = VIS()
        gmod = vis.get_gmod(VisVersion.v3_4a)
        path = gmod.parse_path("411.1/C101.72/I101")
        return {"vis": vis, "gmod": gmod, "path": path}

    @pytest.mark.parametrize(
        ("source_version", "target_version"),
        [
            (source, target)
            for source, target in zip(
                VisVersions.all_versions(), VisVersions.all_versions()[1:], strict=False
            )
        ],
    )
    def test_convert_path(
        self,
        benchmark: BenchmarkFixture,
        setup_conversion,  # noqa: ANN001
        source_version: VisVersion,
        target_version: VisVersion,
    ) -> None:
        """Mirror of C#'s ConvertPath benchmark method."""
        # Skip v3_8a -> v3_9a conversion due to known issues
        if source_version == VisVersion.v3_8a and target_version == VisVersion.v3_9a:
            pytest.skip("Conversion from v3_8a to v3_9a not yet supported")

        vis = setup_conversion["vis"]
        path = setup_conversion["path"]

        def convert_path() -> GmodPath:
            result = vis.convert_path(source_version, path, target_version)
            if result is None:
                raise ValueError(
                    f"Conversion failed for {source_version} -> {target_version}"
                )
            return result

        config = BenchmarkConfig(
            group="GmodVersioningConvertPath",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description=f"{source_version} -> {target_version}",
        )
        result = run_benchmark(benchmark, convert_path, config)
        assert isinstance(result, GmodPath)
