import json
from datetime import datetime, timezone, timedelta
import pytest
from vista_sdk.system_text_json import (
    TimeSeriesDataPackage, DataChannelListPackage,
    JsonSerializer, time_series_to_json_dto,
    data_channel_list_to_json_dto
)
from vista_sdk.system_text_json.serializer import DateTimeConverter

# Test ISO8601 DateTime parsing
@pytest.mark.parametrize("value,expected_valid", [
    ("2022-04-04T20:44:31Z", True),
    ("2022-04-04T20:44:31.1234567Z", True),
    ("2022-04-04T20:44:31.123456789Z", True),
    ("2022-04-04T20:44:31.1234567+00:00", True),
    ("2022-04-04T20:44:31.1234567-00:00", True),
    ("2022-04-04T20:44:31.1234567+01:00", True),
    ("2022-04-04T20:44:31.1234567-01:00", True),
    ("2022-04-04T20:44:31.1234567-01", False),
    ("20-11-1994T20:44:31Z", False),
])
def test_iso8601_datetime(value: str, expected_valid: bool):
    if expected_valid:
        # Test parsing
        dt = DateTimeConverter.parse(value)
        assert isinstance(dt, datetime)

        # Test round-trip
        formatted = DateTimeConverter.format(dt)
        dt2 = DateTimeConverter.parse(formatted)
        assert dt == dt2
    else:
        with pytest.raises(ValueError):
            DateTimeConverter.parse(value)

# Test DataChannelList Schema Validation
@pytest.mark.parametrize("file_path", [
    "/home/oleeko/workspace/vista-sdk/schemas/json/DataChannelList.sample.json",
    "/home/oleeko/workspace/vista-sdk/schemas/json/DataChannelList.sample.compact.json"
])
def test_data_channel_list_schema_validation(file_path):
    with open(file_path) as f:
        json_str = f.read()

    # Test deserialization
    data = JsonSerializer.deserialize(json_str, DataChannelListPackage)
    assert data is not None

    # Test serialization round-trip
    json_str2 = JsonSerializer.serialize(data)
    data2 = JsonSerializer.deserialize(json_str2, DataChannelListPackage)

    # Compare objects
    assert data == data2

# Test TimeSeriesData Schema Validation
@pytest.mark.parametrize("file_path", [
    "/home/oleeko/workspace/vista-sdk/schemas/json/TimeSeriesData.sample.json"
])
def test_time_series_data_schema_validation(file_path):
    with open(file_path) as f:
        json_str = f.read()

    # Test deserialization
    data = JsonSerializer.deserialize(json_str, TimeSeriesDataPackage)
    assert data is not None

    # Test serialization round-trip
    json_str2 = JsonSerializer.serialize(data)
    data2 = JsonSerializer.deserialize(json_str2, TimeSeriesDataPackage)

    # Compare objects
    assert data == data2

# Test conversion from domain objects
def test_time_series_data_conversion():
    # Create a mock domain object
    class MockTimeSeriesData:
        def __init__(self):
            self.Package = MockPackage()

    class MockPackage:
        def __init__(self):
            self.Header = MockHeader()
            self.TimeSeriesData = []

    class MockHeader:
        def __init__(self):
            self.ShipId = "123"
            self.TimeSpan = MockTimeSpan()
            self.DateCreated = datetime.now(timezone.utc)
            self.DateModified = datetime.now(timezone.utc)
            self.Author = "Test"
            self.SystemConfiguration = None
            self.CustomHeaders = None

    class MockTimeSpan:
        def __init__(self):
            self.Start = datetime.now(timezone.utc)
            self.End = datetime.now(timezone.utc) + timedelta(hours=1)

    # Convert to JSON DTO
    domain_obj = MockTimeSeriesData()
    json_dto = time_series_to_json_dto(domain_obj)

    # Verify conversion
    assert isinstance(json_dto, TimeSeriesDataPackage)
    assert json_dto.Package.Header.ShipID == "123"
    assert json_dto.Package.Header.Author == "Test"
    assert isinstance(json_dto.Package.Header.TimeSpan.Start, datetime)
    assert isinstance(json_dto.Package.Header.TimeSpan.End, datetime)

    # Test conversion from domain objects for DataChannelList
def test_data_channel_list_conversion():
    # Create a mock domain object
    class MockDataChannelList:
        def __init__(self):
            self.Header = MockHeader()
            self.DataChannelList = []

    class MockHeader:
        def __init__(self):
            self.ShipId = "123"
            self.DataChannelListID = "config-123"
            self.DateCreated = datetime.now(timezone.utc)
            self.DateModified = datetime.now(timezone.utc)
            self.Author = "Test"
            self.SystemConfiguration = None
            self.CustomHeaders = None

    # Convert to JSON DTO
    domain_obj = MockDataChannelList()
    json_dto = data_channel_list_to_json_dto(domain_obj)

    # Verify conversion
    assert isinstance(json_dto, DataChannelListPackage)
    assert json_dto.Package.Header.ShipID == "123"
    assert json_dto.Package.Header.Author == "Test"
    assert isinstance(json_dto.Package.Header.DateCreated, datetime)
