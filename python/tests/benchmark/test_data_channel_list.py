"""Data channel list serialization benchmarks matching C# implementation."""

import json
from io import BytesIO
from pathlib import Path
import bz2
import brotli
from typing import Any, Dict, List

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from vista_sdk.transport.data_channel import DataChannel
from vista_sdk.transport.time_series_data import TimeSeriesData, TabularData, TabularDataSet

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    run_benchmark,
    MethodOrderPolicy,
    SummaryOrderPolicy
)


@pytest.mark.benchmark(group="transport")
class TestDataChannelListSerialization:
    """Mirror of C#'s DataChannelListSerialization benchmark class."""

    def serialize_time_series(self, time_series: TimeSeriesData) -> str:
        """Convert TimeSeriesData to JSON string."""
        data = {}
        if time_series.data_configuration:
            data["dataConfiguration"] = {
                "id": time_series.data_configuration.id,
                "version": time_series.data_configuration.version,
                "timestamp": time_series.data_configuration.timestamp.isoformat() if time_series.data_configuration.timestamp else None
            }

        if time_series.tabular_data:
            data["tabularData"] = [{
                "dataChannelIds": [
                    {"localId": str(dc_id.local_id), "shortId": dc_id.short_id}
                    for dc_id in td.data_channel_ids
                ] if td.data_channel_ids else [],
                "dataSets": [] if not td.data_sets else [
                    {"timestamp": ds.time_stamp.isoformat() if ds.time_stamp else None,
                     "quality": ds.quality,
                     "value": ds.value}
                    for ds in td.data_sets
                ]
            } for td in time_series.tabular_data]

        if time_series.custom_data_kinds:
            data["customDataKinds"] = time_series.custom_data_kinds

        import json
        return json.dumps(data)

    @pytest.fixture(scope="class")
    def setup_data(self):
        """Setup sample data for serialization benchmarks."""
        from vista_sdk.transport.data_channel.data_channel import (
            DataChannel, DataChannelId, DataChannelType, Property, Format, Range, Unit,
            NameObject
        )
        from vista_sdk.local_id import LocalId
        from vista_sdk.local_id_builder import LocalIdBuilder
        from vista_sdk.vis_version import VisVersion
        from vista_sdk.codebook_names import CodebookName
        from vista_sdk.vis import VIS
        from vista_sdk.gmod_path import GmodPath
        def create_local_id(i: int) -> LocalId:
            vis = VIS()
            codebooks = vis.get_codebooks(VisVersion.v3_7a)
            builder = LocalIdBuilder()
            builder = builder.with_vis_version(VisVersion.v3_7a)

            # Add a primary item which is required for a valid local ID
            path_str = "411.1/C101.72/I101"  # Using a known valid path
            success, path = GmodPath.try_parse(path_str, VisVersion.v3_7a)
            if not success or path is None:
                raise ValueError(f"Failed to parse path: {path_str}")
            builder = builder.with_primary_item(path)

            # Add metadata tags which are required for a valid local ID
            state_tag = codebooks.try_create_tag(CodebookName.State, "common.alarm")
            if state_tag is None:
                raise ValueError("Failed to create state tag")
            builder = builder.with_metadata_tag(state_tag)

            detail_tag = codebooks.try_create_tag(CodebookName.Detail, f"sensor.{i}")
            if detail_tag is None:
                raise ValueError("Failed to create detail tag")
            builder = builder.with_metadata_tag(detail_tag)

            return LocalId(builder)

        channels = [
            DataChannel(
                data_channel_id=DataChannelId(
                    local_id=create_local_id(i),
                    short_id=f"TC{i}",
                    name_object=NameObject()
                ),
                prop=Property(
                    data_channel_type=DataChannelType("Inst", None, None),
                    data_format=Format("String"),
                    data_range=None,  # Not needed for String format
                    unit=None,  # Not needed for String format
                    name=f"Test Channel {i}"
                )
            )
            for i in range(100)
        ]

        # Create data channel IDs from the channels
        data_channel_ids = [channel.data_channel_id for channel in channels]

        # Create a TabularData instance
        tabular_data = [
            TabularData(
                data_channel_ids=data_channel_ids,
                data_sets=[]  # You can add TabularDataSet instances here if needed
            )
        ]

        # Create TimeSeriesData with the correct structure
        time_series = TimeSeriesData(
            data_configuration=None,  # Add configuration if needed
            tabular_data=tabular_data,
            event_data=None,
            custom_data_kinds=None
        )

        memory_stream = BytesIO()
        compression_stream = BytesIO()

        return {
            'time_series': time_series,
            'memory_stream': memory_stream,
            'compression_stream': compression_stream
        }

    def test_json(self, benchmark: BenchmarkFixture, setup_data) -> None:
        """Benchmark JSON serialization."""
        memory_stream = setup_data['memory_stream']
        time_series = setup_data['time_series']

        def serialize_json() -> None:
            memory_stream.seek(0)
            memory_stream.truncate()
            json_str = self.serialize_time_series(time_series)
            memory_stream.write(json_str.encode('utf-8'))

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json"
        )
        run_benchmark(benchmark, serialize_json, config)
        assert memory_stream.tell() > 0

    @pytest.mark.parametrize("compression_level", [5, 9])
    def test_json_bzip2(self, benchmark: BenchmarkFixture, setup_data, compression_level: int) -> None:
        """Mirror of C#'s Json_Bzip2 benchmark method."""
        memory_stream = setup_data['memory_stream']
        compression_stream = setup_data['compression_stream']
        time_series = setup_data['time_series']

        def serialize_json_bzip2() -> None:
            memory_stream.seek(0)
            memory_stream.truncate()
            compression_stream.seek(0)
            compression_stream.truncate()

            # Convert time_series to JSON and write to memory stream
            json_str = self.serialize_time_series(time_series)
            memory_stream.write(json_str.encode('utf-8'))
            memory_stream.seek(0)

            compressed = bz2.compress(memory_stream.getvalue(), compression_level)
            compression_stream.write(compressed)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json-Bzip2"
        )
        run_benchmark(benchmark, serialize_json_bzip2, config)
        assert compression_stream.tell() > 0

    def test_json_brotli(self, benchmark: BenchmarkFixture, setup_data) -> None:
        """Mirror of C#'s Json_Brotli benchmark method."""
        memory_stream = setup_data['memory_stream']
        compression_stream = setup_data['compression_stream']
        time_series = setup_data['time_series']

        def serialize_json_brotli() -> None:
            memory_stream.seek(0)
            memory_stream.truncate()
            compression_stream.seek(0)
            compression_stream.truncate()

            # Convert time_series to JSON and write to memory stream
            json_str = self.serialize_time_series(time_series)
            memory_stream.write(json_str.encode('utf-8'))
            memory_stream.seek(0)

            compressed = brotli.compress(memory_stream.getvalue(), quality=11)
            compression_stream.write(compressed)

        config = BenchmarkConfig(
            group="DataChannelListSerialization",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Json-Brotli"
        )
        run_benchmark(benchmark, serialize_json_brotli, config)
        assert compression_stream.tell() > 0
