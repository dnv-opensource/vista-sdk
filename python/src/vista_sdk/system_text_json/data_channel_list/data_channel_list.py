from dataclasses import dataclass, field
from typing import Dict, List, Optional
from datetime import datetime

@dataclass
class DataChannelListPackage:
    """JSON serialization model for DataChannelList Package wrapper"""
    Package: 'DataChannelListContainer'

@dataclass
class DataChannelListContainer:
    """JSON serialization model for DataChannelList Package container"""
    Header: 'Header'
    DataChannelList: 'ChannelList'

@dataclass
class ChannelList:
    """JSON serialization model for DataChannelList channels container"""
    DataChannel: List['DataChannel'] = field(default_factory=list)

@dataclass
class Header:
    """JSON serialization model for DataChannelList Header"""
    ShipID: str
    Author: str
    DateCreated: datetime
    DateModified: Optional[datetime] = None
    SystemConfiguration: Optional[List['ConfigurationReference']] = field(default_factory=list)
    DataChannelListID: Optional['DataChannelListID'] = None
    VersionInformation: Optional['VersionInformation'] = None
    CustomHeaders: Optional[Dict[str, object]] = field(default_factory=dict)

@dataclass
class DataChannelListID:
    """JSON serialization model for DataChannelList ID"""
    ID: str
    Version: str
    TimeStamp: datetime

@dataclass
class VersionInformation:
    """JSON serialization model for version information"""
    NamingRule: str
    NamingSchemeVersion: str
    ReferenceURL: str

@dataclass
class DataChannel:
    """JSON serialization model for DataChannel"""
    DataChannelID: 'ChannelID'
    Property: 'Property'

@dataclass
class ChannelID:
    """JSON serialization model for Channel ID"""
    LocalID: str
    ShortID: str
    NameObject: 'NameObject'

@dataclass
class NameObject:
    """JSON serialization model for Name Object"""
    NamingRule: str
    CustomNameObject: Optional[str] = None

@dataclass
class Property:
    """JSON serialization model for Channel Property"""
    DataChannelType: 'DataChannelType'
    QualityCoding: str
    Name: str
    Format: Optional['Format'] = None
    Range: Optional['Range'] = None
    Unit: Optional['Unit'] = None
    AlertPriority: Optional[str] = None
    Remarks: Optional[str] = None
    CustomPropertyElement: Optional[str] = None

@dataclass
class DataChannelType:
    """JSON serialization model for Channel Type"""
    Type: str
    UpdateCycle: Optional[int] = None
    CalculationPeriod: Optional[int] = None

@dataclass
class Format:
    """JSON serialization model for Format"""
    Type: str
    Restriction: Optional['Restriction'] = None

@dataclass
class Restriction:
    """JSON serialization model for Format Restriction"""
    FractionDigits: Optional[int] = None
    MaxInclusive: Optional[float] = None
    MinInclusive: Optional[float] = None
    WhiteSpace: Optional[str] = None
    Enumeration: Optional[List[str]] = None

@dataclass
class Range:
    """JSON serialization model for Range"""
    High: float
    Low: float

@dataclass
class Unit:
    """JSON serialization model for Unit"""
    UnitSymbol: str
    QuantityName: str
    EventDefinitions: Optional[Dict[str, str]] = field(default_factory=dict)
    CustomMetadata: Optional[Dict[str, object]] = field(default_factory=dict)

@dataclass
class ConfigurationReference:
    """JSON serialization model for DataChannelList ConfigurationReference"""
    ID: str
    TimeStamp: datetime
