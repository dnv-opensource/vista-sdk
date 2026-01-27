"""Gmod lookup benchmarks."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.gmod import Gmod
from vista_sdk.gmod_node import GmodNode
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.benchmark(group="gmod")
class TestGmodLookup:
    """Benchmark tests for Gmod node lookup operations."""

    @pytest.fixture(scope="class")
    def gmod(self) -> Gmod:
        """Load Gmod for benchmarking."""
        vis = VIS()
        return vis.get_gmod(VisVersion.v3_4a)

    def test_lookup_by_code(self, benchmark: BenchmarkFixture, gmod: Gmod) -> None:
        """Benchmark looking up a node by its code using __getitem__."""
        code = "411.1"

        def lookup() -> GmodNode:
            return gmod[code]

        config = BenchmarkConfig(
            group="GmodLookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Lookup by code",
        )
        result = run_benchmark(benchmark, lookup, config)
        assert result is not None
        assert result.code == code

    def test_try_get_node(self, benchmark: BenchmarkFixture, gmod: Gmod) -> None:
        """Benchmark looking up a node using try_get_node."""
        code = "411.1"

        def try_get() -> tuple[bool, GmodNode | None]:
            return gmod.try_get_node(code)

        config = BenchmarkConfig(
            group="GmodLookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Try get node",
        )
        result = run_benchmark(benchmark, try_get, config)
        assert result[0] is True
        assert result[1] is not None
        assert result[1].code == code

    def test_lookup_root_node(self, benchmark: BenchmarkFixture, gmod: Gmod) -> None:
        """Benchmark looking up the root node."""
        code = "VE"

        def lookup_root() -> GmodNode:
            return gmod[code]

        config = BenchmarkConfig(
            group="GmodLookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Lookup root node",
        )
        result = run_benchmark(benchmark, lookup_root, config)
        assert result is not None
        assert result.code == code

    def test_lookup_deep_node(self, benchmark: BenchmarkFixture, gmod: Gmod) -> None:
        """Benchmark looking up a deeply nested node."""
        code = "C101.72"

        def lookup_deep() -> GmodNode:
            return gmod[code]

        config = BenchmarkConfig(
            group="GmodLookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Lookup deep node",
        )
        result = run_benchmark(benchmark, lookup_deep, config)
        assert result is not None
        assert result.code == code
