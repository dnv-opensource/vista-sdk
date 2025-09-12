"""Data channel list serialization benchmarks matching C# implementation."""

import bz2
import io
import json
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
from vista_sdk.local_id import LocalId
from vista_sdk.transport.time_series_data import DataChannelId as TSDataChannelId


@pytest.mark.benchmark(group="transport")
class TestDataChannelListSerialization:
    """Mirror of C#'s DataChannelListSerialization benchmark class."""

    @pytest.fixture(scope="class")
    def setup_data(self) -> dict:
        """Set up test data for benchmarks."""
        # Load DataChannelList data - simplified approach
        # Path relative to workspace root, not python subdirectory
        test_file_path = Path(__file__).resolve()
        workspace_root = test_file_path.parent.parent.parent.parent
        schema_path = (
            workspace_root / "schemas" / "json" / "DataChannelList.sample.compact.json"
        )
        with schema_path.open() as f:
            self._json_data = json.load(f)

        # Create time series data channel IDs from the JSON data
        # Following C# pattern: use short_id if available, otherwise use local_id
        self.ts_data_channel_ids = []
        data_channels = self._json_data["Package"]["DataChannelList"]["DataChannel"]
        for data_channel in data_channels:
            channel_id = data_channel["DataChannelID"]
            if channel_id.get("ShortID"):
                # Use short ID if available
                ts_dc_id = TSDataChannelId.from_short_id(channel_id["ShortID"])
            else:
                # Fall back to local ID
                local_id = LocalId.parse(channel_id["LocalID"])
                ts_dc_id = TSDataChannelId.from_local_id(local_id)
            self.ts_data_channel_ids.append(ts_dc_id)

        # Pre-serialize the JSON data for benchmarks
        self._json_string = json.dumps(self._json_data)

        # Return data for test methods
        return {
            "json_data": self._json_data,
            "json_string": self._json_string,
            # Python equivalent of memory stream using BytesIO
            "memory_stream": io.BytesIO(),
            "compression_stream": io.BytesIO(),  # For compression tests
            "ts_data_channel_ids": self.ts_data_channel_ids,
        }

    def test_json(self, benchmark: BenchmarkFixture, setup_data: dict) -> None:
        """Benchmark JSON serialization."""
        """Mirror of C#'s Json benchmark method."""
        memory_stream = setup_data["memory_stream"]
        json_data = setup_data["json_data"]

        def serialize_json() -> None:
            memory_stream.seek(0)
            memory_stream.truncate()
            json_str = json.dumps(json_data)
            memory_stream.write(json_str.encode("utf-8"))

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json",
        )
        run_benchmark(benchmark, serialize_json, config)
        assert memory_stream.tell() > 0

    @pytest.mark.parametrize("compression_level", [5, 9])
    def test_json_bzip2(
        self, benchmark: BenchmarkFixture, setup_data: dict, compression_level: int
    ) -> None:
        """Mirror of C#'s Json_Bzip2 benchmark method."""
        memory_stream = setup_data["memory_stream"]
        compression_stream = setup_data["compression_stream"]
        json_data = setup_data["json_data"]

        def serialize_json_bzip2() -> None:
            memory_stream.seek(0)
            memory_stream.truncate()
            compression_stream.seek(0)
            compression_stream.truncate()

            # Convert JSON data to string and write to memory stream
            json_str = json.dumps(json_data)
            memory_stream.write(json_str.encode("utf-8"))
            memory_stream.seek(0)

            compressed = bz2.compress(memory_stream.getvalue(), compression_level)
            compression_stream.write(compressed)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json-Bzip2",
        )
        run_benchmark(benchmark, serialize_json_bzip2, config)
        assert compression_stream.tell() > 0

    def test_json_brotli(self, benchmark: BenchmarkFixture, setup_data: dict) -> None:
        """Mirror of C#'s Json_Brotli benchmark method."""
        memory_stream = setup_data["memory_stream"]
        compression_stream = setup_data["compression_stream"]
        json_data = setup_data["json_data"]

        def serialize_json_brotli() -> None:
            memory_stream.seek(0)
            memory_stream.truncate()
            compression_stream.seek(0)
            compression_stream.truncate()

            # Convert JSON data to string and write to memory stream
            json_str = json.dumps(json_data)
            memory_stream.write(json_str.encode("utf-8"))
            memory_stream.seek(0)

            compressed = brotli.compress(memory_stream.getvalue(), quality=11)
            compression_stream.write(compressed)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json-Brotli",
        )
        run_benchmark(benchmark, serialize_json_brotli, config)
        assert compression_stream.tell() > 0
        assert compression_stream.tell() > 0
