"""Extensions for converting DataChannelList domain objects to JSON DTOs."""

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .data_channel_list import DataChannelListContainer

from .data_channel_list import (
    ChannelList,
    ConfigurationReference,
    DataChannel,
    DataChannelListContainer,
    DataChannelListPackage,
    Header,
)


def to_json_dto(domain_list: "DataChannelListContainer") -> DataChannelListPackage:
    """Convert domain DataChannelList to JSON DTO."""
    h = domain_list.Header

    header = None
    if h is not None:
        system_config = None
        if h.SystemConfiguration is not None:
            system_config = [
                ConfigurationReference(ID=r.ID, TimeStamp=r.TimeStamp)
                for r in h.SystemConfiguration
            ]

        header = Header(
            ShipID=str(h.ShipID),
            DateCreated=h.DateCreated,
            DateModified=h.DateModified,
            Author=h.Author,
            SystemConfiguration=system_config,
            CustomHeaders=h.CustomHeaders,
        )

    channels = []
    for dc in domain_list.DataChannelList.DataChannel:
        channel = DataChannel(
            DataChannelID=dc.DataChannelID,
            Property=dc.Property,
        )
        channels.append(channel)

    channel_list = ChannelList(DataChannel=channels)
    if header is None:
        raise ValueError("Header must not be None for DataChannelListContainer")
    container = DataChannelListContainer(Header=header, DataChannelList=channel_list)
    return DataChannelListPackage(Package=container)
