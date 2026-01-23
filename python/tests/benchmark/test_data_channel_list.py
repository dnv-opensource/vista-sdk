"""Data channel list serialization benchmarks matching C# implementation."""

import bz2
import io
from pathlib import Path

import brotli
import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)
from vista_sdk.system_text_json import Serializer as JsonSerializer
from vista_sdk.system_text_json.data_channel_list import DataChannelListPackage


@pytest.mark.benchmark(group="transport")
class TestDataChannelListSerialization:
    """Mirror of C#'s DataChannelListSerialization benchmark class."""

    @pytest.fixture(scope="class")
    def setup_data(self) -> dict:
        """Set up test data for benchmarks."""
        # Load DataChannelList JSON file
        test_file_path = Path(__file__).resolve()
        json_path = (
            test_file_path.parent.parent / "transport" / "json" / "DataChannelList.json"
        )

        with json_path.open() as f:
            json_str = f.read()

        # Deserialize using JsonSerializer
        data_channel_list = JsonSerializer.deserialize_data_channel_list(json_str)

        return {
            "json_string": json_str,
            "data_channel_list": data_channel_list,
            "memory_stream": io.BytesIO(),
            "compression_stream": io.BytesIO(),
        }

    def test_json_deserialize(
        self, benchmark: BenchmarkFixture, setup_data: dict
    ) -> None:
        """Benchmark JSON deserialization."""
        json_string = setup_data["json_string"]

        def deserialize() -> DataChannelListPackage:
            return JsonSerializer.deserialize_data_channel_list(json_string)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json Deserialize",
        )
        result = run_benchmark(benchmark, deserialize, config)
        assert result is not None

    def test_json_serialize(
        self, benchmark: BenchmarkFixture, setup_data: dict
    ) -> None:
        """Benchmark JSON serialization."""
        data_channel_list = setup_data["data_channel_list"]

        def serialize() -> str:
            return JsonSerializer.serialize(data_channel_list)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json Serialize",
        )
        result = run_benchmark(benchmark, serialize, config)
        assert result is not None
        assert len(result) > 0

    @pytest.mark.parametrize("compression_level", [5, 9])
    def test_json_bzip2(
        self, benchmark: BenchmarkFixture, setup_data: dict, compression_level: int
    ) -> None:
        """Mirror of C#'s Json_Bzip2 benchmark method."""
        data_channel_list = setup_data["data_channel_list"]

        def serialize_and_compress() -> bytes:
            json_str = JsonSerializer.serialize(data_channel_list)
            return bz2.compress(json_str.encode("utf-8"), compression_level)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description=f"Json+Bzip2 (level={compression_level})",
        )
        result = run_benchmark(benchmark, serialize_and_compress, config)
        assert result is not None
        assert len(result) > 0

    def test_json_brotli(self, benchmark: BenchmarkFixture, setup_data: dict) -> None:
        """Mirror of C#'s Json_Brotli benchmark method."""
        data_channel_list = setup_data["data_channel_list"]

        def serialize_and_compress() -> bytes:
            json_str = JsonSerializer.serialize(data_channel_list)
            return brotli.compress(json_str.encode("utf-8"), quality=11)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json+Brotli (quality=11)",
        )
        result = run_benchmark(benchmark, serialize_and_compress, config)
        assert result is not None
        assert len(result) > 0
