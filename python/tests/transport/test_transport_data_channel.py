"""Tests for the transport data channel module."""

from datetime import datetime, timezone

import pytest

from vista_sdk.imo_number import ImoNumber
from vista_sdk.local_id import LocalId
from vista_sdk.result import Invalid, Ok
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
    WhiteSpace,
)
from vista_sdk.transport.ship_id import ShipId


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
    assert dc.prop == prop


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
    assert WhiteSpace.PRESERVE.value == 0
    assert WhiteSpace.REPLACE.value == 1
    assert WhiteSpace.COLLAPSE.value == 2


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
