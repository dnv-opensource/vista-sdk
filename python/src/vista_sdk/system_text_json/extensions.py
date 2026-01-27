"""JSON extension methods for converting between domain objects and JSON DTOs.

Mirrors TypeScript's JSONExtensions pattern.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from vista_sdk.system_text_json.data_channel_list import extensions as _dcl_ext
from vista_sdk.system_text_json.time_series_data import extensions as _tsd_ext

if TYPE_CHECKING:
    from vista_sdk.system_text_json.data_channel_list import DataChannelListPackage
    from vista_sdk.system_text_json.time_series_data import TimeSeriesDataPackage
    from vista_sdk.transport.data_channel import (
        data_channel as dcl_domain,
    )
    from vista_sdk.transport.time_series_data import (
        time_series_data as tsd_domain,
    )


class _DataChannelListExtensions:
    """Conversion utilities for DataChannelList."""

    @staticmethod
    def to_json_dto(
        package: dcl_domain.DataChannelListPackage,
    ) -> DataChannelListPackage:
        """Convert domain DataChannelListPackage to JSON DTO."""
        return _dcl_ext.to_json_dto(package)

    @staticmethod
    def to_domain_model(
        package: DataChannelListPackage,
    ) -> dcl_domain.DataChannelListPackage:
        """Convert JSON DTO to domain DataChannelListPackage."""
        return _dcl_ext.to_domain_model(package)


class _TimeSeriesDataExtensions:
    """Conversion utilities for TimeSeriesData."""

    @staticmethod
    def to_json_dto(
        package: tsd_domain.TimeSeriesDataPackage,
    ) -> TimeSeriesDataPackage:
        """Convert domain TimeSeriesDataPackage to JSON DTO."""
        return _tsd_ext.to_json_dto(package)

    @staticmethod
    def to_domain_model(
        package: TimeSeriesDataPackage,
    ) -> tsd_domain.TimeSeriesDataPackage:
        """Convert JSON DTO to domain TimeSeriesDataPackage."""
        return _tsd_ext.to_domain_model(package)


class JsonExtensions:
    """Extension methods for converting between domain objects and JSON DTOs.

    Mirrors TypeScript's JSONExtensions pattern.

    Example:
        >>> from vista_sdk.system_text_json import JsonExtensions, Serializer
        >>>
        >>> # Domain → DTO → JSON
        >>> dto = JsonExtensions.DataChannelList.to_json_dto(domain_package)
        >>> json_str = Serializer.serialize(dto)
        >>>
        >>> # JSON → DTO → Domain
        >>> dto = Serializer.deserialize_data_channel_list(json_str)
        >>> domain = JsonExtensions.DataChannelList.to_domain_model(dto)
    """

    DataChannelList = _DataChannelListExtensions
    TimeSeriesData = _TimeSeriesDataExtensions
