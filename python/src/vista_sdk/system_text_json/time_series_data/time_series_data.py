"""Implementation of JSON serialization models for TimeSeriesData Package."""

from dataclasses import dataclass, field
from datetime import datetime
from typing import Any, Optional


@dataclass
class TimeSeriesDataPackage:
    """JSON serialization model for TimeSeriesData Package wrapper."""

    Package: "Package"


@dataclass
class Package:
    """JSON serialization model for TimeSeriesData Package."""

    Header: Optional["Header"] = None
    TimeSeriesData: list["TimeSeriesData"] = field(default_factory=list)


@dataclass
class Header:
    """JSON serialization model for TimeSeriesData Header."""

    ShipID: str
    TimeSpan: Optional["TimeSpan"] = None
    DateCreated: datetime | None = None
    DateModified: datetime | None = None
    Author: str | None = None
    SystemConfiguration: list["ConfigurationReference"] | None = None
    CustomHeaders: dict[str, object] | None = field(default_factory=dict)


@dataclass
class TimeSpan:
    """JSON serialization model for TimeSeriesData TimeSpan."""

    Start: datetime
    End: datetime


@dataclass
class ConfigurationReference:
    """JSON serialization model for TimeSeriesData ConfigurationReference."""

    ID: str
    TimeStamp: datetime


@dataclass
class TimeSeriesData:
    """JSON serialization model for TimeSeriesData TimeSeriesData."""

    DataConfiguration: ConfigurationReference | None = None
    TabularData: list["TabularData"] | None = None
    EventData: Optional["EventData"] = None
    CustomData: dict[str, object] | None = field(default_factory=dict)


@dataclass
class TabularData:
    """JSON serialization model for TimeSeriesData TabularData."""

    DataChannelID: str
    Rows: list["TabularRow"] = field(default_factory=list)
    CustomMetadata: dict[str, object] | None = field(default_factory=dict)


@dataclass
class TabularRow:
    """JSON serialization model for TimeSeriesData TabularRow."""

    TimeStamp: datetime
    Value: Any
    Quality: str | None = None
    Parameters: dict[str, Any] | None = field(default_factory=dict)


@dataclass
class EventData:
    """JSON serialization model for TimeSeriesData EventData."""

    DataChannelID: str
    Events: list["Event"] = field(default_factory=list)
    CustomMetadata: dict[str, object] | None = field(default_factory=dict)


@dataclass
class Event:
    """JSON serialization model for TimeSeriesData Event."""

    TimeStamp: datetime
    Value: str
    CustomMetadata: dict[str, object] | None = field(default_factory=dict)
