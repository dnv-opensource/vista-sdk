"""ISO19848 benchmarks for DataChannelList and TimeSeriesData."""

from pathlib import Path

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.local_id import LocalId
from vista_sdk.system_text_json import Serializer as JsonSerializer
from vista_sdk.system_text_json.data_channel_list import DataChannelListPackage
from vista_sdk.system_text_json.extensions import JsonExtensions
from vista_sdk.system_text_json.time_series_data import TimeSeriesDataPackage
from vista_sdk.transport.data_channel.data_channel import (
    DataChannelListPackage as DomainDataChannelListPackage,
)
from vista_sdk.transport.time_series_data.time_series_data import (
    TimeSeriesDataPackage as DomainTimeSeriesDataPackage,
)


@pytest.fixture(scope="module")
def test_data_paths() -> dict[str, Path]:
    """Get paths to test data files."""
    test_file_path = Path(__file__).resolve()
    json_dir = test_file_path.parent.parent / "transport" / "json"
    return {
        "dc_json_path": json_dir / "DataChannelList.json",
        "ts_json_path": json_dir / "TimeSeriesData.json",
    }


@pytest.fixture(scope="module")
def dc_json_string(test_data_paths: dict[str, Path]) -> str:
    """Load DataChannelList JSON string."""
    with test_data_paths["dc_json_path"].open() as f:
        return f.read()


@pytest.fixture(scope="module")
def ts_json_string(test_data_paths: dict[str, Path]) -> str:
    """Load TimeSeriesData JSON string."""
    with test_data_paths["ts_json_path"].open() as f:
        return f.read()


@pytest.fixture(scope="module")
def dc_dto(dc_json_string: str) -> DataChannelListPackage:
    """Deserialize DataChannelList DTO."""
    return JsonSerializer.deserialize_data_channel_list(dc_json_string)


@pytest.fixture(scope="module")
def ts_dto(ts_json_string: str) -> TimeSeriesDataPackage:
    """Deserialize TimeSeriesData DTO."""
    return JsonSerializer.deserialize_time_series_data(ts_json_string)


@pytest.fixture(scope="module")
def dc_domain(dc_dto: DataChannelListPackage) -> DomainDataChannelListPackage:
    """Build domain DataChannelListPackage."""
    return JsonExtensions.DataChannelList.to_domain_model(dc_dto)


@pytest.fixture(scope="module")
def ts_domain(ts_dto: TimeSeriesDataPackage) -> DomainTimeSeriesDataPackage:
    """Build domain TimeSeriesDataPackage."""
    return JsonExtensions.TimeSeriesData.to_domain_model(ts_dto)


# =============================================================================
# DataChannelList: To Domain
# =============================================================================


@pytest.mark.benchmark(group="iso19848")
class TestDataChannelListToDomain:
    """Benchmark DataChannelList DTO to domain conversion."""

    def test_data_channel_list_to_domain(
        self, benchmark: BenchmarkFixture, dc_dto: DataChannelListPackage
    ) -> None:
        """Benchmark DataChannelList DTO to domain conversion."""

        def convert() -> DomainDataChannelListPackage:
            return JsonExtensions.DataChannelList.to_domain_model(dc_dto)

        config = BenchmarkConfig(
            group="ISO19848_DataChannelList_ToDomain",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="DataChannelList DTO to Domain",
        )
        result = run_benchmark(benchmark, convert, config)
        assert result is not None


# =============================================================================
# DataChannelList: To JSON
# =============================================================================


@pytest.mark.benchmark(group="iso19848")
class TestDataChannelListToJson:
    """Benchmark DataChannelList domain to JSON conversion."""

    def test_data_channel_list_to_json(
        self, benchmark: BenchmarkFixture, dc_domain: DomainDataChannelListPackage
    ) -> None:
        """Benchmark DataChannelList domain to JSON serialization."""
        dto = JsonExtensions.DataChannelList.to_json_dto(dc_domain)

        def serialize() -> str:
            return JsonSerializer.serialize(dto)

        config = BenchmarkConfig(
            group="ISO19848_DataChannelList_ToJson",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="DataChannelList Domain to JSON",
        )
        result = run_benchmark(benchmark, serialize, config)
        assert result is not None
        assert len(result) > 0


# =============================================================================
# TimeSeriesData: To Domain
# =============================================================================


@pytest.mark.benchmark(group="iso19848")
class TestTimeSeriesDataToDomain:
    """Benchmark TimeSeriesData DTO to domain conversion."""

    def test_time_series_data_to_domain(
        self,
        benchmark: BenchmarkFixture,
        ts_dto: TimeSeriesDataPackage,
    ) -> None:
        """Benchmark TimeSeriesData DTO to domain conversion."""

        def convert() -> DomainTimeSeriesDataPackage:
            return JsonExtensions.TimeSeriesData.to_domain_model(ts_dto)

        config = BenchmarkConfig(
            group="ISO19848_TimeSeriesData_ToDomain",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="TimeSeriesData DTO to Domain",
        )
        result = run_benchmark(benchmark, convert, config)
        assert result is not None


# =============================================================================
# TimeSeriesData: To JSON
# =============================================================================


@pytest.mark.benchmark(group="iso19848")
class TestTimeSeriesDataToJson:
    """Benchmark TimeSeriesData domain to JSON conversion."""

    def test_time_series_data_to_json(
        self, benchmark: BenchmarkFixture, ts_domain: DomainTimeSeriesDataPackage
    ) -> None:
        """Benchmark TimeSeriesData domain to JSON serialization."""
        dto = JsonExtensions.TimeSeriesData.to_json_dto(ts_domain)

        def serialize() -> str:
            return JsonSerializer.serialize(dto)

        config = BenchmarkConfig(
            group="ISO19848_TimeSeriesData_ToJson",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="TimeSeriesData Domain to JSON",
        )
        result = run_benchmark(benchmark, serialize, config)
        assert result is not None
        assert len(result) > 0


# =============================================================================
# DataChannelList: Lookup
# =============================================================================


@pytest.mark.benchmark(group="iso19848")
class TestDataChannelListLookup:
    """Benchmark DataChannelList lookup operations."""

    def test_lookup_by_short_id(
        self, benchmark: BenchmarkFixture, dc_domain: DomainDataChannelListPackage
    ) -> None:
        """Benchmark lookup by short ID."""
        dc_list = dc_domain.data_channel_list
        # Get a sample short_id from the first data channel that has one
        sample_short_id: str | None = None
        for dc in dc_list.data_channels:
            if dc.data_channel_id.short_id is not None:
                sample_short_id = dc.data_channel_id.short_id
                break

        if sample_short_id is None:
            pytest.skip("No data channels with short_id found")

        def lookup() -> bool:
            found, _ = dc_list.try_get_by_short_id(sample_short_id)  # type: ignore[arg-type]
            return found

        config = BenchmarkConfig(
            group="ISO19848_DataChannelList_Lookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Lookup by ShortId",
        )
        result = run_benchmark(benchmark, lookup, config)
        assert result is True

    def test_lookup_by_local_id(
        self, benchmark: BenchmarkFixture, dc_domain: DomainDataChannelListPackage
    ) -> None:
        """Benchmark lookup by LocalId."""
        dc_list = dc_domain.data_channel_list
        # Get a sample local_id from the first data channel
        sample_local_id: LocalId | None = None
        for dc in dc_list.data_channels:
            sample_local_id = dc.data_channel_id.local_id
            break

        if sample_local_id is None:
            pytest.skip("No data channels found")

        def lookup() -> bool:
            found, _ = dc_list.try_get_by_local_id(sample_local_id)  # type: ignore[arg-type]
            return found

        config = BenchmarkConfig(
            group="ISO19848_DataChannelList_Lookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Lookup by LocalId",
        )
        result = run_benchmark(benchmark, lookup, config)
        assert result is True

    def test_lookup_by_index(
        self, benchmark: BenchmarkFixture, dc_domain: DomainDataChannelListPackage
    ) -> None:
        """Benchmark lookup by index."""
        dc_list = dc_domain.data_channel_list
        if len(dc_list) == 0:
            pytest.skip("No data channels found")

        # Lookup middle element
        index = len(dc_list) // 2

        def lookup() -> object:
            return dc_list[index]

        config = BenchmarkConfig(
            group="ISO19848_DataChannelList_Lookup",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Lookup by Index",
        )
        result = run_benchmark(benchmark, lookup, config)
        assert result is not None
