"""This file is part of the VISTA SDK."""

from .data_channel_list import (
    ConfigurationReference as DataChannelConfigurationReference,
)
from .data_channel_list import (
    DataChannel,
    DataChannelID,
    DataChannelList,
    DataChannelListPackage,
    DataChannelType,
    Format,
    NameObject,
    Property,
    Range,
    Restriction,
    Unit,
    VersionInformation,
)
from .data_channel_list import Header as DataChannelHeader
from .data_channel_list import (
    Package as DataChannelPackage,
)
from .extensions import JsonExtensions
from .serializer import DateTimeEncoder, Serializer
from .time_series_data import (
    ConfigurationReference,
    DataSetEvent,
    DataSetTabular,
    EventData,
    Package,
    TabularData,
    TimeSeriesData,
    TimeSeriesDataPackage,
    TimeSpan,
)
from .time_series_data import Header as TimeSeriesHeader

__all__ = [
    # Time Series Data DTOs
    "ConfigurationReference",
    # Data Channel List DTOs
    "DataChannel",
    "DataChannelConfigurationReference",
    "DataChannelHeader",
    "DataChannelID",
    "DataChannelList",
    "DataChannelListPackage",
    "DataChannelPackage",
    "DataChannelType",
    "DataSetEvent",
    "DataSetTabular",
    # DateTime encoder for json.dumps
    "DateTimeEncoder",
    "EventData",
    "Format",
    # Extensions (mirrors TS JSONExtensions)
    "JsonExtensions",
    "NameObject",
    "Package",
    "Property",
    "Range",
    "Restriction",
    # Serializer (mirrors C# Serializer)
    "Serializer",
    "TabularData",
    "TimeSeriesData",
    "TimeSeriesDataPackage",
    "TimeSeriesHeader",
    "TimeSpan",
    "Unit",
    "VersionInformation",
]
