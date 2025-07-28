from typing import Dict, List, Optional, Any
from datetime import datetime
from .data_channel_list import (
    DataChannelListPackage, DataChannelListContainer, ChannelList,
    Header, DataChannel, ChannelID, Property, DataChannelType,
    Format, Range, Unit, NameObject, Restriction,
    DataChannelListID, VersionInformation
)

def to_json_dto(domain_list: Any) -> DataChannelListPackage:
    """Convert domain DataChannelList to JSON DTO"""
    h = domain_list.Header

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

        header = Header(
            ShipID=str(h.ShipId),
            DateCreated=h.DateCreated,
            DateModified=h.DateModified,
            Author=h.Author,
            SystemConfiguration=system_config,
            CustomHeaders=h.CustomHeaders
        )

    channels = []
    for dc in domain_list.DataChannelList:
        channel = DataChannel(
            DataChannelID=dc.DataChannelID,
            LocalID=dc.LocalID,
            Property=dc.Property,
            Quantity=dc.Quantity,
            Quality=dc.Quality,
            TabularParameters=dc.TabularParameters,
            Package=dc.Package,
            EventDefinitions=dc.EventDefinitions,
            CustomMetadata=dc.CustomMetadata
        )
        channels.append(channel)

    channel_list = ChannelList(DataChannel=channels)
    container = DataChannelListContainer(
        Header=header,
        DataChannelList=channel_list
    )
    return DataChannelListPackage(Package=container)
