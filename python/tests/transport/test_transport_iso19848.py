"""Tests for the transport ISO19848 module."""

import pytest

from vista_sdk.transport.iso19848 import (
    ISO19848,
    DataChannelTypeNames,
    FormatDataTypes,
    ISO19848Version,
)


def test_iso19848_version_enum() -> None:
    """Test ISO19848Version enum."""
    assert ISO19848Version.V2018.value == "v2018"
    assert ISO19848Version.V2024.value == "v2024"
    assert ISO19848.LATEST_VERSION == ISO19848Version.V2024


def test_iso19848_singleton() -> None:
    """Test ISO19848 singleton pattern.

    Mirrors C#'s Test_Instance.
    """
    instance1 = ISO19848()
    instance2 = ISO19848()
    assert instance1 is instance2
    assert instance1 is not None


@pytest.mark.parametrize("version", list(ISO19848Version))
def test_data_channel_type_names_load(version: ISO19848Version) -> None:
    """Test DataChannelTypeNames load for all versions.

    Mirrors C#'s Test_DataChannelTypeNames_Load.
    """
    iso = ISO19848()
    data_channel_type_names = iso.get_data_channel_type_names(version)
    assert data_channel_type_names is not None


def test_data_channel_type_names_parse_self() -> None:
    """Test parsing data channel type names parses all its own values.

    Mirrors C#'s Test_DataChannelTypeNames_Parse_Self.
    """
    iso = ISO19848()
    data_channel_type_names = iso.get_data_channel_type_names(ISO19848Version.V2024)
    assert data_channel_type_names is not None

    for type_name in data_channel_type_names:
        result = data_channel_type_names.parse(type_name.type)
        assert isinstance(result, DataChannelTypeNames.ParseResult.Ok)
        assert result.type_name.type == type_name.type


@pytest.mark.parametrize(
    ("value", "expected_result"),
    [
        ("Inst", True),
        ("Average", True),
        ("Max", True),
        ("Min", True),
        ("Median", True),
        ("Mode", True),
        ("StandardDeviation", True),
        ("Calculated", True),
        ("SetPoint", True),
        ("Command", True),
        ("Alert", True),
        ("Status", True),
        ("ManualInput", True),
        ("manualInput", False),  # Case sensitive
        ("asd", False),
        ("some-random", False),
        ("InputManual", False),
    ],
)
def test_data_channel_type_names_parse(value: str, expected_result: bool) -> None:
    """Test DataChannelTypeNames parsing with various inputs.

    Mirrors C#'s Test_DataChannelTypeNames_Parse.
    """
    iso = ISO19848()
    data_channel_type_names = iso.get_data_channel_type_names(ISO19848Version.V2024)
    assert data_channel_type_names is not None

    result = data_channel_type_names.parse(value)

    if expected_result:
        assert isinstance(result, DataChannelTypeNames.ParseResult.Ok)
        assert result.type_name.type == value
    else:
        assert isinstance(result, DataChannelTypeNames.ParseResult.Invalid)


@pytest.mark.parametrize("version", list(ISO19848Version))
def test_format_data_types_load(version: ISO19848Version) -> None:
    """Test FormatDataTypes load for all versions.

    Mirrors C#'s Test_FormatDataTypes_Load.
    """
    iso = ISO19848()
    format_types = iso.get_format_data_types(version)
    assert format_types is not None


def test_format_data_type_parse_self() -> None:
    """Test parsing format data types parses all its own values.

    Mirrors C#'s Test_FormatDataType_Parse_Self.
    """
    iso = ISO19848()
    format_types = iso.get_format_data_types(ISO19848Version.V2024)
    assert format_types is not None

    for type_name in format_types:
        result = format_types.parse(type_name.type)
        assert isinstance(result, FormatDataTypes.ParseResult.Ok)
        assert result.type_name.type == type_name.type
        failure = format_types.parse("Non_Existent-Type")
        assert isinstance(failure, FormatDataTypes.ParseResult.Invalid)


@pytest.mark.parametrize(
    ("value", "expected_result"),
    [
        ("Decimal", True),
        ("Integer", True),
        ("Boolean", True),
        ("String", True),
        ("DateTime", True),
        ("decimal", False),  # Case sensitive
        ("string", False),
        ("asd", False),
        ("some-random", False),
        ("TimeDate", False),
    ],
)
def test_format_data_type_parse(value: str, expected_result: bool) -> None:
    """Test FormatDataType parsing with various inputs.

    Mirrors C#'s Test_FormatDataType_Parse.
    """
    iso = ISO19848()
    format_types = iso.get_format_data_types(ISO19848Version.V2024)
    assert format_types is not None

    result = format_types.parse(value)

    if expected_result:
        assert isinstance(result, FormatDataTypes.ParseResult.Ok)
        assert result.type_name.type == value
    else:
        assert isinstance(result, FormatDataTypes.ParseResult.Invalid)
