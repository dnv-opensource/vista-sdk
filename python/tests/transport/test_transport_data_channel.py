"""Tests for the transport data channel module."""

from datetime import datetime, timezone

import pytest

from vista_sdk.imo_number import ImoNumber
from vista_sdk.local_id import LocalId
from vista_sdk.result import Invalid, Ok
from vista_sdk.system_text_json.extensions import JsonExtensions
from vista_sdk.transport.data_channel import (
    ConfigurationReference,
    DataChannel,
    DataChannelId,
    DataChannelList,
    DataChannelListPackage,
    DataChannelType,
    Format,
    Header,
    NameObject,
    Package,
    Property,
    Range,
    Restriction,
    Unit,
    VersionInformation,
    WhiteSpace,
)
from vista_sdk.transport.ship_id import ShipId

# Test fixtures matching C# IsoMessageTests.DataChannel.cs


def create_valid_fully_custom_data_channel_list() -> DataChannelListPackage:
    """Create a fully custom DataChannelListPackage for testing.

    Mirrors C#'s ValidFullyCustomDataChannelList.
    """
    dc_list = DataChannelList()

    # First data channel with all custom properties
    local_id1 = LocalId.parse(
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
    )
    dc_id1 = DataChannelId(
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

    # Second data channel (simpler)
    local_id2 = LocalId.parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power")
    dc_id2 = DataChannelId(local_id=local_id2, short_id="0020")
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
    version_info = VersionInformation(
        naming_rule="some_naming_rule",
        naming_scheme_version="2.0",
        reference_url="http://somewhere.net",
    )
    header = Header(
        ship_id=ship_id,
        data_channel_list_id=config_ref,
        version_information=version_info,
        author="Author1",
        date_created=datetime(2015, 12, 1, 0, 0, 0, tzinfo=timezone.utc),
        custom_headers={"nr:CustomHeaderElement": "Vender specific headers"},
    )

    package = Package(header, dc_list)
    return DataChannelListPackage(package)


def create_valid_data_channel_list() -> DataChannelListPackage:
    """Create a simple valid DataChannelListPackage for testing.

    Mirrors C#'s ValidDataChannelList.
    """
    dc_list = DataChannelList()

    local_id = LocalId.parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power")
    dc_id = DataChannelId(local_id=local_id, short_id="0010")
    prop = Property(
        data_channel_type=DataChannelType("Inst"),
        data_format=Format("String", restriction=None),
        data_range=None,
        unit=None,
        alert_priority=None,
    )
    dc = DataChannel(dc_id, prop)
    dc_list.add(dc)

    ship_id = ShipId(ImoNumber(1234567))
    config_ref = ConfigurationReference(
        config_id="some-id",
        timestamp=datetime(2016, 1, 1, 0, 0, 0, tzinfo=timezone.utc),
    )
    header = Header(
        ship_id=ship_id,
        data_channel_list_id=config_ref,
        author="some-author",
    )

    package = Package(header, dc_list)
    return DataChannelListPackage(package)


def test_version_information() -> None:
    """Test VersionInformation creation."""
    version_info = VersionInformation()
    assert version_info.naming_rule == "dnv"
    assert version_info.naming_scheme_version == "v2"
    assert version_info.reference_url == "https://docs.vista.dnv.com"

    # Test with custom values
    custom_version = VersionInformation(
        naming_rule="custom",
        naming_scheme_version="v3",
        reference_url="https://example.com",
    )
    assert custom_version.naming_rule == "custom"
    assert custom_version.naming_scheme_version == "v3"
    assert custom_version.reference_url == "https://example.com"


def test_data_channel_type() -> None:
    """Test DataChannelType creation and validation."""
    # Test basic creation
    dc_type = DataChannelType("Inst")
    assert dc_type.type == "Inst"
    assert not dc_type.is_alert

    # Test with update cycle and calculation period
    dc_type = DataChannelType("Alert", update_cycle=1.0, calculation_period=2.0)
    assert dc_type.type == "Alert"
    assert dc_type.update_cycle == 1.0
    assert dc_type.calculation_period == 2.0
    assert dc_type.is_alert

    # Test validation errors
    with pytest.raises(ValueError, match="Should be positive"):
        DataChannelType("Inst", update_cycle=-1.0)

    with pytest.raises(ValueError, match="Should be positive"):
        DataChannelType("Inst", calculation_period=-1.0)


def test_format() -> None:
    """Test Format creation and validation."""
    # Test basic format
    fmt = Format("String")
    assert fmt.type == "String"
    assert not fmt.is_decimal

    # Test decimal format
    decimal_fmt = Format("Decimal")
    assert decimal_fmt.type == "Decimal"
    assert decimal_fmt.is_decimal

    # Test format validation
    result, value = decimal_fmt.validate_value("123.45")
    assert isinstance(result, Ok)
    assert value is not None


def test_restriction() -> None:
    """Test Restriction validation."""
    # Test enumeration restriction
    restriction = Restriction(enumeration=["red", "green", "blue"])
    result = restriction.validate_value("red", Format("String"))
    assert isinstance(result, Ok)

    result = restriction.validate_value("yellow", Format("String"))
    assert isinstance(result, Invalid)

    # Test length restrictions
    restriction = Restriction(min_length=2, max_length=10)
    result = restriction.validate_value("abc", Format("String"))
    assert isinstance(result, Ok)

    result = restriction.validate_value("a", Format("String"))  # Too short
    assert isinstance(result, Invalid)


def test_range() -> None:
    """Test Range creation and validation."""
    # Test valid range
    range_obj = Range(0.0, 100.0)
    assert range_obj.low == 0.0
    assert range_obj.high == 100.0

    # Test validation errors
    with pytest.raises(ValueError, match="should be less than"):
        Range(100.0, 50.0)  # low > high


def test_unit() -> None:
    """Test Unit creation."""
    unit = Unit("°C", "Temperature")
    assert unit.unit_symbol == "°C"
    assert unit.quantity_name == "Temperature"


def test_property_validation() -> None:
    """Test Property validation logic."""
    # Valid property for non-decimal format
    prop = Property(
        data_channel_type=DataChannelType("Inst"),
        data_format=Format("String"),
        data_range=None,
        unit=None,
    )
    result = prop.validate()
    assert isinstance(result, Ok)

    # Invalid property - decimal format without range
    prop = Property(
        data_channel_type=DataChannelType("Inst"),
        data_format=Format("Decimal"),
        data_range=None,
        unit=None,
    )
    result = prop.validate()
    assert isinstance(result, Invalid)
    assert "Range is required for Decimal format type" in result.messages

    # Invalid property - alert type without priority
    prop = Property(
        data_channel_type=DataChannelType("Alert"),
        data_format=Format("String"),
        data_range=None,
        unit=None,
    )
    result = prop.validate()
    assert isinstance(result, Invalid)
    assert "AlertPriority is required for Alert DataChannelType" in result.messages


def test_data_channel() -> None:
    """Test DataChannel creation."""
    # Use a valid LocalId from the test data
    local_id = LocalId.parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power")
    dc_id = DataChannelId(local_id, "0010")

    prop = Property(
        data_channel_type=DataChannelType("Inst"),
        data_format=Format("String"),
        data_range=None,
        unit=None,
    )

    dc = DataChannel(dc_id, prop)
    assert dc.data_channel_id == dc_id
    assert dc.property_ == prop


def test_data_channel_list() -> None:
    """Test DataChannelList operations."""
    dc_list = DataChannelList()
    assert len(dc_list) == 0

    # Create a data channel with a valid LocalId
    local_id = LocalId.parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power")
    dc_id = DataChannelId(local_id, "0010")
    prop = Property(
        data_channel_type=DataChannelType("Inst"),
        data_format=Format("String"),
        data_range=None,
        unit=None,
    )
    dc = DataChannel(dc_id, prop)

    # Add data channel
    dc_list.add(dc)
    assert len(dc_list) == 1

    # Test lookups
    success, found_dc = dc_list.try_get_by_short_id("0010")
    assert success
    assert found_dc == dc

    success, found_dc = dc_list.try_get_by_local_id(local_id)
    assert success
    assert found_dc == dc

    # Test indexing
    assert dc_list["0010"] == dc
    assert dc_list[local_id] == dc
    assert dc_list[0] == dc

    # Test iteration
    channels = list(dc_list)
    assert len(channels) == 1
    assert channels[0] == dc


def test_white_space_enum() -> None:
    """Test WhiteSpace enumeration."""
    assert WhiteSpace.Preserve.value == 0
    assert WhiteSpace.Replace.value == 1
    assert WhiteSpace.Collapse.value == 2


def test_data_channel_list_package() -> None:
    """Test complete DataChannelListPackage structure."""
    # Create version info
    version_info = VersionInformation()

    # Create header using the proper constructor
    ship_id = ShipId(ImoNumber(1234567))
    config_ref = ConfigurationReference(
        config_id="test-list", timestamp=datetime.now(timezone.utc)
    )
    header = Header(
        ship_id=ship_id,
        data_channel_list_id=config_ref,
        author="Test Author",
        version_information=version_info,
    )

    # Create data channel list
    dc_list = DataChannelList()

    # Create package
    package = Package(header, dc_list)

    # Create package wrapper
    list_package = DataChannelListPackage(package)

    assert list_package.package == package
    assert list_package.data_channel_list == dc_list


# Tests matching C# IsoMessageTests.DataChannel.cs


def test_data_channel_list_valid() -> None:
    """Test ValidDataChannelList creation.

    Mirrors C#'s Test_DataChannelList.
    """
    message = create_valid_data_channel_list()
    assert message is not None


def test_local_id_lookup() -> None:
    """Test looking up data channel by LocalId.

    Mirrors C#'s Test_LocalId_Lookup.
    """
    message = create_valid_data_channel_list()
    data_channel = message.package.data_channel_list[0]
    local_id = data_channel.data_channel_id.local_id

    # Lookup by LocalId using indexer
    lookup = message.package.data_channel_list[local_id]

    # Lookup using try_get method
    success, lookup2 = message.package.data_channel_list.try_get_by_local_id(local_id)

    assert success
    assert data_channel == lookup
    assert data_channel == lookup2


def test_short_id_lookup() -> None:
    """Test looking up data channel by ShortId.

    Mirrors C#'s Test_ShortId_Lookup.
    """
    message = create_valid_data_channel_list()
    data_channel = message.package.data_channel_list[0]
    short_id = data_channel.data_channel_id.short_id

    assert short_id is not None

    # Lookup by ShortId using indexer
    lookup = message.package.data_channel_list[short_id]

    # Lookup using try_get method
    success, lookup2 = message.package.data_channel_list.try_get_by_short_id(short_id)

    assert success
    assert data_channel == lookup
    assert data_channel == lookup2


def test_data_channel_list_enumerator() -> None:
    """Test DataChannelList enumeration.

    Mirrors C#'s Test_DataChannelList_Enumerator.
    """
    message = create_valid_data_channel_list()

    expected_length = len(message.data_channel_list.data_channels)
    actual_length = len(message.data_channel_list)
    counter = 0

    assert expected_length == actual_length

    for dc in message.data_channel_list:
        assert dc is not None
        counter += 1

    assert expected_length == counter


def test_data_channel_list_json() -> None:
    """Test DataChannelList JSON roundtrip.

    Mirrors C#'s Test_DataChannelList_Json.
    """
    message = create_valid_data_channel_list()

    # Convert to JSON DTO
    dto = JsonExtensions.DataChannelList.to_json_dto(message)

    # Convert back to domain model
    message2 = JsonExtensions.DataChannelList.to_domain_model(dto)

    # Verify roundtrip
    assert message.package.header.ship_id == message2.package.header.ship_id
    assert (
        message.package.header.data_channel_list_id.id
        == message2.package.header.data_channel_list_id.id
    )
    assert message.package.header.author == message2.package.header.author
    assert len(message.data_channel_list) == len(message2.data_channel_list)

    # Compare data channels
    for i in range(len(message.data_channel_list)):
        dc1 = message.data_channel_list[i]
        dc2 = message2.data_channel_list[i]
        assert dc1.data_channel_id.local_id == dc2.data_channel_id.local_id
        assert dc1.data_channel_id.short_id == dc2.data_channel_id.short_id
        assert (
            dc1.property_.data_channel_type.type == dc2.property_.data_channel_type.type
        )
        assert dc1.property_.format.type == dc2.property_.format.type


def test_data_channel_list_json_fully_custom() -> None:
    """Test fully custom DataChannelList JSON roundtrip."""
    message = create_valid_fully_custom_data_channel_list()

    # Convert to JSON DTO
    dto = JsonExtensions.DataChannelList.to_json_dto(message)

    # Convert back to domain model
    message2 = JsonExtensions.DataChannelList.to_domain_model(dto)

    # Verify roundtrip
    assert message.package.header.ship_id == message2.package.header.ship_id
    assert message.package.header.author == message2.package.header.author
    assert (
        message.package.header.version_information.naming_rule
        == message2.package.header.version_information.naming_rule
    )
    assert (
        message.package.header.custom_headers == message2.package.header.custom_headers
    )

    assert len(message.data_channel_list) == len(message2.data_channel_list)

    # Compare first data channel (with all custom properties)
    dc1 = message.data_channel_list[0]
    dc2 = message2.data_channel_list[0]
    assert dc1.data_channel_id.local_id == dc2.data_channel_id.local_id
    assert dc1.data_channel_id.short_id == dc2.data_channel_id.short_id
    assert (
        dc1.data_channel_id.name_object.naming_rule
        == dc2.data_channel_id.name_object.naming_rule
    )
    assert dc1.property_.name == dc2.property_.name
    assert dc1.property_.remarks == dc2.property_.remarks
    assert dc1.property_.custom_properties == dc2.property_.custom_properties
    assert dc1.property_.range is not None
    assert dc2.property_.range is not None
    assert dc1.property_.range.low == dc2.property_.range.low
    assert dc1.property_.range.high == dc2.property_.range.high
    assert dc1.property_.unit is not None
    assert dc2.property_.unit is not None
    assert dc1.property_.unit.unit_symbol == dc2.property_.unit.unit_symbol
