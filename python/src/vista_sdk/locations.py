"""Locations module for handling and parsing location codes in the VISTA SDK."""

from __future__ import annotations

from enum import Enum
from typing import overload

from python.src.vista_sdk.locations_dto import LocationsDto
from python.src.vista_sdk.vis_version import VisVersion

from .internal.location_parsing_error_builder import (
    LocationParsingErrorBuilder,
    LocationValidationResult,
    ParsingErrors,
)


class LocationGroup(Enum):
    """Enum representing different groups of location codes."""

    NUMBER = 0
    SIDE = 1
    VERTICAL = 2
    TRANSVERSE = 3
    LONGITUDINAL = 4


class Location:
    """Represents a location with a string value."""

    def __init__(self, value: str) -> None:
        """Initialize a Location instance with a string value."""
        self._value = value

    @property
    def value(self) -> str:
        return self._value

    def __str__(self) -> str:
        return self._value

    def __repr__(self) -> str:
        return f"Location(value={self._value!r})"

    @staticmethod
    def from_string(s: str) -> Location:
        return Location(s)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Location):
            return NotImplemented
        return self._value == other._value

    def __hash__(self) -> int:
        return hash(self._value)


class RelativeLocation:
    """Represents a relative location with a code, name, and optional definition."""

    def __init__(
        self, code: str, name: str, location: Location, definition: str | None
    ) -> None:
        self._code: str = code
        self._name: str = name
        self._location: Location = location
        self._definition: str | None = definition

    @property
    def code(self) -> str:
        return self._code

    @property
    def name(self) -> str:
        return self._name

    @property
    def location(self) -> Location:
        return self._location

    @property
    def definition(self) -> str | None:
        return self._definition

    def __hash__(self) -> int:
        return hash(self._code)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, RelativeLocation):
            return NotImplemented
        if isinstance(other, RelativeLocation):
            return self._code == other._code
        return NotImplemented


class Locations:
    """Handles parsing and validation of location codes in the VISTA SDK."""

    def __init__(self, version: VisVersion, dto: LocationsDto) -> None:
        """Initialize Locations with a version and DTO."""
        self.vis_version = version
        self._location_codes = [d.code for d in dto.items]
        self._relative_locations = []
        self._reversed_groups = {}
        groups: dict[LocationGroup, list[RelativeLocation]] = {}

        for relative_locations_dto in dto.items:
            relative_location = RelativeLocation(
                relative_locations_dto.code,
                relative_locations_dto.name,
                Location(str(relative_locations_dto.code)),
                relative_locations_dto.definition,
            )
            self._relative_locations.append(relative_location)

            key = self.determine_group_by_code(relative_locations_dto.code)
            if key not in groups:
                groups[key] = []
            groups[key].append(relative_location)
            if key != LocationGroup.NUMBER:
                self._reversed_groups[relative_locations_dto.code] = key

        self._groups: dict = {k: tuple(v) for k, v in groups.items()}

    def determine_group_by_code(self, code: str) -> LocationGroup:
        if code in "N":
            return LocationGroup.NUMBER
        if code in "PCS":
            return LocationGroup.SIDE
        if code in "UML":
            return LocationGroup.VERTICAL
        if code in "IO":
            return LocationGroup.TRANSVERSE
        if code in "FA":
            return LocationGroup.LONGITUDINAL
        raise Exception(f"Unsupported code: {code}")

    @property
    def relative_locations(self) -> list[RelativeLocation]:
        return self._relative_locations.copy()

    @property
    def groups(self) -> dict[LocationGroup, list[RelativeLocation]]:
        return self._groups

    def parse(self, location_str: str | None) -> Location:
        error_builder = LocationParsingErrorBuilder.create()
        location = None

        success, location = self.try_parse_internal(
            location_str if location_str else "", location_str, error_builder
        )
        if not success or not location:
            error_details = error_builder.build()
            raise ValueError(
                f"Invalid value for location: '{location_str}', errors: {error_details}"
            )

        return location

    @overload
    def try_parse(self, value: str | None) -> tuple[bool, Location | None]: ...
    @overload
    def try_parse(self, value: Location | None) -> tuple[bool, Location | None]: ...

    def try_parse(
        self, value: str | None | Location | None
    ) -> tuple[bool, Location | None]:
        if isinstance(value, Location):
            return True, value
        if isinstance(value, str):
            error_builder = LocationParsingErrorBuilder.create()
            location = None

            if (
                self.try_parse_internal(value if value else "", value, error_builder)
                and value
            ):
                location = Location(value)
            else:
                return False, None
            return location is not None, location
        raise ValueError("Invalid value for location")

    def try_parse_with_errors(
        self, value: str | None
    ) -> tuple[bool, Location | None, ParsingErrors]:
        error_builder = LocationParsingErrorBuilder.create()
        location = None
        result = self.try_parse_internal(value if value else "", value, error_builder)
        errors = error_builder.build()
        if result and value:
            location = Location(value)
        return result[0], location, errors

    def try_parse_internal(
        self,
        value: str,
        original_str: str | None,
        error_builder: LocationParsingErrorBuilder,
    ) -> tuple[bool, Location | None]:
        location = None

        if not value:
            error_builder.add_error(
                LocationValidationResult.NULL_OR_WHITE_SPACE,
                "Invalid location: contains only whitespace",
            )
            return False, location

        if value.isspace():
            error_builder.add_error(
                LocationValidationResult.NULL_OR_WHITE_SPACE,
                "Invalid location: contains only whitespace",
            )
            return False, location

        original_span = value
        prev_digit_index = None
        digit_start_index = None
        char_dict: dict[LocationGroup, str] = {}

        assert len(LocationGroup) == 5  # noqa : S101

        for i, ch in enumerate(value):
            if ch.isdigit():
                if digit_start_index is None and i != 0:
                    error_builder.add_error(
                        LocationValidationResult.INVALID,
                        "Invalid location: numeric location should start before"
                        "location code(s) in location:"
                        f"'{original_str or original_span}'",
                    )
                    return False, location
                if prev_digit_index is not None and prev_digit_index != (i - 1):
                    error_builder.add_error(
                        LocationValidationResult.INVALID,
                        "Invalid location: cannot have multiple separated"
                        f"digits in location: '{original_str or original_span}'",
                    )
                    return False, location
                if digit_start_index is None:
                    digit_start_index = i

                prev_digit_index = i
            else:
                group = self._reversed_groups.get(ch)
                if not group:
                    invalid_chars = ",".join(
                        {
                            c
                            for c in original_str or original_span
                            if not c.isdigit() and c not in self._location_codes
                        }
                    )
                    error_builder.add_error(
                        LocationValidationResult.INVALID_CODE,
                        f"Invalid location code: '{original_str or original_span}'"
                        f"with invalid location code(s): {invalid_chars}",
                    )
                    return False, location

                if group in char_dict and char_dict[group] != ch:
                    error_builder.add_error(
                        LocationValidationResult.INVALID,
                        f"Invalid location: Multiple '{group}' values."
                        " Got both '{char_dict[group]}' and '{ch}' in "
                        f"'{original_str or original_span}'",
                    )
                    return False, location
                char_dict[group] = ch

        location = Location(original_str or original_span)
        return True, location

    @staticmethod
    def try_parse_int(span: str, start: int, length: int) -> tuple[bool, int]:
        try:
            if start + length > len(span):
                return False, 0
            number = int(span[start : start + length])
            return True, number
        except ValueError:
            return False, 0


class LocationCodeError(Exception):
    """Exception raised for errors in LocationCharDict operations."""


class LocationCharDict:
    """A dictionary-like structure to store location codes by their group.

    This class maintains a fixed-size array of location codes, indexed by
    LocationGroup values. It provides dictionary-like access while ensuring
    type safety and bounds checking.
    """

    def __init__(self, size: int) -> None:
        """Initialize the dictionary with a specified size."""
        if size < 1:
            raise ValueError("Size must be at least 1")
        self._codes: list[str | None] = [None] * size

    def __getitem__(self, key: LocationGroup) -> str | None:
        """Get the location code for a given group."""
        index = key.value - 1
        if index >= len(self._codes):
            raise LocationCodeError(
                f"Location group {key.name} with value {key.value} "
                f"exceeds storage size of {len(self._codes)}"
            )
        return self._codes[index]

    def __setitem__(self, key: LocationGroup, new_value: str) -> None:
        """Set the location code for a given group."""
        index = key.value - 1
        if index >= len(self._codes):
            raise LocationCodeError(
                f"Location group {key.name} with value {key.value} "
                f"exceeds storage size of {len(self._codes)}"
            )
        self._codes[index] = new_value

    def try_add(self, key: LocationGroup, value: str) -> tuple[bool, str | None]:
        """Try to add a location code for a group if not already present."""
        current_value = self[key]
        if current_value is not None:
            return False, current_value
        self[key] = value
        return True, None

    def __len__(self) -> int:
        """Get the total size of the internal storage."""
        return len(self._codes)
