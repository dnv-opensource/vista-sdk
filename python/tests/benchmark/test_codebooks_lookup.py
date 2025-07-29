"""Codebooks lookup benchmarks matching C# implementation."""

from functools import cache  # We'll use this to simulate frozen behavior

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.codebook import Codebook
from vista_sdk.codebook_names import CodebookName
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@pytest.mark.benchmark(group="codebooks")
class TestCodebooksLookup:
    """Mirror of C#'s CodebooksLookup benchmark class."""

    @pytest.fixture(scope="class")
    def setup_codebooks(self) -> dict:
        """Mirror of C#'s Setup method."""
        vis = VIS()
        codebooks = vis.get_codebooks(VisVersion.v3_7a)

        # Create dictionary equivalent to C# Dictionary
        dict_codebooks: dict[CodebookName, Codebook] = {}
        for name, codebook in codebooks:
            dict_codebooks[name] = codebook

        # Create a cached dict lookup to simulate FrozenDictionary
        @cache
        def frozen_dict_get(key: CodebookName) -> Codebook | None:
            return dict_codebooks.get(key)

        return {
            "codebooks": codebooks,
            "dict": dict_codebooks,
            "frozen_dict": frozen_dict_get,
        }

    def test_dict_lookup(self, benchmark: BenchmarkFixture, setup_codebooks) -> None:  # noqa: ANN001
        """Mirror of C#'s Dict benchmark method."""
        print("Running dict_lookup benchmark")
        dict_codebooks = setup_codebooks["dict"]

        def dict_lookup() -> bool:
            return (
                CodebookName.Quantity in dict_codebooks
                and CodebookName.Type in dict_codebooks
                and CodebookName.Detail in dict_codebooks
            )

        config = BenchmarkConfig(
            group="CodebooksLookup",
            baseline=True,
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.SlowestToFastest,
        )
        result = run_benchmark(benchmark, dict_lookup, config)
        assert result is True

    def test_frozen_dict_lookup(
        self,
        benchmark: BenchmarkFixture,
        setup_codebooks,  # noqa: ANN001
    ) -> None:
        """Mirror of C#'s FrozenDict benchmark method."""
        frozen_dict_get = setup_codebooks["frozen_dict"]

        def frozen_dict_lookup() -> bool:
            return all(
                frozen_dict_get(name) is not None
                for name in (
                    CodebookName.Quantity,
                    CodebookName.Type,
                    CodebookName.Detail,
                )
            )

        config = BenchmarkConfig(
            group="CodebooksLookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.SlowestToFastest,
        )
        result = run_benchmark(benchmark, frozen_dict_lookup, config)
        assert result is True

    def test_codebooks_lookup(
        self,
        benchmark: BenchmarkFixture,
        setup_codebooks,  # noqa: ANN001
    ) -> None:
        """Mirror of C#'s Codebooks benchmark method."""
        codebooks = setup_codebooks["codebooks"]

        def codebooks_lookup() -> bool:
            a = codebooks.get_codebook(CodebookName.Quantity)
            b = codebooks.get_codebook(CodebookName.Type)
            c = codebooks.get_codebook(CodebookName.Detail)
            return all(x is not None for x in (a, b, c))

        config = BenchmarkConfig(
            group="CodebooksLookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.SlowestToFastest,
        )
        result = run_benchmark(benchmark, codebooks_lookup, config)
        assert result is True
