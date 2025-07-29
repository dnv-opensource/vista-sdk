"""Extensions for converting DataChannelList domain objects to JSON DTOs."""

from dataclasses import dataclass, field
from datetime import datetime
from typing import Optional


@dataclass
class DataChannelListPackage:
    """JSON serialization model for DataChannelList Package wrapper."""

    Package: "DataChannelListContainer"


@dataclass
class DataChannelListContainer:
    """JSON serialization model for DataChannelList Package container."""

    Header: "Header"
    DataChannelList: "ChannelList"


@dataclass
class ChannelList:
    """JSON serialization model for DataChannelList channels container."""

    DataChannel: list["DataChannel"] = field(default_factory=list)


@dataclass
class Header:
    """JSON serialization model for DataChannelList Header."""

    ShipID: str
    Author: str
    DateCreated: datetime
    DateModified: datetime | None = None
    SystemConfiguration: list["ConfigurationReference"] | None = field(
        default_factory=list
    )
    DataChannelListID: Optional["DataChannelListID"] = None
    VersionInformation: Optional["VersionInformation"] = None
    CustomHeaders: dict[str, object] | None = field(default_factory=dict)


@dataclass
class DataChannelListID:
    """JSON serialization model for DataChannelList ID."""

    ID: str
    Version: str
    TimeStamp: datetime


@dataclass
class VersionInformation:
    """JSON serialization model for version information."""

    NamingRule: str
    NamingSchemeVersion: str
    ReferenceURL: str


@dataclass
class DataChannel:
    """JSON serialization model for DataChannel."""

    DataChannelID: "ChannelID"
    Property: "Property"


@dataclass
class ChannelID:
    """JSON serialization model for Channel ID."""

    LocalID: str
    ShortID: str
    NameObject: "NameObject"


@dataclass
class NameObject:
    """JSON serialization model for Name Object."""

    NamingRule: str
    CustomNameObject: str | None = None


@dataclass
class Property:
    """JSON serialization model for Channel Property."""

    DataChannelType: "DataChannelType"
    QualityCoding: str
    Name: str
    Format: Optional["Format"] = None
    Range: Optional["Range"] = None
    Unit: Optional["Unit"] = None
    AlertPriority: str | None = None
    Remarks: str | None = None
    CustomPropertyElement: str | None = None


@dataclass
class DataChannelType:
    """JSON serialization model for Channel Type."""

    Type: str
    UpdateCycle: int | None = None
    CalculationPeriod: int | None = None


@dataclass
class Format:
    """JSON serialization model for Format."""

    Type: str
    Restriction: Optional["Restriction"] = None


@dataclass
class Restriction:
    """JSON serialization model for Format Restriction."""

    FractionDigits: int | None = None
    MaxInclusive: float | None = None
    MinInclusive: float | None = None
    WhiteSpace: str | None = None
    Enumeration: list[str] | None = None


@dataclass
class Range:
    """JSON serialization model for Range."""

    High: float
    Low: float


@dataclass
class Unit:
    """JSON serialization model for Unit."""

    UnitSymbol: str
    QuantityName: str
    EventDefinitions: dict[str, str] | None = field(default_factory=dict)
    CustomMetadata: dict[str, object] | None = field(default_factory=dict)


@dataclass
class ConfigurationReference:
    """JSON serialization model for DataChannelList ConfigurationReference."""

    ID: str
    TimeStamp: datetime
