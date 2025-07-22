"""ISO19848 implementation for transport module."""

from __future__ import annotations

from enum import Enum
from typing import Protocol, Union

from vista_sdk.transport.iso19848_dtos import (
    DataChannelTypeNameDto,
    DataChannelTypeNamesDto,
    FormatDataTypeDto,
    FormatDataTypesDto,
)


class ISO19848Version(Enum):
    """ISO19848 version enumeration."""

    V2018 = "v2018"
    V2024 = "v2024"


class DataChannelTypeNames:
    """Data channel type names."""

    def __init__(self, dto: DataChannelTypeNamesDto) -> None:
        """Initialize with DTO."""
        self._dto = dto

    def parse(
        self, value: str
    ) -> Union[
        DataChannelTypeNames.ParseResult.Ok, DataChannelTypeNames.ParseResult.Invalid
    ]:
        """Parse a data channel type name."""
        for type_name in self._dto.values:
            if type_name.type == value:
                return DataChannelTypeNames.ParseResult.Ok(type_name)
        return DataChannelTypeNames.ParseResult.Invalid(
            f"Unknown data channel type: {value}"
        )

    class ParseResult:
        """Parse result for data channel type names."""

        class Ok:
            """Successful parse result."""

            def __init__(self, type_name: DataChannelTypeNameDto) -> None:
                """Initialize with type name."""
                self.type_name = type_name

        class Invalid:
            """Invalid parse result."""

            def __init__(self, message: str) -> None:
                """Initialize with error message."""
                self.message = message


class FormatDataTypes:
    """Format data types."""

    def __init__(self, dto: FormatDataTypesDto) -> None:
        """Initialize with DTO."""
        self._dto = dto

    def parse(
        self, value: str
    ) -> Union[FormatDataTypes.ParseResult.Ok, FormatDataTypes.ParseResult.Invalid]:
        """Parse a format data type."""
        for type_name in self._dto.values:
            if type_name.type == value:
                return FormatDataTypes.ParseResult.Ok(type_name)
        return FormatDataTypes.ParseResult.Invalid(f"Unknown format data type: {value}")

    class ParseResult:
        """Parse result for format data types."""

        class Ok:
            """Successful parse result."""

            def __init__(self, type_name: FormatDataTypeDto) -> None:
                """Initialize with type name."""
                self.type_name = type_name

        class Invalid:
            """Invalid parse result."""

            def __init__(self, message: str) -> None:
                """Initialize with error message."""
                self.message = message


class IISO19848(Protocol):
    """ISO19848 interface."""

    def get_data_channel_type_names(
        self, version: ISO19848Version
    ) -> DataChannelTypeNames:
        """Get data channel type names for version."""
        ...

    def get_format_data_types(self, version: ISO19848Version) -> FormatDataTypes:
        """Get format data types for version."""
        ...


class ISO19848:
    """ISO19848 implementation."""

    LATEST_VERSION = ISO19848Version.V2024
    _instance: ISO19848 | None = None

    def __init__(self) -> None:
        """Initialize ISO19848."""
        # Simple cache implementation - in real implementation would use proper caching
        self._data_channel_type_names_cache: dict[
            ISO19848Version, DataChannelTypeNames
        ] = {}
        self._format_data_types_cache: dict[ISO19848Version, FormatDataTypes] = {}

    @classmethod
    def get_instance(cls) -> ISO19848:
        """Get singleton instance."""
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    def get_data_channel_type_names(
        self, version: ISO19848Version
    ) -> DataChannelTypeNames:
        """Get data channel type names for version."""
        if version not in self._data_channel_type_names_cache:
            # In real implementation, this would load from resources
            # For now, return empty implementation
            dto = DataChannelTypeNamesDto(values=[])
            self._data_channel_type_names_cache[version] = DataChannelTypeNames(dto)
        return self._data_channel_type_names_cache[version]

    def get_format_data_types(self, version: ISO19848Version) -> FormatDataTypes:
        """Get format data types for version."""
        if version not in self._format_data_types_cache:
            # In real implementation, this would load from resources
            # For now, return empty implementation
            dto = FormatDataTypesDto(values=[])
            self._format_data_types_cache[version] = FormatDataTypes(dto)
        return self._format_data_types_cache[version]


# Singleton instance
Instance = ISO19848.get_instance()
