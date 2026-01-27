"""Tests for the transport time series data module."""

from datetime import datetime, timezone

import pytest

from vista_sdk.imo_number import ImoNumber
from vista_sdk.local_id import LocalId  # type: ignore
from vista_sdk.result import Invalid, Ok  # type: ignore

# Import for fixtures
from vista_sdk.system_text_json.extensions import JsonExtensions
from vista_sdk.transport.data_channel import (
    ConfigurationReference,
    DataChannel,
    DataChannelList,
    DataChannelListPackage,
    DataChannelType,
    Format,
    NameObject,
    Property,
    Range,
    Restriction,
    Unit,
)
from vista_sdk.transport.data_channel import (
    DataChannelId as DCLDataChannelId,
)
from vista_sdk.transport.data_channel import (
    Header as DCLHeader,
)
from vista_sdk.transport.data_channel import (
    Package as DCLPackage,
)
from vista_sdk.transport.ship_id import ShipId  # type: ignore
from vista_sdk.transport.time_series_data.data_channel_id import (
    DataChannelId,  # type: ignore
)
from vista_sdk.transport.time_series_data.time_series_data import (  # type: ignore
    ConfigurationReference as TimeSeriesConfigurationReference,
)
from vista_sdk.transport.time_series_data.time_series_data import (
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

# Test fixtures matching C# IsoMessageTests.TimeSeries.cs


def create_test_data_channel_list_package() -> DataChannelListPackage:
    """Create a DataChannelListPackage for testing TimeSeriesData.

    Mirrors C#'s TestDataChannelListPackage (ValidFullyCustomDataChannelList).
    """
    dc_list = DataChannelList()

    # First data channel
    local_id1 = LocalId.parse(
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
    )
    dc_id1 = DCLDataChannelId(
        local_id=local_id1,
        short_id="0010",
        name_object=NameObject(
            naming_rule="Naming_Rule",
            custom_name_objects={"nr:CustomNameObject": "Vender specific NameObject"},
        ),
    )
    prop1 = Property(
        data_channel_type=DataChannelType("Inst", update_cycle=1.0),
        data_format=Format(
            "Decimal",
            restriction=Restriction(
                fraction_digits=1,
                max_inclusive=200.0,
                min_inclusive=-150.0,
            ),
        ),
        data_range=Range(0.0, 150.0),
        unit=Unit("°C", "Temperature"),
        quality_coding="OPC_QUALITY",
        alert_priority=None,
        name="M/E #1 Air Cooler CFW OUT Temp",
        remarks=" Location: ECR, Manufacturer: AAA Company, Type: TYPE-AAA ",
        custom_properties={"nr:CustomPropertyElement": "Vender specific Property"},
    )
    dc1 = DataChannel(dc_id1, prop1)
    dc_list.add(dc1)

    # Second data channel
    local_id2 = LocalId.parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power")
    dc_id2 = DCLDataChannelId(local_id=local_id2, short_id="0020")
    prop2 = Property(
        data_channel_type=DataChannelType("Alert"),
        data_format=Format(
            "String",
            restriction=Restriction(max_length=100, min_length=0),
        ),
        data_range=None,
        unit=None,
        alert_priority="Warning",
    )
    dc2 = DataChannel(dc_id2, prop2)
    dc_list.add(dc2)

    # Create header
    ship_id = ShipId(ImoNumber(1234567))
    config_ref = ConfigurationReference(
        config_id="DataChannelList.xml",
        version="1.0",
        timestamp=datetime(2016, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
    )
    header = DCLHeader(
        ship_id=ship_id,
        data_channel_list_id=config_ref,
        author="Author1",
        date_created=datetime(2015, 12, 1, 0, 0, 0, tzinfo=timezone.utc),
    )

    package = DCLPackage(header, dc_list)
    return DataChannelListPackage(package)


def create_test_time_series_data_package() -> TimeSeriesDataPackage:
    """Create a TimeSeriesDataPackage for testing.

    Mirrors C#'s TestTimeSeriesDataPackage.
    """
    dc_list_package = create_test_data_channel_list_package()

    # Get data channel IDs
    dc_ids = [
        DataChannelId.parse(
            dc.data_channel_id.short_id or str(dc.data_channel_id.local_id)
        )
        for dc in dc_list_package.data_channel_list
    ]

    # Create header
    ship_id = ShipId.parse("IMO1234567")
    time_span = TimeSpan(
        start=datetime(2016, 1, 1, 12, 0, 0, tzinfo=timezone.utc),
        end=datetime(2016, 1, 3, 12, 0, 0, tzinfo=timezone.utc),
    )
    system_config = [
        TimeSeriesConfigurationReference(
            "SystemConfiguration.xml",
            datetime(2016, 1, 3, 0, 0, 0, tzinfo=timezone.utc),
        ),
        TimeSeriesConfigurationReference(
            "SystemConfiguration.xml",
            datetime(2016, 1, 3, 0, 0, 0, tzinfo=timezone.utc),
        ),
    ]
    header = Header(
        ship_id=ship_id,
        time_span=time_span,
        date_created=datetime(2016, 1, 3, 12, 0, 0, tzinfo=timezone.utc),
        date_modified=datetime(2016, 1, 3, 12, 0, 0, tzinfo=timezone.utc),
        author="Shipboard data server",
        system_configuration=system_config,
    )

    # Create tabular data
    tabular_data_1 = TabularData(
        data_channel_ids=dc_ids,
        data_sets=[
            TabularDataSet(
                time_stamp=datetime(2016, 1, 1, 12, 0, 0, tzinfo=timezone.utc),
                value=[f"{100.0 * (i + 1)}" for i in range(len(dc_ids))],
                quality=["0"] * len(dc_ids),
            ),
            TabularDataSet(
                time_stamp=datetime(2016, 1, 2, 12, 0, 0, tzinfo=timezone.utc),
                value=[f"{105.0 * (i + 1)}" for i in range(len(dc_ids))],
                quality=["0"] * len(dc_ids),
            ),
        ],
    )

    tabular_data_2 = TabularData(
        data_channel_ids=[dc_ids[0]],
        data_sets=[
            TabularDataSet(
                time_stamp=datetime(2016, 1, 1, 12, 0, 0, tzinfo=timezone.utc),
                value=["100.0"],
                quality=None,
            ),
            TabularDataSet(
                time_stamp=datetime(2016, 1, 2, 12, 0, 0, tzinfo=timezone.utc),
                value=["100.1"],
                quality=None,
            ),
            TabularDataSet(
                time_stamp=datetime(2016, 1, 3, 12, 0, 0, tzinfo=timezone.utc),
                value=["100.2"],
                quality=None,
            ),
        ],
    )

    # Create event data
    event_data = EventData(
        data_set=[
            EventDataSet(
                time_stamp=datetime(2016, 1, 1, 12, 0, 1, tzinfo=timezone.utc),
                data_channel_id=dc_ids[1],
                value="HIGH",
                quality="0",
            ),
            EventDataSet(
                time_stamp=datetime(2016, 1, 1, 12, 0, 1, tzinfo=timezone.utc),
                data_channel_id=dc_ids[1],
                value="LOW",
                quality="0",
            ),
            EventDataSet(
                time_stamp=datetime(2016, 1, 1, 12, 0, 1, tzinfo=timezone.utc),
                data_channel_id=dc_ids[1],
                value="AVERAGE",
                quality="0",
            ),
        ]
    )

    # First TimeSeriesData
    data_config = TimeSeriesConfigurationReference(
        dc_list_package.package.header.data_channel_list_id.id,
        dc_list_package.package.header.data_channel_list_id.timestamp,
    )
    ts_data_1 = TimeSeriesData(
        data_configuration=data_config,
        tabular_data=[tabular_data_1, tabular_data_2],
        event_data=event_data,
    )

    # Second TimeSeriesData (same structure)
    ts_data_2 = TimeSeriesData(
        data_configuration=data_config,
        tabular_data=[tabular_data_1, tabular_data_2],
        event_data=event_data,
    )

    package = Package(header=header, time_series_data=[ts_data_1, ts_data_2])
    return TimeSeriesDataPackage(package)


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


def test_data_channel_list_time_series_configuration_reference() -> None:
    """Test DataChannelList to TimeSeriesConfigurationReference conversion."""
    dc = create_test_data_channel_list_package()

    data_channel_list_id = dc.package.header.data_channel_list_id
    ts_config_ref = data_channel_list_id.as_time_series_reference()
    assert ts_config_ref.id == data_channel_list_id.id
    assert ts_config_ref.time_stamp == data_channel_list_id.timestamp


# Tests matching C# IsoMessageTests.TimeSeries.cs


def test_time_series_data_valid() -> None:
    """Test TimeSeriesDataPackage creation.

    Mirrors C#'s Test_TimeSeriesData.
    """
    message = create_test_time_series_data_package()
    assert message is not None


def test_time_series_data_json() -> None:
    """Test TimeSeriesData JSON roundtrip.

    Mirrors C#'s Test_TimeSeriesData_Json.
    """
    message = create_test_time_series_data_package()

    # Convert to JSON DTO
    dto = JsonExtensions.TimeSeriesData.to_json_dto(message)

    # Convert back to domain model
    message2 = JsonExtensions.TimeSeriesData.to_domain_model(dto)
    assert message is not None
    assert message2 is not None
    assert message.package.header is not None
    assert message2.package.header is not None
    # Verify roundtrip - header
    assert message.package.header.ship_id == message2.package.header.ship_id
    assert message.package.header.author == message2.package.header.author
    assert message.package.header.time_span is not None
    assert message2.package.header.time_span is not None
    assert (
        message.package.header.time_span.start
        == message2.package.header.time_span.start
    )
    assert message.package.header.time_span.end == message2.package.header.time_span.end

    # Verify time series data count
    assert len(message.package.time_series_data) == len(
        message2.package.time_series_data
    )

    # Verify first time series data
    ts1 = message.package.time_series_data[0]
    ts2 = message2.package.time_series_data[0]

    assert ts1.data_configuration is not None
    assert ts2.data_configuration is not None
    assert ts1.data_configuration.id == ts2.data_configuration.id

    # Verify tabular data
    assert ts1.tabular_data is not None
    assert ts2.tabular_data is not None
    assert len(ts1.tabular_data) == len(ts2.tabular_data)

    # Verify event data
    assert ts1.event_data is not None
    assert ts2.event_data is not None
    assert len(ts1.event_data.data_set) == len(ts2.event_data.data_set)


def test_time_series_data_validation() -> None:
    """Test TimeSeriesData validation.

    Mirrors C#'s Test_TimeSeriesData_Validation.
    """
    dc_list = create_test_data_channel_list_package()
    message = create_test_time_series_data_package()

    # Validate each TimeSeriesData
    for ts_data in message.package.time_series_data:
        result = ts_data.validate(
            dc_list,
            on_tabular_data=lambda timestamp, dc, value, quality: Ok(),  # noqa: ARG005
            on_event_data=lambda timestamp, dc, value, quality: Ok(),  # noqa: ARG005
        )
        assert isinstance(result, Ok), f"Validation failed: {result}"
