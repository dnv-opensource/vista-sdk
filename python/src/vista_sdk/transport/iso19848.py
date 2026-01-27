"""ISO19848 implementation for transport module."""

from __future__ import annotations

import gzip
import importlib.resources as pkg_resources
import json
from collections.abc import Callable, Iterator
from datetime import datetime
from decimal import InvalidOperation
from enum import Enum
from typing import Protocol, TypeVar

from vista_sdk.result import Invalid, Ok, ValidateResult
from vista_sdk.transport.iso19848_dtos import (
    DataChannelTypeNameDto,
    DataChannelTypeNamesDto,
    FormatDataTypeDto,
    FormatDataTypesDto,
)
from vista_sdk.transport.value import (
    AnyValue,
    BooleanValue,
    DateTimeValue,
    DecimalValue,
    IntegerValue,
    StringValue,
)

T = TypeVar("T")


class ISO19848Version(Enum):
    """ISO19848 version enumeration."""

    V2018 = "v2018"
    V2024 = "v2024"


class DataChannelTypeName:
    """Data channel type name domain object."""

    def __init__(self, type_name: str, description: str) -> None:
        """Initialize with type and description."""
        self.type = type_name
        self.description = description


class DataChannelTypeNames:
    """Data channel type names collection."""

    def __init__(self, values: list[DataChannelTypeName]) -> None:
        """Initialize with list of type names."""
        self._values = values

    def parse(
        self, value: str
    ) -> DataChannelTypeNames.ParseResult.Ok | DataChannelTypeNames.ParseResult.Invalid:
        """Parse a data channel type name."""
        for type_name in self._values:
            if type_name.type == value:
                return DataChannelTypeNames.ParseResult.Ok(type_name)
        return DataChannelTypeNames.ParseResult.Invalid(
            f"Unknown data channel type: {value}"
        )

    def __iter__(self) -> Iterator[DataChannelTypeName]:
        """Iterate over type names."""
        return iter(self._values)

    class ParseResult:
        """Parse result for data channel type names."""

        class Ok:
            """Successful parse result."""

            def __init__(self, type_name: DataChannelTypeName) -> None:
                """Initialize with type name."""
                self.type_name = type_name

        class Invalid:
            """Invalid parse result."""

            def __init__(self, message: str) -> None:
                """Initialize with error message."""
                self.message = message


class FormatDataTypes:
    """Format data types collection."""

    def __init__(self, values: list[FormatDataType]) -> None:
        """Initialize with list of format data types."""
        self._values = values

    def parse(
        self, value: str
    ) -> FormatDataTypes.ParseResult.Ok | FormatDataTypes.ParseResult.Invalid:
        """Parse a format data type."""
        for type_name in self._values:
            if type_name.type == value:
                return FormatDataTypes.ParseResult.Ok(type_name)
        return FormatDataTypes.ParseResult.Invalid(f"Unknown format data type: {value}")

    def __iter__(self) -> Iterator[FormatDataType]:
        """Iterate over format data types."""
        return iter(self._values)

    class ParseResult:
        """Parse result for format data types."""

        class Ok:
            """Successful parse result."""

            def __init__(self, type_name: FormatDataType) -> None:
                """Initialize with type name."""
                self.type_name = type_name

        class Invalid:
            """Invalid parse result."""

            def __init__(self, message: str) -> None:
                """Initialize with error message."""
                self.message = message


class FormatDataType:
    """Represents a format data type with validation."""

    def __init__(self, type_name: str, description: str = "") -> None:
        """Initialize format data type."""
        self.type = type_name
        self.description = description

    def validate(self, value: str) -> tuple[ValidateResult, AnyValue | None]:
        """Validate and parse a string value, returning both result and parsed value."""
        if self.type == "Decimal":
            try:
                return Ok(), DecimalValue(float(value))
            except InvalidOperation:
                return Invalid([f"Invalid decimal value - Value='{value}'"]), None
        if self.type == "Integer":
            try:
                return Ok(), IntegerValue(int(value))
            except ValueError:
                return Invalid([f"Invalid integer value - Value='{value}'"]), None
        if self.type == "Boolean":
            lower_value = value.lower()
            if lower_value not in ("true", "false"):
                return Invalid([f"Invalid boolean value - Value='{value}'"]), None
            return Ok(), BooleanValue(lower_value == "true")
        if self.type == "String":
            return Ok(), StringValue(value)
        if self.type == "DateTime":
            try:
                # C# uses pattern "yyyy-MM-ddTHH:mm:ssZ"
                parsed_dt = datetime.fromisoformat(value.replace("Z", "+00:00"))
                return Ok(), DateTimeValue(parsed_dt)
            except ValueError:
                return Invalid([f"Invalid datetime value - Value='{value}'"]), None
        return Invalid([f"Invalid format type {self.type}"]), None

    def switch(
        self,
        value: str,
        on_decimal: Callable[[float], None],
        on_integer: Callable[[int], None],
        on_boolean: Callable[[bool], None],
        on_string: Callable[[str], None],
        on_datetime: Callable[[datetime], None],
    ) -> None:
        """Pattern match on the format data type and execute callback."""
        result, parsed_value = self.validate(value)
        if isinstance(result, Invalid):
            raise ValueError("Invalid value")
        if isinstance(parsed_value, DecimalValue):
            on_decimal(parsed_value.value)
        elif isinstance(parsed_value, IntegerValue):
            on_integer(parsed_value.value)
        elif isinstance(parsed_value, BooleanValue):
            on_boolean(parsed_value.value)
        elif isinstance(parsed_value, StringValue):
            on_string(parsed_value.value)
        elif isinstance(parsed_value, DateTimeValue):
            on_datetime(parsed_value.value)
        else:
            raise ValueError("Invalid type")

    def match(
        self,
        value: str,
        on_decimal: Callable[[float], T],
        on_integer: Callable[[int], T],
        on_boolean: Callable[[bool], T],
        on_string: Callable[[str], T],
        on_datetime: Callable[[datetime], T],
    ) -> T:
        """Pattern match on the format data type and return result."""
        result, parsed_value = self.validate(value)
        if isinstance(result, Invalid):
            raise ValueError("Invalid value")
        if isinstance(parsed_value, DecimalValue):
            return on_decimal(parsed_value.value)
        if isinstance(parsed_value, IntegerValue):
            return on_integer(parsed_value.value)
        if isinstance(parsed_value, BooleanValue):
            return on_boolean(parsed_value.value)
        if isinstance(parsed_value, StringValue):
            return on_string(parsed_value.value)
        if isinstance(parsed_value, DateTimeValue):
            return on_datetime(parsed_value.value)
        raise ValueError("Invalid type")


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
    """ISO19848 implementation.

    Loads data channel type names and format data types from embedded resources.
    """

    LATEST_VERSION = ISO19848Version.V2024
    _instance: ISO19848 | None = None

    def __init__(self) -> None:
        """Initialize ISO19848."""
        self._data_channel_type_names_cache: dict[
            ISO19848Version, DataChannelTypeNames
        ] = {}
        self._format_data_types_cache: dict[ISO19848Version, FormatDataTypes] = {}

    def __new__(cls) -> ISO19848:
        """Ensure singleton instance."""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance

    def get_data_channel_type_names(
        self, version: ISO19848Version
    ) -> DataChannelTypeNames:
        """Get data channel type names for version."""
        if version not in self._data_channel_type_names_cache:
            dto = self._load_data_channel_type_names_dto(version)
            values = [DataChannelTypeName(x.type, x.description) for x in dto.values]
            self._data_channel_type_names_cache[version] = DataChannelTypeNames(values)
        return self._data_channel_type_names_cache[version]

    def get_format_data_types(self, version: ISO19848Version) -> FormatDataTypes:
        """Get format data types for version."""
        if version not in self._format_data_types_cache:
            dto = self._load_format_data_types_dto(version)
            values = [FormatDataType(x.type, x.description) for x in dto.values]
            self._format_data_types_cache[version] = FormatDataTypes(values)
        return self._format_data_types_cache[version]

    @staticmethod
    def _load_data_channel_type_names_dto(
        version: ISO19848Version,
    ) -> DataChannelTypeNamesDto:
        """Load data channel type names from embedded resource."""
        version_str = version.value  # e.g., "v2018" or "v2024"
        resource_name = f"iso19848-{version_str}-data-channel-type-names.json.gz"

        with (
            pkg_resources.path("vista_sdk.resources", resource_name) as resource_path,
            gzip.open(resource_path, "rt") as gzip_file,
        ):
            data = json.load(gzip_file)

        # Convert JSON to DTOs
        values = [
            DataChannelTypeNameDto(type=item["type"], description=item["description"])
            for item in data.get("values", [])
        ]
        return DataChannelTypeNamesDto(values=values)

    @staticmethod
    def _load_format_data_types_dto(version: ISO19848Version) -> FormatDataTypesDto:
        """Load format data types from embedded resource."""
        version_str = version.value  # e.g., "v2018" or "v2024"
        resource_name = f"iso19848-{version_str}-format-data-types.json.gz"

        with (
            pkg_resources.path("vista_sdk.resources", resource_name) as resource_path,
            gzip.open(resource_path, "rt") as gzip_file,
        ):
            data = json.load(gzip_file)

        # Convert JSON to DTOs
        values = [
            FormatDataTypeDto(type=item["type"], description=item["description"])
            for item in data.get("values", [])
        ]
        return FormatDataTypesDto(values=values)
