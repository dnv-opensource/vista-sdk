"""Gmod loading benchmarks matching C# implementation."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.gmod import Gmod
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.benchmark(group="gmod")
class TestGmodLoad:
    """Mirror of C#'s GmodLoad benchmark class."""

    def test_load(self, benchmark: BenchmarkFixture) -> None:
        """Mirror of C#'s Load benchmark method."""

        def load_gmod() -> Gmod:
            vis = VIS()
            return vis.get_gmod(VisVersion.v3_7a)

        config = BenchmarkConfig(
            group="GmodLoad",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.SlowestToFastest,
            memory_tracking=True,
        )

        result = run_benchmark(benchmark, load_gmod, config)
        assert result is not None
        assert isinstance(result, Gmod)
