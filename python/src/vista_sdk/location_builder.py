"""Locations module for handling and parsing location codes in the VISTA SDK."""

from __future__ import annotations

from copy import copy
from typing import TypeVar
from xml.dom import ValidationErr

from vista_sdk.locations import Location, LocationGroup, Locations

T = TypeVar("T")


class LocationBuilder:
    """Builder for constructing Location objects in the VISTA SDK."""

    def __init__(self, locations: Locations) -> None:
        """Initialize the LocationBuilder with a Locations instance."""
        self.vis_version = locations.vis_version
        self.reversed_groups = locations._reversed_groups
        self.number: int | None = None
        self.side: str | None = None
        self.vertical: str | None = None
        self.transverse: str | None = None
        self.longitudinal: str | None = None

    @staticmethod
    def create(locations: Locations) -> LocationBuilder:
        """Create a LocationBuilder instance from a Locations object."""
        return LocationBuilder(locations)

    def with_location(self, value: Location) -> LocationBuilder:
        """Construct a LocationBuilder from an existing Location object."""
        builder = copy(self)
        span = str(value.__str__())
        n: int | None = None

        for i, ch in enumerate(span):
            if ch.isdigit():
                if n is None:
                    n = int(ch)
                else:
                    num = Locations.try_parse_int(span, 0, i + 1)
                    if num[0] is False:
                        raise ValidationErr("Should include a valid number")
                    n = num[1]
                continue

            builder = builder.with_value_char(ch)

        if n is not None:
            builder = builder.with_number(n)

        return builder

    def with_number(self, number: int) -> LocationBuilder:
        """Set the number for the location."""
        return self.with_value_internal(LocationGroup.NUMBER, number)

    def with_side(self, side: str) -> LocationBuilder:
        """Set the side for the location."""
        return self.with_value_internal(LocationGroup.SIDE, side)

    def with_vertical(self, vertical: str) -> LocationBuilder:
        """Set the vertical for the location."""
        return self.with_value_internal(LocationGroup.VERTICAL, vertical)

    def with_transverse(self, transverse: str) -> LocationBuilder:
        """Set the transverse for the location."""
        return self.with_value_internal(LocationGroup.TRANSVERSE, transverse)

    def with_longitudinal(self, longitudinal: str) -> LocationBuilder:
        """Set the longitudinal for the location."""
        return self.with_value_internal(LocationGroup.LONGITUDINAL, longitudinal)

    def with_value(self, value: int) -> LocationBuilder:
        """Set the value for the location, which can be a number."""
        return self.with_value_internal(LocationGroup.NUMBER, value)

    def with_value_char(self, value: str) -> LocationBuilder:
        """Set the value for the location, which can be a single character."""
        if value not in self.reversed_groups:
            raise ValueError(f"The value {value} is an invalid Locations value")
        group = self.reversed_groups[value]
        return self.with_value_internal(group, value)

    def with_value_internal(
        self, group: LocationGroup, value: int | str | None
    ) -> LocationBuilder:
        """Set the value for the location based on the group and value type."""
        if group == LocationGroup.NUMBER:
            if not isinstance(value, int):
                raise ValueError("Value should be a number")
            if value < 1:
                raise ValueError("Number must be greater than 0")
            builder = copy(self)
            builder.number = value
            return builder

        if not isinstance(value, str) or len(value) != 1:
            raise ValueError("Value should be a single character")

        if value not in self.reversed_groups or self.reversed_groups[value] != group:
            raise ValueError(f"The value {value} is an invalid {group.name} value")

        builder = copy(self)
        setattr(builder, group.name.lower(), value)
        return builder

    def without_value(self, group: LocationGroup) -> LocationBuilder:
        """Remove a value from the location based on the group."""
        builder = copy(self)
        setattr(builder, group.name.lower(), None)
        return builder

    def build(self) -> Location:
        """Construct and return a Location object from the builder."""
        return Location(self.__str__())

    def __str__(self) -> str:
        """Return a string representation of the location."""
        parts = [self.side, self.vertical, self.transverse, self.longitudinal]
        parts = [p for p in parts if p is not None]
        if self.number:
            parts.insert(0, str(self.number))
        return "".join(sorted([p for p in parts if p is not None]))

    def without_number(self) -> LocationBuilder:
        """Remove the number from the location."""
        return self.without_value(LocationGroup.NUMBER)

    def without_side(self) -> LocationBuilder:
        """Remove the side from the location."""
        return self.without_value(LocationGroup.SIDE)

    def without_vertical(self) -> LocationBuilder:
        """Remove the vertical from the location."""
        return self.without_value(LocationGroup.VERTICAL)

    def without_transverse(self) -> LocationBuilder:
        """Remove the transverse from the location."""
        return self.without_value(LocationGroup.TRANSVERSE)

    def without_longitudinal(self) -> LocationBuilder:
        """Remove the longitudinal from the location."""
        return self.without_value(LocationGroup.LONGITUDINAL)
