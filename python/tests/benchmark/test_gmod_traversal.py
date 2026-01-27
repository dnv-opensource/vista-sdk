"""Gmod traversal benchmarks matching C# implementation."""

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.gmod import Gmod
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


class TestGmodTraversal:
    """Mirror of C#'s GmodTraversal benchmark class."""

    @pytest.fixture(scope="class")
    def setup_gmod(self) -> Gmod:
        """Mirror of C#'s [GlobalSetup] Setup method."""
        vis = VIS()
        return vis.get_gmod(VisVersion.v3_4a)

    @pytest.mark.benchmark(group="gmod")
    def test_full_traversal(
        self, benchmark: BenchmarkFixture, setup_gmod: Gmod
    ) -> None:
        """Mirror of C#'s FullTraversal benchmark method."""

        def full_traversal() -> bool:
            return setup_gmod.traverse(
                lambda parents, node: TraversalHandlerResult.CONTINUE  # noqa: ARG005
            )

        config = BenchmarkConfig(
            group="GmodTraversal",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
        )
        result = run_benchmark(benchmark, full_traversal, config)
        assert result is True
