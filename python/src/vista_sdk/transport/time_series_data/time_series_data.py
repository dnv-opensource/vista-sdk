"""Time series data implementation for transport module."""

from __future__ import annotations

from collections.abc import Callable
from datetime import datetime, timezone

# Forward declare to avoid circular imports
from typing import TYPE_CHECKING, Any

from vista_sdk.local_id import LocalId
from vista_sdk.result import Invalid, Ok, ValidateResult
from vista_sdk.transport.ship_id import ShipId

if TYPE_CHECKING:
    from vista_sdk.transport.data_channel.data_channel import (
        DataChannel,
        DataChannelListPackage,
    )
    from vista_sdk.transport.value import AnyValue

from vista_sdk.transport.time_series_data.data_channel_id import DataChannelId


class TimeSeriesDataPackage:
    """Package containing time series data."""

    def __init__(self, package: Package) -> None:
        """Initialize with package."""
        self.package = package


class Package:
    """Package containing header and time series data."""

    def __init__(
        self, header: Header | None, time_series_data: list[TimeSeriesData]
    ) -> None:
        """Initialize package."""
        self.header = header
        self.time_series_data = time_series_data


class Header:
    """Header containing ship ID and configuration information."""

    def __init__(
        self,
        ship_id: ShipId,
        time_span: TimeSpan | None,
        author: str | None,
        date_created: datetime | None = None,
        date_modified: datetime | None = None,
        system_configuration: list[ConfigurationReference] | None = None,
        custom_headers: dict[str, Any] | None = None,
    ) -> None:
        """Initialize header."""
        self.ship_id = ship_id
        self.time_span = time_span
        self.author = author
        self.date_created = date_created or datetime.now(timezone.utc)
        self.date_modified = date_modified
        self.system_configuration = system_configuration
        self.custom_headers = custom_headers


class TimeSpan:
    """Time span with start and end."""

    def __init__(self, start: datetime, end: datetime) -> None:
        """Initialize time span."""
        self.start = start
        self.end = end


class ConfigurationReference:
    """Configuration reference with ID and timestamp."""

    def __init__(self, id_value: str, time_stamp: datetime) -> None:
        """Initialize configuration reference."""
        self.id = id_value
        self.time_stamp = time_stamp


# Custom definitions
ValidateData = Callable[
    [datetime, "DataChannel", "AnyValue", str | None], ValidateResult
]


class TimeSeriesData:
    """Time series data with configuration and data."""

    def __init__(
        self,
        data_configuration: ConfigurationReference | None,
        tabular_data: list[TabularData] | None,
        event_data: EventData | None,
        custom_data_kinds: dict[str, Any] | None = None,
    ) -> None:
        """Initialize time series data."""
        self.data_configuration = data_configuration
        self.tabular_data = tabular_data
        self.event_data = event_data
        self.custom_data_kinds = custom_data_kinds

    def validate(  # noqa : C901
        self,
        dc_package: DataChannelListPackage,
        on_tabular_data: ValidateData,
        on_event_data: ValidateData,
    ) -> ValidateResult:
        """Validate the time series data."""
        if (
            self.data_configuration
            and dc_package.package.header.data_channel_list_id.id
            != self.data_configuration.id
        ):
            return Invalid(["DataConfiguration Id does not match DataChannelList Id"])

        if (not self.tabular_data or len(self.tabular_data) == 0) and (
            not self.event_data
            or not self.event_data.data_set
            or len(self.event_data.data_set) == 0
        ):
            return Invalid(["Can't ingest timeseries data without data"])

        erroneous_data_channels: list[tuple[DataChannelId, str]] = []

        # Validate Tabular data
        for table in self.tabular_data or []:
            if not table or not table.data_sets or not table.data_channel_ids:
                continue
            if len(table.data_sets) != table.number_of_data_sets:
                return Invalid(
                    [
                        f"Tabular data expects {table.number_of_data_sets} datasets, "
                        f"but {len(table.data_sets)} sets are provided"
                    ]
                )

            result = TabularData.validate(table)
            if not isinstance(result, Ok):
                return result

            for i, dataset in enumerate(table.data_sets):
                if len(dataset.value) != table.number_of_data_channels:
                    return Invalid(
                        [
                            f"Tabular data set {i} expects {len(table.data_channel_ids)} values, "  # noqa : E501
                            f"but {len(dataset.value)} values are provided"
                        ]
                    )

                for j, data_channel_id in enumerate(table.data_channel_ids):
                    data_channel = data_channel_id.match(
                        on_local_id=lambda local_id, dc_id=data_channel_id: (
                            self._get_data_channel_by_local_id(
                                dc_package,
                                dc_id,
                                local_id,
                                erroneous_data_channels,
                            )
                        ),
                        on_short_id=lambda short_id, dc_id=data_channel_id: (
                            self._get_data_channel_by_short_id(
                                dc_package,
                                dc_id,
                                short_id,
                                erroneous_data_channels,
                            )
                        ),
                    )

                    if not data_channel:
                        continue

                    # Validate value and get parsed value
                    if data_channel.prop is None:
                        erroneous_data_channels.append(
                            (data_channel_id, "DataChannel property is None")
                        )
                        continue

                    type_validation = data_channel.prop.format.validate_value(
                        dataset.value[j]
                    )
                    if isinstance(type_validation[0], Invalid):
                        erroneous_data_channels.append(
                            (data_channel_id, ", ".join(type_validation[0].messages))
                        )
                        continue

                    parsed_value = type_validation[1]
                    if parsed_value is None:
                        erroneous_data_channels.append(
                            (data_channel_id, "Parsed value is None")
                        )
                        continue

                    quality = (
                        dataset.quality[j]
                        if dataset.quality and j < len(dataset.quality)
                        else None
                    )
                    # Only call on_tabular_data if parsed_value is not None
                    # parsed_value is guaranteed not None here
                    if parsed_value is not None:
                        result = on_tabular_data(
                            dataset.time_stamp, data_channel, parsed_value, quality
                        )

                        if not isinstance(result, Ok):
                            erroneous_data_channels.append(
                                (data_channel_id, str(result))
                            )

        # Validate event data
        if self.event_data and self.event_data.data_set:
            for event_data_item in self.event_data.data_set:
                data_channel = event_data_item.data_channel_id.match(
                    on_local_id=lambda local_id, event_item=event_data_item: (
                        self._get_data_channel_by_local_id(
                            dc_package,
                            event_item.data_channel_id,
                            local_id,
                            erroneous_data_channels,
                        )
                    ),
                    on_short_id=lambda short_id, event_item=event_data_item: (
                        self._get_data_channel_by_short_id(
                            dc_package,
                            event_item.data_channel_id,
                            short_id,
                            erroneous_data_channels,
                        )
                    ),
                )

                if not data_channel:
                    continue

                if data_channel.prop is None:
                    erroneous_data_channels.append(
                        (
                            event_data_item.data_channel_id,
                            "DataChannel property is None",
                        )
                    )
                    continue

                type_validation = data_channel.prop.format.validate_value(
                    event_data_item.value
                )
                if isinstance(type_validation[0], Invalid):
                    erroneous_data_channels.append(
                        (
                            event_data_item.data_channel_id,
                            ", ".join(type_validation[0].messages),
                        )
                    )
                    continue

                parsed_value = type_validation[1]
                if parsed_value is not None:
                    result = on_event_data(
                        event_data_item.time_stamp,
                        data_channel,
                        parsed_value,
                        event_data_item.quality,
                    )

                    if not isinstance(result, Ok):
                        erroneous_data_channels.append(
                            (event_data_item.data_channel_id, str(result))
                        )
                else:
                    erroneous_data_channels.append(
                        (event_data_item.data_channel_id, "Parsed value is None")
                    )

        if erroneous_data_channels:
            error_messages = [
                f"DataChannel {dc_id} is invalid: {cause}"
                for dc_id, cause in erroneous_data_channels
            ]
            return Invalid(error_messages)

        return Ok()

    def _get_data_channel_by_local_id(
        self,
        dc_package: DataChannelListPackage,
        data_channel_id: DataChannelId,
        local_id: LocalId,
        erroneous_data_channels: list[tuple[DataChannelId, str]],
    ) -> DataChannel | None:
        """Get data channel by local ID."""
        success, data_channel = (
            dc_package.package.data_channel_list.try_get_by_local_id(local_id)
        )
        if not success or not data_channel:
            erroneous_data_channels.append(
                (data_channel_id, f"Data channel with localId '{local_id}' not found")
            )
            return None
        return data_channel

    def _get_data_channel_by_short_id(
        self,
        dc_package: DataChannelListPackage,
        data_channel_id: DataChannelId,
        short_id: str,
        erroneous_data_channels: list[tuple[DataChannelId, str]],
    ) -> DataChannel | None:
        """Get data channel by short ID."""
        success, data_channel = (
            dc_package.package.data_channel_list.try_get_by_short_id(short_id)
        )
        if not success or not data_channel:
            erroneous_data_channels.append(
                (data_channel_id, f"Data channel with short id '{short_id}' not found")
            )
            return None
        return data_channel


class TabularData:
    """Tabular data with data sets and channel IDs."""

    def __init__(
        self,
        data_channel_ids: list[DataChannelId] | None,
        data_sets: list[TabularDataSet] | None,
    ) -> None:
        """Initialize tabular data."""
        self.data_channel_ids = data_channel_ids
        self.data_sets = data_sets

    @property
    def number_of_data_sets(self) -> int | None:
        """Get the number of data sets."""
        return len(self.data_sets) if self.data_sets else None

    @property
    def number_of_data_channels(self) -> int | None:
        """Get the number of data channels."""
        return len(self.data_channel_ids) if self.data_channel_ids else None

    @staticmethod
    def validate(table: TabularData) -> ValidateResult:
        """Validate the tabular data."""
        # Ensure data channels are provided
        if not table.data_channel_ids or len(table.data_channel_ids) == 0:
            return Invalid(["Tabular data has no data channels"])
        if (
            table.number_of_data_channels is not None
            and table.number_of_data_channels != len(table.data_channel_ids)
        ):
            return Invalid(
                [
                    f"Tabular data has {table.number_of_data_channels} data channels, "
                    f"but {len(table.data_channel_ids)} data channels are provided"
                ]
            )

        # Ensure data sets are provided
        if not table.data_sets or len(table.data_sets) == 0:
            return Invalid(["Tabular data has no data"])

        if table.number_of_data_sets is not None and table.number_of_data_sets != len(
            table.data_sets
        ):
            return Invalid(
                [
                    f"Tabular data has {table.number_of_data_sets} data sets, "
                    f"but {len(table.data_sets)} data sets are provided"
                ]
            )

        return Ok()


class EventData:
    """Event data with data sets."""

    def __init__(self, data_set: list[EventDataSet] | None = None) -> None:
        """Initialize event data."""
        self.data_set = data_set or []

    @property
    def number_of_data_set(self) -> int | None:
        """Get the number of data sets."""
        return len(self.data_set) if self.data_set else None


class TabularDataSet:
    """Tabular data set with timestamp and values."""

    def __init__(
        self, time_stamp: datetime, value: list[str], quality: list[str] | None = None
    ) -> None:
        """Initialize tabular data set."""
        self.time_stamp = time_stamp
        self.value = value
        self.quality = quality


class EventDataSet:
    """Event data set with timestamp, channel ID, and value."""

    def __init__(
        self,
        time_stamp: datetime,
        data_channel_id: DataChannelId,
        value: str,
        quality: str | None = None,
    ) -> None:
        """Initialize event data set."""
        self.time_stamp = time_stamp
        self.data_channel_id = data_channel_id
        self.value = value
        self.quality = quality
