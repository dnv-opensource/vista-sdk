"""Data channel implementation for transport module."""

from __future__ import annotations

import re
from collections.abc import Callable, Iterable, Iterator
from datetime import datetime, timezone
from enum import Enum
from typing import Any, TypeVar

from vista_sdk.local_id import LocalId
from vista_sdk.result import Invalid, Ok, ValidateResult
from vista_sdk.transport.ship_id import ShipId
from vista_sdk.transport.value import (
    AnyValue,
    BooleanValue,
    DateTimeValue,
    DecimalValue,
    IntegerValue,
    StringValue,
)

T = TypeVar("T")


class DataChannelListPackage:
    """Package containing data channel list."""

    def __init__(self, package: Package) -> None:
        """Initialize with package."""
        self.package = package

    @property
    def data_channel_list(self) -> DataChannelList:
        """Get the data channel list."""
        return self.package.data_channel_list


class Package:
    """Package containing header and data channel list."""

    def __init__(self, header: Header, data_channel_list: DataChannelList) -> None:
        """Initialize package."""
        self.header = header
        self.data_channel_list = data_channel_list


class Header:
    """Header containing ship ID and configuration information."""

    def __init__(
        self,
        ship_id: ShipId,
        data_channel_list_id: ConfigurationReference,
        author: str | None,
        version_information: VersionInformation | None = None,
        date_created: datetime | None = None,
        custom_headers: dict[str, Any] | None = None,
    ) -> None:
        """Initialize header."""
        self.ship_id = ship_id
        self.data_channel_list_id = data_channel_list_id
        self.author = author
        self.version_information = version_information or VersionInformation()
        self.date_created = date_created or datetime.now(timezone.utc)
        self.custom_headers = custom_headers


class ConfigurationReference:
    """Reference to a configuration."""

    def __init__(
        self, config_id: str, timestamp: datetime, version: str | None = None
    ) -> None:
        """Initialize configuration reference."""
        self.id = config_id
        self.version = version
        self.timestamp = timestamp


class VersionInformation:
    """Version information for naming scheme."""

    ANNEX_C_NAMING_RULE = "dnv"
    ANNEX_C_NAMING_SCHEME_VERSION = "v2"

    def __init__(
        self,
        naming_rule: str | None = None,
        naming_scheme_version: str | None = None,
        reference_url: str | None = None,
    ) -> None:
        """Initialize version information."""
        self.naming_rule = naming_rule or self.ANNEX_C_NAMING_RULE
        self.naming_scheme_version = (
            naming_scheme_version or self.ANNEX_C_NAMING_SCHEME_VERSION
        )
        self.reference_url = reference_url or "https://docs.vista.dnv.com"


class DataChannelList:
    """Collection of data channels with lookup capabilities."""

    def __init__(self) -> None:
        """Initialize empty data channel list."""
        self._data_channels: list[DataChannel] = []
        self._short_id_map: dict[str, DataChannel] = {}
        self._local_id_map: dict[LocalId, DataChannel] = {}

    @property
    def data_channels(self) -> list[DataChannel]:
        """Get read-only list of data channels."""
        return self._data_channels.copy()

    def __len__(self) -> int:
        """Get number of data channels."""
        return len(self._data_channels)

    def try_get_by_short_id(self, short_id: str) -> tuple[bool, DataChannel | None]:
        """Try to get data channel by short ID."""
        data_channel = self._short_id_map.get(short_id)
        return data_channel is not None, data_channel

    def try_get_by_local_id(self, local_id: LocalId) -> tuple[bool, DataChannel | None]:
        """Try to get data channel by local ID."""
        data_channel = self._local_id_map.get(local_id)
        return data_channel is not None, data_channel

    def add(self, data_channel: DataChannel | Iterable[DataChannel]) -> None:
        """Add data channel(s) to the list."""
        if isinstance(data_channel, DataChannel):
            self._add_single(data_channel)
        else:
            self._add_multiple(data_channel)

    def _add_single(self, data_channel: DataChannel) -> None:
        """Add a single data channel."""
        local_id = data_channel.data_channel_id.local_id
        if local_id in self._local_id_map:
            raise ValueError(f"DataChannel with LocalId {local_id} already exists")

        short_id = data_channel.data_channel_id.short_id
        if short_id is not None:
            if short_id in self._short_id_map:
                raise ValueError(f"DataChannel with ShortId {short_id} already exists")
            self._short_id_map[short_id] = data_channel

        self._data_channels.append(data_channel)
        self._local_id_map[local_id] = data_channel

    def _add_multiple(self, data_channels: Iterable[DataChannel]) -> None:
        """Add multiple data channels."""
        for data_channel in data_channels:
            self._add_single(data_channel)

    def clear(self) -> None:
        """Clear all data channels."""
        self._data_channels.clear()
        self._short_id_map.clear()
        self._local_id_map.clear()

    def __contains__(self, item: DataChannel) -> bool:
        """Check if data channel is in the list."""
        return item in self._data_channels

    def remove(self, item: DataChannel) -> bool:
        """Remove data channel from the list."""
        local_id = item.data_channel_id.local_id
        if local_id not in self._local_id_map:
            return False

        short_id = item.data_channel_id.short_id
        if short_id is not None and short_id not in self._short_id_map:
            return False

        del self._local_id_map[local_id]
        if short_id is not None:
            del self._short_id_map[short_id]

        try:
            self._data_channels.remove(item)
            return True
        except ValueError:
            return False

    def __getitem__(self, key: str | int | LocalId) -> DataChannel:
        """Get data channel by key."""
        if isinstance(key, str):
            if key not in self._short_id_map:
                raise KeyError(f"No data channel with short ID '{key}'")
            return self._short_id_map[key]
        if isinstance(key, int):
            return self._data_channels[key]
        if isinstance(key, LocalId):
            if key not in self._local_id_map:
                raise KeyError(f"No data channel with local ID '{key}'")
            return self._local_id_map[key]
        raise TypeError(f"Invalid key type: {type(key)}")

    def __iter__(self) -> Iterator[DataChannel]:
        """Iterate over data channels."""
        return iter(self._data_channels)


class DataChannel:
    """Represents a data channel with ID and property."""

    def __init__(self, data_channel_id: DataChannelId, prop: Property) -> None:
        """Initialize data channel."""
        self.data_channel_id = data_channel_id
        self._property = None
        self.prop = prop  # This will trigger validation

    @property
    def prop(self) -> Property:
        """Get the property."""
        if self._property is None:
            raise ValueError("Property not set")
        return self._property

    @prop.setter
    def prop(self, value: Property) -> None:
        """Set the property with validation."""
        validation_result = value.validate()
        if isinstance(validation_result, Invalid):
            messages = ", ".join(validation_result.messages)
            raise ValueError(f"Invalid property - Messages='[{messages}]'")
        self._property = value


class DataChannelId:
    """Data channel identifier with local ID and optional short ID."""

    def __init__(
        self,
        local_id: LocalId,
        short_id: str | None,
        name_object: NameObject | None = None,
    ) -> None:
        """Initialize data channel ID."""
        self.local_id = local_id
        self.short_id = short_id
        self.name_object = name_object or NameObject()


class NameObject:
    """Name object for data channel."""

    ANNEX_C_NAMING_RULE = (
        f"/{VersionInformation.ANNEX_C_NAMING_RULE}-"
        f"{VersionInformation.ANNEX_C_NAMING_SCHEME_VERSION}"
    )

    def __init__(
        self,
        naming_rule: str | None = None,
        custom_name_objects: dict[str, Any] | None = None,
    ) -> None:
        """Initialize name object."""
        self.naming_rule = naming_rule or self.ANNEX_C_NAMING_RULE
        self.custom_name_objects = custom_name_objects


class Property:
    """Property containing data channel type, format, range, and unit."""

    def __init__(
        self,
        data_channel_type: DataChannelType,
        data_format: Format,
        data_range: Range | None,
        unit: Unit | None,
        quality_coding: str | None = None,
        alert_priority: str | None = None,
        name: str | None = None,
        remarks: str | None = None,
        custom_properties: dict[str, Any] | None = None,
    ) -> None:
        """Initialize property."""
        self.data_channel_type = data_channel_type
        self.format = data_format
        self.range = data_range
        self.unit = unit
        self.quality_coding = quality_coding
        self.alert_priority = alert_priority
        self.name = name
        self.remarks = remarks
        self.custom_properties = custom_properties

    def validate(self) -> ValidateResult:
        """Validate the property."""
        messages = []

        if self.format.is_decimal and self.range is None:
            messages.append("Range is required for Decimal format type")
        if self.format.is_decimal and self.unit is None:
            messages.append("Unit is required for Decimal format type")
        if self.data_channel_type.is_alert and self.alert_priority is None:
            messages.append("AlertPriority is required for Alert DataChannelType")

        if messages:
            return Invalid(messages)
        return Ok()


class DataChannelType:
    """Data channel type with validation."""

    def __init__(
        self,
        channel_type: str,
        update_cycle: float | None = None,
        calculation_period: float | None = None,
    ) -> None:
        """Initialize data channel type."""
        self._type: str | None = None
        self._update_cycle: float | None = None
        self._calculation_period: float | None = None

        self.type = channel_type
        if update_cycle is not None:
            self.update_cycle = update_cycle
        if calculation_period is not None:
            self.calculation_period = calculation_period

    @property
    def type(self) -> str:
        """Get the data channel type."""
        if self._type is None:
            raise ValueError("DataChannelType not set")
        return self._type

    @type.setter
    def type(self, value: str) -> None:
        """Set the data channel type with validation."""
        # Note: In the C# version, this validates against ISO19848
        # data channel type names
        # For now, we'll do basic validation and can enhance later
        if not value:
            raise ValueError(f"Invalid data channel type {value}")
        self._type = value

    @property
    def update_cycle(self) -> float | None:
        """Get the update cycle."""
        return self._update_cycle

    @update_cycle.setter
    def update_cycle(self, value: float | None) -> None:
        """Set the update cycle with validation."""
        if value is not None and value < 0:
            raise ValueError(f"Invalid update cycle {value}. Should be positive")
        self._update_cycle = value

    @property
    def calculation_period(self) -> float | None:
        """Get the calculation period."""
        return self._calculation_period

    @calculation_period.setter
    def calculation_period(self, value: float | None) -> None:
        """Set the calculation period with validation."""
        if value is not None and value < 0:
            raise ValueError(f"Invalid calculation period {value}. Should be positive")
        self._calculation_period = value

    @property
    def is_alert(self) -> bool:
        """Check if this is an alert type."""
        return self.type == "Alert"


class Format:
    """Format specification with data type and restrictions."""

    def __init__(
        self, format_type: str, restriction: Restriction | None = None
    ) -> None:
        """Initialize format."""
        self._type: str | None = None
        self._data_type = None
        self.restriction = restriction
        self.type = format_type

    @property
    def type(self) -> str:
        """Get the format type."""
        if self._type is None:
            raise ValueError("Format type not set")
        return self._type

    @type.setter
    def type(self, value: str) -> None:
        """Set the format type with validation."""
        # Note: In the C# version, this validates against ISO19848 format data types
        # For now, we'll do basic validation and can enhance later
        if not value:
            raise ValueError(f"Invalid format type {value}")
        self._type = value
        # For now, we'll create a simple data type object
        self._data_type = FormatDataType(value)

    @property
    def data_type(self) -> FormatDataType:
        """Get the data type."""
        if self._data_type is None:
            raise ValueError("Format type not set")
        return self._data_type

    @property
    def is_decimal(self) -> bool:
        """Check if this is a decimal format."""
        return self.type == "Decimal"

    def validate_value(self, value: str) -> tuple[ValidateResult, AnyValue | None]:
        """Validate a value against this format."""
        result, parsed_value = self.data_type.validate(value)
        if isinstance(result, Invalid):
            return result, None

        if self.restriction is not None:
            restriction_result = self.restriction.validate_value(value, self)
            if isinstance(restriction_result, Invalid):
                return restriction_result, None

        return Ok(), parsed_value


class FormatDataType:
    """Represents a format data type with validation."""

    def __init__(self, type_name: str) -> None:
        """Initialize format data type."""
        self.type = type_name

    def validate(self, value: str) -> tuple[ValidateResult, AnyValue | None]:
        """Validate and parse a string value, returning both result and parsed value."""
        if self.type == "Decimal":
            try:
                return Ok(), DecimalValue(float(value))
            except ValueError:
                return Invalid([f"Invalid decimal value - Value='{value}'"]), None
        if self.type == "Integer":
            try:
                return Ok(), IntegerValue(int(value))
            except ValueError:
                return Invalid([f"Invalid integer value - Value='{value}'"]), None
        if self.type == "Boolean":
            try:
                return Ok(), BooleanValue(value.lower() in ("true", "false"))
            except ValueError:
                return Invalid([f"Invalid boolean value - Value='{value}'"]), None
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


class Restriction:
    """Validation restrictions for format values."""

    def __init__(
        self,
        enumeration: list[str] | None = None,
        fraction_digits: int | None = None,
        length: int | None = None,
        max_exclusive: float | None = None,
        max_inclusive: float | None = None,
        max_length: int | None = None,
        min_exclusive: float | None = None,
        min_inclusive: float | None = None,
        min_length: int | None = None,
        pattern: str | None = None,
        total_digits: int | None = None,
        white_space: WhiteSpace | None = None,
    ) -> None:
        """Initialize restriction."""
        self.enumeration = enumeration
        self.fraction_digits = fraction_digits
        self.length = length
        self.max_exclusive = max_exclusive
        self.max_inclusive = max_inclusive
        self.max_length = max_length
        self.min_exclusive = min_exclusive
        self.min_inclusive = min_inclusive
        self.min_length = min_length
        self.pattern = pattern
        self.white_space = white_space
        self._total_digits = None
        if total_digits is not None:
            self.total_digits = total_digits

    @property
    def total_digits(self) -> int | None:
        """Get total digits constraint."""
        return self._total_digits

    @total_digits.setter
    def total_digits(self, value: int | None) -> None:
        """Set total digits constraint with validation."""
        if value is not None and value <= 0:
            raise ValueError("TotalDigits should be greater than zero")
        self._total_digits = value

    def validate_value(self, value: str, format_obj: Format) -> ValidateResult:
        """Validate a value against the restrictions."""
        if self.enumeration is not None and value not in self.enumeration:
            return Invalid([f"Value {value} is not in the enumeration"])

        # Handle different data types
        if format_obj.type == "Decimal":
            return self._validate_decimal(value)
        if format_obj.type == "Integer":
            return self._validate_integer(value)
        if format_obj.type == "String":
            return self._validate_string(value)

        return Ok()

    def _validate_decimal(self, value: str) -> ValidateResult:
        """Validate decimal value against restrictions."""
        try:
            dec_value = float(value)
            if self.fraction_digits is not None:
                decimal_places = self._count_decimal_places(value)
                if decimal_places > self.fraction_digits:
                    return Invalid(["Value has more decimal places than allowed"])
            return self._validate_number(dec_value)
        except ValueError:
            return Invalid([f"Invalid decimal value: {value}"])

    def _validate_integer(self, value: str) -> ValidateResult:
        """Validate integer value against restrictions."""
        try:
            int_value = int(value)
            validation_result = self._validate_number(float(int_value))
            if isinstance(validation_result, Invalid):
                return validation_result
            if self.total_digits is not None:
                num_digits = len(str(abs(int_value)))
                if num_digits != self.total_digits:
                    return Invalid(
                        [
                            f"Value {int_value} has {num_digits} digits "
                            f"but should be {self.total_digits}"
                        ]
                    )
            return Ok()
        except ValueError:
            return Invalid([f"Invalid integer value: {value}"])

    def _validate_string(self, value: str) -> ValidateResult:
        """Validate string value against restrictions."""
        length = len(value)
        if self.length is not None and length != self.length:
            return Invalid(
                [f"Value {value} has length {length} but should be {self.length}"]
            )
        if self.max_length is not None and length > self.max_length:
            return Invalid(
                [
                    f"Value {value} has length {length} "
                    f"but should be less than {self.max_length}"
                ]
            )
        if self.min_length is not None and length < self.min_length:
            return Invalid(
                [
                    f"Value {value} has length {length} "
                    f"but should be greater than {self.min_length}"
                ]
            )
        if self.pattern is not None and not re.match(self.pattern, value):
            return Invalid([f"Value {value} does not match pattern {self.pattern}"])

        return Ok()

    def _validate_number(self, number: float) -> ValidateResult:
        """Validate numeric constraints."""
        if self.max_exclusive is not None and number >= self.max_exclusive:
            return Invalid(
                [f"Value {number} is greater than or equal to {self.max_exclusive}"]
            )
        if self.max_inclusive is not None and number > self.max_inclusive:
            return Invalid([f"Value {number} is greater than {self.max_inclusive}"])
        if self.min_exclusive is not None and number <= self.min_exclusive:
            return Invalid(
                [f"Value {number} is less than or equal to {self.min_exclusive}"]
            )
        if self.min_inclusive is not None and number < self.min_inclusive:
            return Invalid([f"Value {number} is less than {self.min_inclusive}"])
        return Ok()

    def _count_decimal_places(self, value: str) -> int:
        """Count decimal places in a string representation of a number."""
        if "." not in value:
            return 0
        return len(value.split(".")[1])


class WhiteSpace(Enum):
    """White space handling enumeration."""

    PRESERVE = 0
    REPLACE = 1
    COLLAPSE = 2


class Range:
    """Numeric range specification."""

    def __init__(self, low: float, high: float) -> None:
        """Initialize range."""
        if low > high:
            raise ValueError(f"Low value {low} should be less than high value {high}")
        self._low = low
        self._high = high

    @property
    def low(self) -> float:
        """Get the low value."""
        return self._low

    @low.setter
    def low(self, value: float) -> None:
        """Set the low value with validation."""
        if value > self._high:
            raise ValueError(
                f"Low value {value} should be less than high value {self._high}"
            )
        self._low = value

    @property
    def high(self) -> float:
        """Get the high value."""
        return self._high

    @high.setter
    def high(self, value: float) -> None:
        """Set the high value with validation."""
        if value < self._low:
            raise ValueError(
                f"High value {value} should be greater than low value {self._low}"
            )
        self._high = value


class Unit:
    """Unit specification."""

    def __init__(
        self,
        unit_symbol: str,
        quantity_name: str | None = None,
        custom_elements: dict[str, Any] | None = None,
    ) -> None:
        """Initialize unit."""
        self.unit_symbol = unit_symbol
        self.quantity_name = quantity_name
        self.custom_elements = custom_elements
