"""Tests for the transport ISO19848 module."""

from vista_sdk.transport.iso19848 import (  # type: ignore
    ISO19848,
    DataChannelTypeNames,
    FormatDataTypes,
    ISO19848Version,
)
from vista_sdk.transport.iso19848_dtos import (  # type: ignore
    DataChannelTypeNameDto,
    DataChannelTypeNamesDto,
    FormatDataTypeDto,
    FormatDataTypesDto,
)


def test_iso19848_version_enum() -> None:
    """Test ISO19848Version enum."""
    assert ISO19848Version.V2018.value == "v2018"
    assert ISO19848Version.V2024.value == "v2024"
    assert ISO19848.LATEST_VERSION == ISO19848Version.V2024


def test_iso19848_singleton() -> None:
    """Test ISO19848 singleton pattern."""
    instance1 = ISO19848.get_instance()
    instance2 = ISO19848.get_instance()
    assert instance1 is instance2


def test_data_channel_type_names_dto() -> None:
    """Test DataChannelTypeNamesDto."""
    dto = DataChannelTypeNamesDto(
        values=[DataChannelTypeNameDto("test-type", "Test Description")]
    )
    assert len(dto.values) == 1
    assert dto.values[0].type == "test-type"
    assert dto.values[0].description == "Test Description"


def test_format_data_types_dto() -> None:
    """Test FormatDataTypesDto."""
    dto = FormatDataTypesDto(values=[FormatDataTypeDto("string", "String format")])
    assert len(dto.values) == 1
    assert dto.values[0].type == "string"
    assert dto.values[0].description == "String format"


def test_data_channel_type_names_parse_success() -> None:
    """Test successful parsing of data channel type names."""
    dto = DataChannelTypeNamesDto(
        values=[DataChannelTypeNameDto("temperature", "Temperature measurement")]
    )
    type_names = DataChannelTypeNames(dto)

    result = type_names.parse("temperature")
    assert isinstance(result, DataChannelTypeNames.ParseResult.Ok)
    ok_result = result  # type: DataChannelTypeNames.ParseResult.Ok
    type_name_dto = ok_result.type_name  # type: DataChannelTypeNameDto
    assert type_name_dto.type == "temperature"


def test_data_channel_type_names_parse_failure() -> None:
    """Test failed parsing of data channel type names."""
    dto = DataChannelTypeNamesDto(values=[])
    type_names = DataChannelTypeNames(dto)

    result = type_names.parse("unknown-type")
    assert isinstance(result, DataChannelTypeNames.ParseResult.Invalid)
    assert "Unknown data channel type" in result.message


def test_format_data_types_parse_success() -> None:
    """Test successful parsing of format data types."""
    dto = FormatDataTypesDto(
        values=[FormatDataTypeDto("decimal", "Decimal number format")]
    )
    format_types = FormatDataTypes(dto)

    result = format_types.parse("decimal")
    assert isinstance(result, FormatDataTypes.ParseResult.Ok)
    assert result.type_name.type == "decimal"


def test_format_data_types_parse_failure() -> None:
    """Test failed parsing of format data types."""
    dto = FormatDataTypesDto(values=[])
    format_types = FormatDataTypes(dto)

    result = format_types.parse("unknown-format")
    assert isinstance(result, FormatDataTypes.ParseResult.Invalid)
    assert "Unknown format data type" in result.message


def test_iso19848_get_data_channel_type_names() -> None:
    """Test getting data channel type names from ISO19848."""
    iso = ISO19848.get_instance()

    # Test both versions
    v2018_names = iso.get_data_channel_type_names(ISO19848Version.V2018)
    v2024_names = iso.get_data_channel_type_names(ISO19848Version.V2024)

    assert isinstance(v2018_names, DataChannelTypeNames)
    assert isinstance(v2024_names, DataChannelTypeNames)

    # Should return the same instance when called again (caching)
    v2018_names_2 = iso.get_data_channel_type_names(ISO19848Version.V2018)
    assert v2018_names is v2018_names_2


def test_iso19848_get_format_data_types() -> None:
    """Test getting format data types from ISO19848."""
    iso = ISO19848.get_instance()

    # Test both versions
    v2018_formats = iso.get_format_data_types(ISO19848Version.V2018)
    v2024_formats = iso.get_format_data_types(ISO19848Version.V2024)

    assert isinstance(v2018_formats, FormatDataTypes)
    assert isinstance(v2024_formats, FormatDataTypes)

    # Should return the same instance when called again (caching)
    v2018_formats_2 = iso.get_format_data_types(ISO19848Version.V2018)
    assert v2018_formats is v2018_formats_2
