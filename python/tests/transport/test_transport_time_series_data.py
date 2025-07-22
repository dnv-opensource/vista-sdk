"""Tests for the transport time series data module."""

from datetime import datetime, timezone

import pytest

from vista_sdk.local_id import LocalId  # type: ignore
from vista_sdk.result import Invalid, Ok  # type: ignore
from vista_sdk.transport.ship_id import ShipId  # type: ignore
from vista_sdk.transport.time_series_data.data_channel_id import DataChannelId  # type: ignore
from vista_sdk.transport.time_series_data.time_series_data import (  # type: ignore
    ConfigurationReference as TimeSeriesConfigurationReference,
    EventData,
    EventDataSet,
    Header,
    Package,
    TabularData,
    TabularDataSet,
    TimeSeriesData,
    TimeSeriesDataPackage,
    TimeSpan,
)


def test_data_channel_id_local_id() -> None:
    """Test DataChannelId with LocalId."""
    local_id_str = "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    local_id = LocalId.parse(local_id_str)
    dc_id = DataChannelId.from_local_id(local_id)

    assert dc_id.is_local_id
    assert not dc_id.is_short_id
    assert dc_id.local_id == local_id
    assert dc_id.short_id is None
    assert str(dc_id) == str(local_id)


def test_data_channel_id_short_id() -> None:
    """Test DataChannelId with short ID."""
    short_id = "test-short-id"
    dc_id = DataChannelId.from_short_id(short_id)

    assert not dc_id.is_local_id
    assert dc_id.is_short_id
    assert dc_id.local_id is None
    assert dc_id.short_id == short_id
    assert str(dc_id) == short_id


def test_data_channel_id_parse_local_id() -> None:
    """Test parsing a valid LocalId string."""
    local_id_str = "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    dc_id = DataChannelId.parse(local_id_str)

    assert dc_id.is_local_id
    assert dc_id.local_id is not None
    assert str(dc_id.local_id) == local_id_str


def test_data_channel_id_parse_short_id() -> None:
    """Test parsing a string that can't be parsed as LocalId."""
    short_id = "simple-short-id"
    dc_id = DataChannelId.parse(short_id)

    assert dc_id.is_short_id
    assert dc_id.short_id == short_id


def test_data_channel_id_match() -> None:
    """Test pattern matching on DataChannelId."""
    local_id_str = "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    local_id = LocalId.parse(local_id_str)
    dc_id = DataChannelId.from_local_id(local_id)

    result = dc_id.match(
        on_local_id=lambda lid: f"LocalId: {lid}",
        on_short_id=lambda sid: f"ShortId: {sid}",
    )

    assert result == f"LocalId: {local_id}"


def test_data_channel_id_switch() -> None:
    """Test switching on DataChannelId."""
    short_id = "test-short-id"
    dc_id = DataChannelId.from_short_id(short_id)

    result = []
    dc_id.switch(
        on_local_id=lambda lid: result.append(f"LocalId: {lid}"),
        on_short_id=lambda sid: result.append(f"ShortId: {sid}"),
    )

    assert result == [f"ShortId: {short_id}"]


def test_data_channel_id_equality() -> None:
    """Test equality comparison of DataChannelIds."""
    local_id_str = "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    local_id = LocalId.parse(local_id_str)
    dc_id1 = DataChannelId.from_local_id(local_id)
    dc_id2 = DataChannelId.from_local_id(local_id)
    dc_id3 = DataChannelId.from_short_id("test")

    assert dc_id1 == dc_id2
    assert dc_id1 != dc_id3
    assert dc_id1 != "not a DataChannelId"


def test_data_channel_id_hash() -> None:
    """Test hashing of DataChannelIds."""
    local_id_str = "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    local_id = LocalId.parse(local_id_str)
    dc_id1 = DataChannelId.from_local_id(local_id)
    dc_id2 = DataChannelId.from_local_id(local_id)
    dc_id3 = DataChannelId.from_short_id("test")

    assert hash(dc_id1) == hash(dc_id2)
    assert hash(dc_id1) != hash(dc_id3)


def test_data_channel_id_invalid_construction() -> None:
    """Test invalid DataChannelId construction."""
    local_id_str = "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    local_id = LocalId.parse(local_id_str)

    # Both local_id and short_id provided
    with pytest.raises(ValueError, match="cannot have both"):
        DataChannelId(local_id=local_id, short_id="test")

    # Neither local_id nor short_id provided
    with pytest.raises(ValueError, match="must have either"):
        DataChannelId()


def test_time_span() -> None:
    """Test TimeSpan creation."""
    start = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    end = datetime(2023, 1, 2, 12, 0, 0, tzinfo=timezone.utc)

    time_span = TimeSpan(start, end)
    assert time_span.start == start
    assert time_span.end == end


def test_configuration_reference() -> None:
    """Test ConfigurationReference creation."""
    timestamp = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    config_ref = TimeSeriesConfigurationReference("test-config.xml", timestamp)

    assert config_ref.id == "test-config.xml"
    assert config_ref.time_stamp == timestamp


def test_header() -> None:
    """Test Header creation."""
    ship_id = ShipId("IMO1234567")
    start = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    end = datetime(2023, 1, 2, 12, 0, 0, tzinfo=timezone.utc)
    time_span = TimeSpan(start, end)

    header = Header(ship_id=ship_id, time_span=time_span, author="test-author")

    assert header.ship_id == ship_id
    assert header.time_span == time_span
    assert header.author == "test-author"
    assert header.date_created is not None  # Should be set to current time
    assert header.date_modified is None
    assert header.system_configuration is None
    assert header.custom_headers is None


def test_tabular_data_set() -> None:
    """Test TabularDataSet creation."""
    timestamp = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    values = ["100.0", "200.0", "300.0"]
    quality = ["0", "0", "0"]

    data_set = TabularDataSet(timestamp, values, quality)

    assert data_set.time_stamp == timestamp
    assert data_set.value == values
    assert data_set.quality == quality


def test_event_data_set() -> None:
    """Test EventDataSet creation."""
    timestamp = datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    dc_id = DataChannelId.from_short_id("test-channel")

    event_set = EventDataSet(timestamp, dc_id, "HIGH", "0")

    assert event_set.time_stamp == timestamp
    assert event_set.data_channel_id == dc_id
    assert event_set.value == "HIGH"
    assert event_set.quality == "0"


def test_tabular_data() -> None:
    """Test TabularData creation and properties."""
    dc_ids = [
        DataChannelId.from_short_id("channel1"),
        DataChannelId.from_short_id("channel2"),
    ]
    data_sets = [
        TabularDataSet(
            datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc),
            ["100.0", "200.0"],
            ["0", "0"],
        )
    ]

    tabular_data = TabularData(dc_ids, data_sets)

    assert tabular_data.data_channel_ids == dc_ids
    assert tabular_data.data_sets == data_sets
    assert tabular_data.number_of_data_channels == 2
    assert tabular_data.number_of_data_sets == 1


def test_tabular_data_validate_success() -> None:
    """Test successful TabularData validation."""
    dc_ids = [DataChannelId.from_short_id("channel1")]
    data_sets = [
        TabularDataSet(
            datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc), ["100.0"], ["0"]
        )
    ]

    tabular_data = TabularData(dc_ids, data_sets)
    result = TabularData.validate(tabular_data)

    assert isinstance(result, Ok)


def test_tabular_data_validate_no_channels() -> None:
    """Test TabularData validation with no data channels."""
    tabular_data = TabularData([], [])
    result = TabularData.validate(tabular_data)

    assert isinstance(result, Invalid)
    assert "no data channels" in result.messages[0]


def test_tabular_data_validate_no_data() -> None:
    """Test TabularData validation with no data sets."""
    dc_ids = [DataChannelId.from_short_id("channel1")]
    tabular_data = TabularData(dc_ids, [])
    result = TabularData.validate(tabular_data)

    assert isinstance(result, Invalid)
    assert "no data" in result.messages[0]


def test_event_data() -> None:
    """Test EventData creation and properties."""
    event_sets = [
        EventDataSet(
            datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc),
            DataChannelId.from_short_id("channel1"),
            "HIGH",
            "0",
        )
    ]

    event_data = EventData(event_sets)

    assert event_data.data_set == event_sets
    assert event_data.number_of_data_set == 1


def test_time_series_data() -> None:
    """Test TimeSeriesData creation."""
    config_ref = TimeSeriesConfigurationReference(
        "test-config.xml", datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc)
    )

    tabular_data = [
        TabularData(
            [DataChannelId.from_short_id("channel1")],
            [
                TabularDataSet(
                    datetime(2023, 1, 1, 12, 0, 0, tzinfo=timezone.utc),
                    ["100.0"],
                    ["0"],
                )
            ],
        )
    ]

    ts_data = TimeSeriesData(config_ref, tabular_data, None)

    assert ts_data.data_configuration == config_ref
    assert ts_data.tabular_data == tabular_data
    assert ts_data.event_data is None
    assert ts_data.custom_data_kinds is None


def test_package() -> None:
    """Test Package creation."""
    ship_id = ShipId("IMO1234567")
    header = Header(ship_id, None, "test-author")

    ts_data = [TimeSeriesData(None, [], None)]
    package = Package(header, ts_data)

    assert package.header == header
    assert package.time_series_data == ts_data


def test_time_series_data_package() -> None:
    """Test TimeSeriesDataPackage creation."""
    ship_id = ShipId("IMO1234567")
    header = Header(ship_id, None, "test-author")
    package = Package(header, [])

    ts_package = TimeSeriesDataPackage(package)

    assert ts_package.package == package
