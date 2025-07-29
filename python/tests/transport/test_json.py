"""Tests for JSON transport serialization and conversion in vista_sdk.

This module covers:
- ISO8601 DateTime parsing and formatting
- DataChannelList and TimeSeriesData schema validation
- Conversion from domain objects to JSON DTOs
"""

from collections.abc import Iterator
from datetime import datetime, timedelta, timezone
from pathlib import Path

import pytest

from vista_sdk.system_text_json import (
    JsonSerializer,
    TimeSeriesDataPackage,
    time_series_to_json_dto,
)
from vista_sdk.system_text_json.data_channel_list import DataChannelListPackage
from vista_sdk.system_text_json.serializer import DateTimeConverter


# Test ISO8601 DateTime parsing
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
def test_iso8601_datetime(value: str, expected_valid: bool) -> None:
    """Test ISO8601 DateTime parsing and formatting."""
    if expected_valid:
        # Test parsing
        dt = DateTimeConverter.parse(value)
        assert isinstance(dt, datetime)

        # Test round-trip
        formatted = DateTimeConverter.format(dt)
        dt2 = DateTimeConverter.parse(formatted)
        assert dt == dt2
    else:
        with pytest.raises(ValueError):  # noqa: PT011
            DateTimeConverter.parse(value)


# Test DataChannelList Schema Validation
@pytest.mark.parametrize("file_name", ["DataChannelList.json"])
def test_data_channel_list_schema_validation(file_name: str) -> None:
    """Test DataChannelList JSON schema validation."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Test deserialization
    data = JsonSerializer.deserialize(json_str, DataChannelListPackage)
    assert data is not None

    # Test serialization round-trip
    json_str2 = JsonSerializer.serialize(data)
    data2 = JsonSerializer.deserialize(
        json_str2, DataChannelListPackage
    )  # Compare objects
    assert data == data2


# Test TimeSeriesData Schema Validation
@pytest.mark.parametrize("file_name", ["TimeSeriesData.json"])
def test_time_series_data_schema_validation(file_name: str) -> None:
    """Test TimeSeriesData JSON schema validation."""
    file_path = Path(__file__).parent / "json" / file_name
    with file_path.open() as f:
        json_str = f.read()

    # Test deserialization
    data = JsonSerializer.deserialize(json_str, TimeSeriesDataPackage)
    assert data is not None

    # Test serialization round-trip
    json_str2 = JsonSerializer.serialize(data)
    data2 = JsonSerializer.deserialize(json_str2, TimeSeriesDataPackage)

    # Compare objects
    assert data == data2


# --- Shared Mock Classes ---
class MockTimeSpan:
    """Mock TimeSpan class for testing."""

    def __init__(self) -> None:
        """Initialize with mock start and end times."""
        self.Start = datetime.now(timezone.utc) + timedelta(hours=1)
        self.End = datetime.now(timezone.utc) + timedelta(hours=1)


class MockHeader:
    """Mock Header class for testing."""

    def __init__(self) -> None:
        """Initialize with mock values."""
        self.ShipID: str = "123"
        self.TimeSpan = MockTimeSpan()
        self.DateCreated = datetime.now(timezone.utc)
        self.DateModified = datetime.now(timezone.utc)
        self.Author = "Test"
        self.SystemConfiguration = None
        self.CustomHeaders = None


class MockPackage:
    """Mock Package class for testing."""

    def __init__(self) -> None:
        """Initialize with a mock header and empty DataChannelList."""
        self.Header: MockHeader = MockHeader()
        self.TimeSeriesData: list = []
        self.DataChannelList: MockChannelList | None = None


class MockTimeSeriesData:
    """Mock TimeSeriesData class for testing."""

    def __init__(self) -> None:
        """Initialize with a mock package."""
        self.Package = MockPackage()


class MockChannelList:
    """Mock ChannelList class for testing."""

    def __init__(self) -> None:
        """Initialize with an empty list of DataChannel and a mock Header."""
        self.DataChannel: list = []
        self.Header = MockHeader()

    def __iter__(self) -> Iterator:
        """Return an iterator over the DataChannel list."""
        return iter(self.DataChannel)


class MockDataChannelList:
    """Mock DataChannelList class for testing."""

    def __init__(self) -> None:
        """Initialize with a mock package and channel list."""
        self.Package = MockPackage()
        self.Package.DataChannelList = MockChannelList()
        if self.Package.DataChannelList and hasattr(
            self.Package.DataChannelList, "Header"
        ):
            self.Package.Header = self.Package.DataChannelList.Header


# --- Test conversion from domain objects ---
def test_time_series_data_conversion() -> None:
    """Test conversion from TimeSeriesData domain object to JSON DTO."""
    domain_obj = MockTimeSeriesData()
    json_dto = time_series_to_json_dto(domain_obj)
    assert isinstance(json_dto, TimeSeriesDataPackage)
    assert json_dto.Package.Header.ShipID == "123"  # type: ignore
    assert json_dto.Package.Header.Author == "Test"  # type: ignore
    assert isinstance(json_dto.Package.Header.TimeSpan.Start, datetime)  # type: ignore
    assert isinstance(json_dto.Package.Header.TimeSpan.End, datetime)  # type: ignore
