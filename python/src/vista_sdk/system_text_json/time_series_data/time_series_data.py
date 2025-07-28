from dataclasses import dataclass, field
from typing import Dict, List, Optional, Union, Any
from datetime import datetime

@dataclass
class TimeSeriesDataPackage:
    """JSON serialization model for TimeSeriesData Package wrapper"""
    Package: 'Package'

@dataclass
class Package:
    """JSON serialization model for TimeSeriesData Package"""
    Header: Optional['Header'] = None
    TimeSeriesData: List['TimeSeriesData'] = field(default_factory=list)

@dataclass
class Header:
    """JSON serialization model for TimeSeriesData Header"""
    ShipID: str
    TimeSpan: Optional['TimeSpan'] = None
    DateCreated: Optional[datetime] = None
    DateModified: Optional[datetime] = None
    Author: Optional[str] = None
    SystemConfiguration: Optional[List['ConfigurationReference']] = None
    CustomHeaders: Optional[Dict[str, object]] = field(default_factory=dict)

@dataclass
class TimeSpan:
    """JSON serialization model for TimeSeriesData TimeSpan"""
    Start: datetime
    End: datetime

@dataclass
class ConfigurationReference:
    """JSON serialization model for TimeSeriesData ConfigurationReference"""
    ID: str
    TimeStamp: datetime

@dataclass
class TimeSeriesData:
    """JSON serialization model for TimeSeriesData TimeSeriesData"""
    DataConfiguration: Optional[ConfigurationReference] = None
    TabularData: Optional[List['TabularData']] = None
    EventData: Optional['EventData'] = None
    CustomData: Optional[Dict[str, object]] = field(default_factory=dict)

@dataclass
class TabularData:
    """JSON serialization model for TimeSeriesData TabularData"""
    DataChannelID: str
    Rows: List['TabularRow'] = field(default_factory=list)
    CustomMetadata: Optional[Dict[str, object]] = field(default_factory=dict)

@dataclass
class TabularRow:
    """JSON serialization model for TimeSeriesData TabularRow"""
    TimeStamp: datetime
    Value: Any
    Quality: Optional[str] = None
    Parameters: Optional[Dict[str, Any]] = field(default_factory=dict)

@dataclass
class EventData:
    """JSON serialization model for TimeSeriesData EventData"""
    DataChannelID: str
    Events: List['Event'] = field(default_factory=list)
    CustomMetadata: Optional[Dict[str, object]] = field(default_factory=dict)

@dataclass
class Event:
    """JSON serialization model for TimeSeriesData Event"""
    TimeStamp: datetime
    Value: str
    CustomMetadata: Optional[Dict[str, object]] = field(default_factory=dict)
