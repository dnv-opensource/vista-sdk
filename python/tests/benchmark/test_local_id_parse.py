"""LocalId parsing benchmarks."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.local_id import LocalId

# Sample LocalIds of varying complexity
SIMPLE_LOCAL_ID = "/dnv-v2/vis-3-4a/751/I101/meta/state-common.alarm"
MEDIUM_LOCAL_ID = (
    "/dnv-v2/vis-3-4a/621.11i-P/H135/meta/qty-temperature/cnt-heavy.fuel.oil"
)
COMPLEX_LOCAL_ID = (
    "/dnv-v2/vis-3-4a/1036.11/S90.3/S61/sec/1036.13i-1/C662.1/C661/"
    "meta/state-auto.control/detail-blow.off"
)


@pytest.mark.benchmark(group="localid")
class TestLocalIdParse:
    """LocalId parsing benchmarks."""

    def test_parse_simple(self, benchmark: BenchmarkFixture) -> None:
        """Parse a simple LocalId with minimal metadata."""

        def parse() -> LocalId:
            return LocalId.parse(SIMPLE_LOCAL_ID)

        config = BenchmarkConfig(
            group="LocalIdParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Simple LocalId",
        )
        result = run_benchmark(benchmark, parse, config)
        assert result is not None

    def test_parse_medium(self, benchmark: BenchmarkFixture) -> None:
        """Parse a medium complexity LocalId."""

        def parse() -> LocalId:
            return LocalId.parse(MEDIUM_LOCAL_ID)

        config = BenchmarkConfig(
            group="LocalIdParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Medium LocalId",
        )
        result = run_benchmark(benchmark, parse, config)
        assert result is not None

    def test_parse_complex(self, benchmark: BenchmarkFixture) -> None:
        """Parse a complex LocalId with secondary item and multiple metadata."""

        def parse() -> LocalId:
            return LocalId.parse(COMPLEX_LOCAL_ID)

        config = BenchmarkConfig(
            group="LocalIdParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Complex LocalId with secondary item",
        )
        result = run_benchmark(benchmark, parse, config)
        assert result is not None

    def test_try_parse_simple(self, benchmark: BenchmarkFixture) -> None:
        """Try parse a simple LocalId (includes validation overhead)."""

        def try_parse() -> bool:
            success, _, _ = LocalId.try_parse(SIMPLE_LOCAL_ID)
            return success

        config = BenchmarkConfig(
            group="LocalIdParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Try parse simple",
        )
        result = run_benchmark(benchmark, try_parse, config)
        assert result is True

    def test_try_parse_complex(self, benchmark: BenchmarkFixture) -> None:
        """Try parse a complex LocalId (includes validation overhead)."""

        def try_parse() -> bool:
            success, _, _ = LocalId.try_parse(COMPLEX_LOCAL_ID)
            return success

        config = BenchmarkConfig(
            group="LocalIdParse",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Try parse complex",
        )
        result = run_benchmark(benchmark, try_parse, config)
        assert result is True
