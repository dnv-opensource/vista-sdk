from __future__ import annotations
from typing import Optional, TypeVar
from xml.dom import ValidationErr
from vista_sdk.Locations import Location, LocationGroup, Locations
from copy import copy as copy

T = TypeVar('T')

class LocationBuilder:
    def __init__(self, locations : Locations):
        self.vis_version = locations.vis_version
        self.reversed_groups = locations._reversed_groups
        self.number = None
        self.side = None
        self.vertical = None
        self.transverse = None
        self.longitudinal = None

    @staticmethod
    def create(locations : Locations) -> LocationBuilder:
        return LocationBuilder(locations)

    def with_location(self, value: Location) -> LocationBuilder:
        builder = copy(self)
        span = str(value.__str__())
        n: Optional[int] = None

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

    def with_number(self, number :int) -> LocationBuilder:
        return self.with_value_internal(LocationGroup.NUMBER, number)

    def with_side(self, side : str) -> LocationBuilder:
        return self.with_value_internal(LocationGroup.SIDE, side)
    
    def with_vertical(self, vertical : str) -> LocationBuilder:
        return self.with_value_internal(LocationGroup.VERTICAL, vertical)

    def with_transverse(self, transverse : str) -> LocationBuilder:
        return self.with_value_internal(LocationGroup.TRANSVERSE, transverse)

    def with_longitudinal(self, longitudinal : str) -> LocationBuilder: 
        return self.with_value_internal(LocationGroup.LONGITUDINAL, longitudinal)

    def with_value(self, value : int) -> LocationBuilder:
        return self.with_value_internal(LocationGroup.NUMBER, value)
    
    def with_value_char(self, value : str) -> LocationBuilder:
        if value not in self.reversed_groups:
            raise ValueError(f"The value {value} is an invalid Locations value")
        group = self.reversed_groups[value]
        return self.with_value_internal(group, value)

    def with_value_internal(self, group : LocationGroup, value : T) -> LocationBuilder:
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

    def without_value(self, group : LocationGroup) -> LocationBuilder:
        builder = copy(self)
        setattr(builder, group.name.lower(), None)
        return builder

    def build(self) -> Location:
        return Location(self.__str__())

    def __str__(self) -> str:
        parts = [self.side, self.vertical, self.transverse, self.longitudinal]
        parts = [str(p) for p in parts if p is not None]
        if self.number:
            parts.insert(0, str(self.number))
        return ''.join(sorted(parts))
    
    def without_number(self) -> LocationBuilder:
        return self.without_value(LocationGroup.NUMBER)

    def without_side(self) -> LocationBuilder:
        return self.without_value(LocationGroup.SIDE)

    def without_vertical(self) -> LocationBuilder:
        return self.without_value(LocationGroup.VERTICAL)

    def without_transverse(self) -> LocationBuilder:
        return self.without_value(LocationGroup.TRANSVERSE)

    def without_longitudinal(self) -> LocationBuilder:
        return self.without_value(LocationGroup.LONGITUDINAL)