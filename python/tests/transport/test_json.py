"""Tests for JSON transport serialization and conversion in vista_sdk.

This module covers:
- DataChannelList and TimeSeriesData schema validation
- JSON roundtrip serialization
- ISO 8601 datetime parsing
"""

from datetime import datetime
from pathlib import Path

import pytest

from vista_sdk.system_text_json import (
    DataChannelListPackage,
    Serializer,
    TimeSeriesDataPackage,
)
from vista_sdk.system_text_json.extensions import JsonExtensions
from vista_sdk.system_text_json.serializer import _ISO_DATETIME_PATTERN


# Test ISO 8601 DateTimeOffset parsing
@pytest.mark.parametrize(
    ("value", "expected_valid"),
    [
        ("2022-04-04T20:44:31Z", True),
        ("2022-04-04T20:44:31.1234567Z", True),
        ("2022-04-04T20:44:31.123456789Z", True),
        ("2022-04-04T20:44:31.1234567+00:00", True),
        ("2022-04-04T20:44:31.1234567-00:00", True),
        ("2022-04-04T20:44:31.1234567+01:00", True),
        ("2022-04-04T20:44:31.1234567-01:00", True),
        ("2022-04-04T20:44:31.1234567-01", False),
        ("20-11-1994T20:44:31Z", False),
    ],
)
def test_iso8601_datetime_offset(value: str, expected_valid: bool) -> None:
    """Test ISO 8601 datetime parsing and roundtrip.

    Mirrors C# Test_ISO8601_DateTimeOffset test.
    """
    # Check if the pattern matches
    pattern_matches = _ISO_DATETIME_PATTERN.match(value) is not None

    if expected_valid:
        assert pattern_matches, f"Expected '{value}' to match ISO 8601 pattern"

        # Parse the datetime
        try:
            parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
        except ValueError:
            pytest.fail(f"Failed to parse valid ISO 8601 datetime: {value}")

        # Format back (simulating write)
        formatted = parsed.isoformat().replace("+00:00", "Z")

        # Parse the formatted value again
        parsed2 = datetime.fromisoformat(formatted.replace("Z", "+00:00"))

        # Verify roundtrip produces equivalent datetime
        assert parsed == parsed2, f"Roundtrip failed for {value}"
    else:
        # For invalid formats, either pattern doesn't match or parsing fails
        if pattern_matches:
            # Pattern matched but should fail parsing
            with pytest.raises(ValueError):  # noqa: PT011
                datetime.fromisoformat(value.replace("Z", "+00:00"))
        else:
            # Pattern correctly rejected the invalid format
            assert not pattern_matches, (
                f"Expected '{value}' to NOT match ISO 8601 pattern"
            )


# Test DataChannelList Schema Validation
@pytest.mark.parametrize("file_name", ["DataChannelList.json"])
def test_data_channel_list_deserialization(file_name: str) -> None:
    """Test DataChannelList JSON schema validation."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Test deserialization
    data: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str)
    assert data is not None
    assert "Package" in data

    # Test serialization round-trip
    json_str2 = Serializer.serialize(data)
    data2: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str2)

    # Compare objects
    assert data == data2


# Test TimeSeriesData Schema Validation
@pytest.mark.parametrize("file_name", ["TimeSeriesData.json"])
def test_time_series_data_deserialization(file_name: str) -> None:
    """Test TimeSeriesData JSON schema validation."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Test deserialization
    data: TimeSeriesDataPackage = Serializer.deserialize_time_series_data(json_str)
    assert data is not None
    assert "Package" in data

    # Test serialization round-trip
    json_str2 = Serializer.serialize(data)
    data2: TimeSeriesDataPackage = Serializer.deserialize_time_series_data(json_str2)

    # Compare objects
    assert data == data2


@pytest.mark.parametrize(
    "file_name",
    ["DataChannelList.json"],
)
def test_data_channel_list_roundtrip(file_name: str) -> None:
    """Test DataChannelList JSON serialization round-trip."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Deserialize
    data: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str)

    # Serialize back to JSON
    json_str2 = Serializer.serialize(data)

    # Deserialize again
    data2: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str2)

    # Compare objects
    assert data == data2


@pytest.mark.parametrize(
    "file_name",
    ["TimeSeriesData.json"],
)
def test_time_series_data_roundtrip(file_name: str) -> None:
    """Test TimeSeriesData JSON serialization round-trip."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Deserialize
    data: TimeSeriesDataPackage = Serializer.deserialize_time_series_data(json_str)

    # Serialize back to JSON
    json_str2 = Serializer.serialize(data)

    # Deserialize again
    data2: TimeSeriesDataPackage = Serializer.deserialize_time_series_data(json_str2)

    # Compare objects
    assert data == data2


@pytest.mark.parametrize("file_name", ["DataChannelList.json"])
def test_data_channel_list_domain_model_roundtrip(file_name: str) -> None:
    """Test DataChannelList JSON to domain model round-trip conversion."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Deserialize to JSON DTO
    data_dto: DataChannelListPackage = Serializer.deserialize_data_channel_list(
        json_str
    )

    # Convert to domain model
    domain_model = JsonExtensions.DataChannelList.to_domain_model(data_dto)

    # Convert back to JSON DTO
    data_dto2 = JsonExtensions.DataChannelList.to_json_dto(domain_model)

    # Compare DTOs
    assert data_dto == data_dto2


@pytest.mark.parametrize("file_name", ["TimeSeriesData.json"])
def test_time_series_data_domain_model_roundtrip(file_name: str) -> None:
    """Test TimeSeriesData JSON to domain model round-trip conversion."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Deserialize to JSON DTO
    data_dto: TimeSeriesDataPackage = Serializer.deserialize_time_series_data(json_str)

    # Convert to domain model
    domain_model = JsonExtensions.TimeSeriesData.to_domain_model(data_dto)

    # Convert back to JSON DTO
    data_dto2 = JsonExtensions.TimeSeriesData.to_json_dto(domain_model)

    # Compare DTOs
    assert data_dto == data_dto2


def test_data_channel_list_structure() -> None:
    """Test DataChannelList JSON structure matches expected TypedDict."""
    file_path = Path(__file__).parent / "json" / "DataChannelList.json"
    with file_path.open() as f:
        json_str = f.read()

    data: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str)

    # Verify structure
    assert "Package" in data
    package = data["Package"]
    assert "Header" in package
    assert "DataChannelList" in package

    header = package["Header"]
    assert "ShipID" in header
    assert "DataChannelListID" in header


def test_time_series_data_structure() -> None:
    """Test TimeSeriesData JSON structure matches expected TypedDict."""
    file_path = Path(__file__).parent / "json" / "TimeSeriesData.json"
    with file_path.open() as f:
        json_str = f.read()

    data: TimeSeriesDataPackage = Serializer.deserialize_time_series_data(json_str)

    # Verify structure
    assert "Package" in data
    package = data["Package"]
    assert "TimeSeriesData" in package

    # Check header if present
    if "Header" in package:
        header = package["Header"]
        assert "ShipID" in header


@pytest.mark.parametrize("file_name", ["DataChannelList.json"])
def test_data_channel_list_id_date_consistency(file_name: str) -> None:
    """Test DataChannelListID timestamp consistency across conversions.

    Mirrors C# Test_DataChannelListId_Date_Consistency test.
    Verifies that timestamps remain consistent through:
    - Original DTO
    - Domain model conversion
    - Serialization/deserialization roundtrip
    """
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Deserialize original package
    package: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str)
    assert package is not None

    # Convert to domain model and back to DTO
    domain_package = JsonExtensions.DataChannelList.to_domain_model(package)
    dto = JsonExtensions.DataChannelList.to_json_dto(domain_package)

    # Serialize and deserialize
    serialized = Serializer.serialize(dto)
    deserialized = Serializer.deserialize_data_channel_list(serialized)
    assert deserialized is not None

    # Verify DTO equivalence
    assert dto == package

    # Extract timestamps from various stages
    dto_timestamp = dto["Package"]["Header"]["DataChannelListID"]["TimeStamp"]
    domain_timestamp = domain_package.package.header.data_channel_list_id.timestamp
    package_timestamp = package["Package"]["Header"]["DataChannelListID"]["TimeStamp"]
    deserialized_timestamp = deserialized["Package"]["Header"]["DataChannelListID"][
        "TimeStamp"
    ]

    # Format domain timestamp to string for comparison
    domain_timestamp_str = domain_timestamp.isoformat().replace("+00:00", "Z")

    # All timestamps should be equal
    assert dto_timestamp == domain_timestamp_str
    assert dto_timestamp == package_timestamp
    assert dto_timestamp == deserialized_timestamp


@pytest.mark.parametrize("file_name", ["DataChannelList.json"])
def test_data_channel_list_compression(file_name: str) -> None:
    """Test DataChannelList JSON compression with bz2.

    Mirrors C# Test_DataChannelList_Compression test.
    Verifies that:
    - Compressed data is smaller than original
    - Decompressed data matches original size
    """
    import bz2

    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Deserialize
    package: DataChannelListPackage = Serializer.deserialize_data_channel_list(json_str)
    assert package is not None

    # Serialize to bytes
    serialized = Serializer.serialize(package).encode("utf-8")

    # Compress
    compressed = bz2.compress(serialized, compresslevel=5)

    # Compressed should be smaller
    assert len(serialized) > len(compressed)

    # Decompress
    decompressed = bz2.decompress(compressed)

    # Decompressed should match original size
    assert len(serialized) == len(decompressed)

    # Content should match
    assert serialized == decompressed
