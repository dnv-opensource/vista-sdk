from typing import Dict, List, Optional, Any
from datetime import datetime
from .time_series_data import (
    TimeSeriesDataPackage, Package, Header, TimeSpan,
    ConfigurationReference, TimeSeriesData, TabularData,
    TabularRow, EventData, Event
)

def to_json_dto(domain_package: Any) -> TimeSeriesDataPackage:
    """Convert domain TimeSeriesDataPackage to JSON DTO"""
    p = domain_package.Package
    h = domain_package.Package.Header

    header = None
    if h is not None:
        system_config = None
        if h.SystemConfiguration is not None:
            system_config = [
                ConfigurationReference(
                    ID=r.Id,
                    TimeStamp=r.TimeStamp
                ) for r in h.SystemConfiguration
            ]

        time_span = None
        if h.TimeSpan is not None:
            time_span = TimeSpan(
                Start=h.TimeSpan.Start,
                End=h.TimeSpan.End
            )

        header = Header(
            ShipID=str(h.ShipId),
            TimeSpan=time_span,
            DateCreated=h.DateCreated,
            DateModified=h.DateModified,
            Author=h.Author,
            SystemConfiguration=system_config,
            CustomHeaders=h.CustomHeaders
        )

    time_series_data = []
    for t in p.TimeSeriesData:
        data_config = None
        if t.DataConfiguration is not None:
            data_config = ConfigurationReference(
                ID=t.DataConfiguration.Id,
                TimeStamp=t.DataConfiguration.TimeStamp
            )

        event_data = None
        if t.EventData is not None:
            events = [
                Event(
                    TimeStamp=e.TimeStamp,
                    Value=e.Value,
                    CustomMetadata=e.CustomMetadata
                ) for e in t.EventData.Events
            ]
            event_data = EventData(
                DataChannelID=t.EventData.DataChannelID,
                Events=events,
                CustomMetadata=t.EventData.CustomMetadata
            )

        tabular_data = None
        if t.TabularData is not None:
            tabular_data = []
            for td in t.TabularData:
                rows = [
                    TabularRow(
                        TimeStamp=row.TimeStamp,
                        Value=row.Value,
                        Quality=row.Quality,
                        Parameters=row.Parameters
                    ) for row in td.Rows
                ]
                tabular_data.append(
                    TabularData(
                        DataChannelID=td.DataChannelID,
                        Rows=rows,
                        CustomMetadata=td.CustomMetadata
                    )
                )

        time_series_data.append(
            TimeSeriesData(
                DataConfiguration=data_config,
                TabularData=tabular_data,
                EventData=event_data,
                CustomData=t.CustomData
            )
        )

    package = Package(
        Header=header,
        TimeSeriesData=time_series_data
    )

    return TimeSeriesDataPackage(Package=package)
