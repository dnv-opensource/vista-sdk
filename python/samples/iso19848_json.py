"""ISO19848 JSON Serialization Examples.

This module demonstrates how to work with ISO19848 data structures,
including JSON serialization/deserialization, domain model conversion,
and data channel lookups.

Examples:
    - Loading DataChannelList from JSON
    - Loading TimeSeriesData from JSON
    - Converting between JSON DTOs and domain models
    - Looking up data channels by ShortId and LocalId
    - Validating TimeSeriesData against DataChannelList

For sensor data flow examples, see sensor_data_flow.py
"""

from datetime import datetime, timezone

from vista_sdk.imo_number import ImoNumber
from vista_sdk.local_id import LocalId
from vista_sdk.result import Invalid, Ok
from vista_sdk.system_text_json import JsonExtensions, Serializer
from vista_sdk.transport.data_channel import (
    ConfigurationReference,
    DataChannel,
    DataChannelId,
    DataChannelList,
    DataChannelListPackage,
    DataChannelType,
    Format,
    Header,
    Package,
    Property,
    Range,
    Restriction,
    Unit,
    VersionInformation,
)
from vista_sdk.transport.ship_id import ShipId
from vista_sdk.transport.time_series_data.time_series_data import (
    ConfigurationReference as TimeSeriesConfigurationReference,
)
from vista_sdk.transport.time_series_data.time_series_data import (
    DataChannelId as TimeSeriesDataChannelId,
)
from vista_sdk.transport.time_series_data.time_series_data import (
    EventData,
    EventDataSet,
    TabularData,
    TabularDataSet,
    TimeSeriesData,
    TimeSeriesDataPackage,
    TimeSpan,
)
from vista_sdk.transport.time_series_data.time_series_data import (
    Header as TimeSeriesHeader,
)
from vista_sdk.transport.time_series_data.time_series_data import (
    Package as TimeSeriesPackage,
)
from vista_sdk.transport.value import AnyValue

"""
Creation of sample data.

Skip to line 242 to see example functions.
"""


def create_sample_data_channel_list() -> DataChannelListPackage:
    """Create a sample DataChannelListPackage for demonstration.

    Returns:
        A fully populated DataChannelListPackage with multiple data channels.
    """
    dc_list = DataChannelList()

    # Data channel 1: Temperature sensor
    local_id1 = LocalId.parse(
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
    )
    dc_id1 = DataChannelId(
        local_id=local_id1,
        short_id="TEMP001",
        name_object=None,
    )
    prop1 = Property(
        data_channel_type=DataChannelType("Inst", update_cycle=1.0),
        data_format=Format(
            "Decimal",
            restriction=Restriction(
                fraction_digits=1,
                max_inclusive=200.0,
                min_inclusive=-50.0,
            ),
        ),
        data_range=Range(0.0, 150.0),
        unit=Unit("°C", "Temperature"),
        quality_coding="OPC_QUALITY",
        name="Main Engine Air Cooler Temperature",
    )
    dc_list.add(DataChannel(dc_id1, prop1))

    # Data channel 2: Power measurement (same update_cycle as TEMP001 for grouping)
    local_id2 = LocalId.parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power")
    dc_id2 = DataChannelId(local_id=local_id2, short_id="PWR001")
    prop2 = Property(
        data_channel_type=DataChannelType("Inst", update_cycle=1.0),
        data_format=Format("Decimal"),
        data_range=Range(0.0, 10000.0),
        unit=Unit("kW", "Power"),
        name="Generator Power Output",
    )
    dc_list.add(DataChannel(dc_id2, prop2))

    # Data channel 3: Alert channel (string type, no range/unit needed)
    local_id3 = LocalId.parse(
        "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop"
    )
    dc_id3 = DataChannelId(local_id=local_id3, short_id="ALT001")
    prop3 = Property(
        data_channel_type=DataChannelType("Alert"),
        data_format=Format("String", restriction=Restriction(max_length=100)),
        data_range=None,
        unit=None,
        alert_priority="Warning",
        name="Diesel Oil Low Level Alert",
    )
    dc_list.add(DataChannel(dc_id3, prop3))

    # Create header
    ship_id = ShipId(ImoNumber(1234567))
    config_ref = ConfigurationReference(
        config_id="DataChannelList-v1",
        version="1.0",
        timestamp=datetime(2024, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
    )
    version_info = VersionInformation(
        naming_rule="dnv",
        naming_scheme_version="v2",
        reference_url="https://docs.vista.dnv.com",
    )
    header = Header(
        ship_id=ship_id,
        data_channel_list_id=config_ref,
        version_information=version_info,
        author="Vista SDK Sample",
        date_created=datetime(2024, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
    )

    package = Package(header, dc_list)
    return DataChannelListPackage(package)


def create_sample_time_series_data(
    dc_list_package: DataChannelListPackage,
) -> TimeSeriesDataPackage:
    """Create sample TimeSeriesData for validation demonstration.

    Args:
        dc_list_package: The DataChannelList to reference.

    Returns:
        A TimeSeriesDataPackage with tabular and event data.
    """
    # Get data channel IDs from the list
    dc_ids = [
        TimeSeriesDataChannelId.parse(
            dc.data_channel_id.short_id or str(dc.data_channel_id.local_id)
        )
        for dc in dc_list_package.data_channel_list
    ]

    # Create header
    ship_id = ShipId.parse("IMO1234567")
    time_span = TimeSpan(
        start=datetime(2024, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
        end=datetime(2024, 1, 1, 1, 0, 0, tzinfo=timezone.utc),
    )
    header = TimeSeriesHeader(
        ship_id=ship_id,
        time_span=time_span,
        author="Vista SDK Sample",
        date_created=datetime(2024, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
    )

    # Create tabular data with measurements
    tabular_data = TabularData(
        data_channel_ids=dc_ids[:2],  # First two channels (numeric)
        data_sets=[
            TabularDataSet(
                time_stamp=datetime(2024, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
                value=["25.5", "1500.0"],
                quality=["0", "0"],
            ),
            TabularDataSet(
                time_stamp=datetime(2024, 1, 1, 0, 30, 0, tzinfo=timezone.utc),
                value=["26.1", "1520.0"],
                quality=["0", "0"],
            ),
            TabularDataSet(
                time_stamp=datetime(2024, 1, 1, 1, 0, 0, tzinfo=timezone.utc),
                value=["25.8", "1480.0"],
                quality=["0", "0"],
            ),
        ],
    )

    # Create event data with alerts
    event_data = EventData(
        data_set=[
            EventDataSet(
                time_stamp=datetime(2024, 1, 1, 0, 15, 0, tzinfo=timezone.utc),
                data_channel_id=dc_ids[2],  # Alert channel
                value="LOW_LEVEL_WARNING",
                quality="0",
            ),
            EventDataSet(
                time_stamp=datetime(2024, 1, 1, 0, 45, 0, tzinfo=timezone.utc),
                data_channel_id=dc_ids[2],
                value="NORMAL",
                quality="0",
            ),
        ]
    )

    # Create TimeSeriesData with reference to DataChannelList
    data_config = TimeSeriesConfigurationReference(
        dc_list_package.package.header.data_channel_list_id.id,
        dc_list_package.package.header.data_channel_list_id.timestamp,
    )
    ts_data = TimeSeriesData(
        data_configuration=data_config,
        tabular_data=[tabular_data],
        event_data=event_data,
    )

    package = TimeSeriesPackage(header=header, time_series_data=[ts_data])
    return TimeSeriesDataPackage(package)


def example_json_to_domain() -> None:
    """Demonstrate JSON to domain model conversion.

    Shows how to:
    1. Create a DataChannelListPackage
    2. Convert to JSON DTO
    3. Serialize to JSON string
    4. Deserialize back to JSON DTO
    5. Convert to domain model
    """
    print("=" * 60)
    print("Example: JSON to Domain Model Conversion")
    print("=" * 60)

    # Create sample data
    original = create_sample_data_channel_list()
    channel_count = len(original.data_channel_list)
    print(f"\n1. Created DataChannelListPackage with {channel_count} channels")

    # Convert domain model to JSON DTO
    json_dto = JsonExtensions.DataChannelList.to_json_dto(original)
    print("2. Converted to JSON DTO")

    # Serialize to JSON string
    json_str = Serializer.serialize(json_dto)
    print(f"3. Serialized to JSON ({len(json_str)} bytes)")
    print(f"   Preview: {json_str[:100]}...")

    # Deserialize JSON string to DTO
    loaded_dto = Serializer.deserialize_data_channel_list(json_str)
    print("4. Deserialized JSON string to DTO")

    # Convert DTO back to domain model
    domain_model = JsonExtensions.DataChannelList.to_domain_model(loaded_dto)
    dc_count = len(domain_model.data_channel_list)
    print(f"5. Converted to domain model with {dc_count} channels")

    # Verify roundtrip
    if json_dto != loaded_dto:
        raise ValueError("JSON DTO roundtrip should be equal")
    print("\n✓ JSON roundtrip successful!")


def example_get_by_short_id() -> None:
    """Demonstrate looking up data channels by ShortId.

    Shows how to use DataChannelList.try_get_by_short_id() to find
    data channels using their short identifier.
    """
    print("\n" + "=" * 60)
    print("Example: Get Data Channel by ShortId")
    print("=" * 60)

    # Create sample data
    package = create_sample_data_channel_list()
    dc_list = package.package.data_channel_list

    # Look up by ShortId - typically your system ids
    short_ids_to_find = ["TEMP001", "PWR001", "ALT001", "UNKNOWN"]

    for short_id in short_ids_to_find:
        found, data_channel = dc_list.try_get_by_short_id(short_id)

        if found and data_channel:
            print(f"\n✓ Found ShortId '{short_id}':")
            print(f"  Name: {data_channel.property_.name}")
            print(f"  LocalId: {data_channel.data_channel_id.local_id}")
            print(f"  Type: {data_channel.property_.data_channel_type.type}")
            if data_channel.property_.unit:
                print(f"  Unit: {data_channel.property_.unit.unit_symbol}")
        else:
            print(f"\n✗ ShortId '{short_id}' not found")


def example_get_by_local_id() -> None:
    """Demonstrate looking up data channels by LocalId.

    Shows how to use DataChannelList.try_get_by_local_id() to find
    data channels using their LocalId.
    """
    print("\n" + "=" * 60)
    print("Example: Get Data Channel by LocalId")
    print("=" * 60)

    # Create sample data
    package = create_sample_data_channel_list()
    dc_list = package.package.data_channel_list

    # Look up by LocalId
    local_id_strs = [
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air",
        "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power",
        "/dnv-v2/vis-3-4a/999.99/meta/unknown",  # Non-existent
    ]

    for local_id_str in local_id_strs:
        try:
            local_id = LocalId.parse(local_id_str)
            found, data_channel = dc_list.try_get_by_local_id(local_id)

            if found and data_channel:
                print("\n✓ Found LocalId:")
                print(f"  LocalId: {local_id}")
                print(f"  ShortId: {data_channel.data_channel_id.short_id}")
                print(f"  Name: {data_channel.property_.name}")
            else:
                print(f"\n✗ LocalId not found: {local_id_str}")
        except Exception as e:
            print(f"\n✗ Invalid LocalId '{local_id_str}': {e}")


def example_validate_time_series() -> None:
    """Demonstrate TimeSeriesData validation against DataChannelList.

    Shows how to validate time series data by checking that all
    referenced data channels exist and values are properly formatted.
    """
    print("\n" + "=" * 60)
    print("Example: Validate TimeSeriesData")
    print("=" * 60)

    # Create sample data
    dc_list_package = create_sample_data_channel_list()
    ts_package = create_sample_time_series_data(dc_list_package)

    dc_count = len(dc_list_package.data_channel_list)
    ts_count = len(ts_package.package.time_series_data)
    print(f"\nDataChannelList: {dc_count} channels")
    print(f"TimeSeriesData: {ts_count} time series")

    # Validation counters
    tabular_count = 0
    event_count = 0

    def on_tabular_data(
        _timestamp: datetime,
        _data_channel: DataChannel,
        _value: AnyValue,
        _quality: str | None,
    ) -> Ok | Invalid:
        """Callback for each tabular data point."""
        nonlocal tabular_count
        tabular_count += 1
        # Could add custom validation here
        return Ok()

    def on_event_data(
        _timestamp: datetime,
        _data_channel: DataChannel,
        _value: AnyValue,
        _quality: str | None,
    ) -> Ok | Invalid:
        """Callback for each event data point."""
        nonlocal event_count
        event_count += 1
        # Could add custom validation here
        return Ok()

    # Validate each TimeSeriesData
    for i, ts_data in enumerate(ts_package.package.time_series_data):
        print(f"\nValidating TimeSeriesData #{i + 1}...")

        result = ts_data.validate(
            dc_list_package,
            on_tabular_data=on_tabular_data,
            on_event_data=on_event_data,
        )

        if isinstance(result, Ok):
            print("  ✓ Validation successful!")
            print(f"    Tabular data points: {tabular_count}")
            print(f"    Event data points: {event_count}")
        elif isinstance(result, Invalid):
            print(f"  ✗ Validation failed: {result.messages}")


def example_time_series_json_roundtrip() -> None:
    """Demonstrate TimeSeriesData JSON serialization roundtrip.

    Shows how to serialize and deserialize TimeSeriesData while
    preserving all data.
    """
    print("\n" + "=" * 60)
    print("Example: TimeSeriesData JSON Roundtrip")
    print("=" * 60)

    # Create sample data
    dc_list_package = create_sample_data_channel_list()
    original = create_sample_time_series_data(dc_list_package)

    print("\n1. Created TimeSeriesDataPackage")
    if original.package.header:
        print(f"   Ship: {original.package.header.ship_id}")
        if original.package.header.time_span:
            ts = original.package.header.time_span
            print(f"   TimeSpan: {ts.start} to {ts.end}")

    # Convert to JSON DTO and serialize
    json_dto = JsonExtensions.TimeSeriesData.to_json_dto(original)
    json_str = Serializer.serialize(json_dto)
    print(f"2. Serialized to JSON ({len(json_str)} bytes)")

    # Deserialize and convert back
    loaded_dto = Serializer.deserialize_time_series_data(json_str)
    domain_model = JsonExtensions.TimeSeriesData.to_domain_model(loaded_dto)

    print("3. Deserialized and converted to domain model")

    # Verify structure
    if json_dto != loaded_dto:
        raise ValueError("JSON DTO roundtrip should be equal")
    orig_len = len(original.package.time_series_data)
    loaded_len = len(domain_model.package.time_series_data)
    if orig_len != loaded_len:
        raise ValueError(f"TimeSeriesData count mismatch: {orig_len} vs {loaded_len}")

    print("\n✓ JSON roundtrip successful!")


def example_time_series_json_to_domain() -> None:
    """Demonstrate TimeSeriesData JSON to domain model conversion.

    Shows how to:
    1. Start with a JSON string (e.g., from a file or API)
    2. Deserialize to JSON DTO
    3. Convert to domain model for processing
    4. Access the structured data
    """
    print("\n" + "=" * 60)
    print("Example: TimeSeriesData JSON to Domain Model")
    print("=" * 60)

    # Simulate loading from JSON (create sample and serialize first)
    dc_list_package = create_sample_data_channel_list()
    original = create_sample_time_series_data(dc_list_package)
    json_dto = JsonExtensions.TimeSeriesData.to_json_dto(original)
    json_str = Serializer.serialize(json_dto)

    print(f"\n1. Received JSON payload ({len(json_str)} bytes)")
    print(f"   Preview: {json_str[:80]}...")

    # Step 1: Deserialize JSON string to DTO
    dto = Serializer.deserialize_time_series_data(json_str)
    print("2. Deserialized to JSON DTO")

    # Step 2: Convert to domain model
    domain = JsonExtensions.TimeSeriesData.to_domain_model(dto)
    print("3. Converted to domain model")

    # Step 3: Access structured data
    print("\n4. Accessing domain model data:")
    if domain.package.header:
        h = domain.package.header
        print(f"   Ship ID: {h.ship_id}")
        print(f"   Author: {h.author}")
        if h.time_span:
            print(f"   Time Span: {h.time_span.start} to {h.time_span.end}")

    for i, ts_data in enumerate(domain.package.time_series_data):
        print(f"\n   TimeSeriesData #{i + 1}:")
        if ts_data.tabular_data:
            for j, tab in enumerate(ts_data.tabular_data):
                if tab.data_channel_ids:
                    ch_count = len(tab.data_channel_ids)
                    print(f"     Tabular #{j + 1}: {ch_count} channels")
                if tab.data_sets:
                    print(f"       {len(tab.data_sets)} data sets")
        if ts_data.event_data and ts_data.event_data.data_set:
            print(f"     Events: {len(ts_data.event_data.data_set)} events")

    print("\n✓ JSON to domain conversion successful!")


def main() -> None:
    """Run all ISO19848 examples."""
    print("\n" + "=" * 60)
    print("ISO19848 JSON Serialization Examples")
    print("=" * 60)

    # DataChannelList examples
    example_json_to_domain()
    example_get_by_short_id()
    example_get_by_local_id()

    # TimeSeriesData examples
    example_time_series_json_to_domain()
    example_time_series_json_roundtrip()
    example_validate_time_series()

    print("\n" + "=" * 60)
    print("All examples completed successfully!")
    print("=" * 60)


if __name__ == "__main__":
    main()
