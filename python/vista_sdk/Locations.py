from __future__ import annotations
from enum import Enum
from typing import Dict, List, Optional, Tuple, overload
from vista_sdk.LocationsDto import LocationsDto
from vista_sdk.VisVersions import VisVersion
from .internal.LocationParsingErrorBuilder import LocationParsingErrorBuilder, LocationValidationResult, ParsingErrors


class LocationGroup(Enum):
    NUMBER = 0
    SIDE = 1
    VERTICAL = 2
    TRANSVERSE = 3
    LONGITUDINAL = 4

class Location:
    def __init__(self, value :str):
        self._value = value

    @property
    def value(self) -> str:
        return self._value

    def __str__(self) -> str:
        return self._value

    def __repr__(self) -> str:
        return f"Location(value={self._value!r})"

    @staticmethod
    def from_string(s : str) -> Location:
        return Location(s)

    def __eq__(self, other: Location) -> bool:
        return self._value == other._value
    
    def __hash__(self) -> int:
        return hash(self._value)

class RelativeLocation:
    def __init__(self, code : str, name : str, location : Location, definition : Optional[str] =None):
        self._code : str = code
        self._name : str = name
        self._location : Location = location
        self._definition : Optional[str] = definition

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
    def definition(self) -> Optional[str]:
        return self._definition

    def __hash__(self) -> int:
        return hash(self._code)

    def __eq__(self, other : RelativeLocation) -> bool:
        if isinstance(other, RelativeLocation):
            return self._code == other._code
        return NotImplemented


class Locations:
    def __init__(self, version : VisVersion, dto : LocationsDto) :
        self.vis_version = version
        self._location_codes = [d.code for d in dto.items]
        self._relative_locations = []
        self._reversed_groups = {}
        groups = {}

        for relative_locations_dto in dto.items:
            relative_location = RelativeLocation(
                relative_locations_dto.code,
                relative_locations_dto.name,
                Location(str(relative_locations_dto.code)),
                relative_locations_dto.definition
            )
            self._relative_locations.append(relative_location)

            key = self.determine_group_by_code(relative_locations_dto.code)
            if key not in groups:
                groups[key] = []
            groups[key].append(relative_location)
            if key != LocationGroup.NUMBER:
                self._reversed_groups[relative_locations_dto.code] = key

        self._groups : Dict = {k: tuple(v) for k, v in groups.items()}

    def determine_group_by_code(self, code : str) -> LocationGroup:
        if code in 'N':
            return LocationGroup.NUMBER
        elif code in 'PCS':
            return LocationGroup.SIDE
        elif code in 'UML':
            return LocationGroup.VERTICAL
        elif code in 'IO':
            return LocationGroup.TRANSVERSE
        elif code in 'FA':
            return LocationGroup.LONGITUDINAL
        else:
            raise Exception(f"Unsupported code: {code}")

    @property
    def relative_locations(self) -> List[RelativeLocation]:
        return self._relative_locations.copy()

    @property
    def groups(self) -> Dict[LocationGroup, List[RelativeLocation]]:
        return self._groups
    
    def parse(self, location_str : Optional[str]) -> Location:
        error_builder = LocationParsingErrorBuilder.create()
        location = None

        success, location = self.try_parse_internal(location_str if location_str else "", location_str, error_builder)
        if not success or not location:
            error_details = error_builder.build()
            raise ValueError(f"Invalid value for location: '{location_str}', errors: {error_details}")

        return location
    
    @overload
    def try_parse(self, value : Optional[str]) -> Tuple[bool, Optional[Location]]:
        ...
    @overload
    def try_parse(self, value : Optional[Location]) -> Tuple[bool, Optional[Location]]:
        ...

    def try_parse(self, value : Optional[str] | Optional[Location]) -> Tuple[bool, Optional[Location]]:
        if isinstance(value, Location):
            return True, value
        if isinstance(value, str):
            error_builder = LocationParsingErrorBuilder.create()
            location = None

            if self.try_parse_internal(value if value else "", value, error_builder) and value:
                location = Location(value)
            else: 
                return False, None
            return location is not None, location
        else: 
            raise ValueError("Invalid value for location")

    def try_parse_with_errors(self, value: Optional[str]) -> Tuple[bool, Optional[Location], ParsingErrors]:
        error_builder = LocationParsingErrorBuilder.create()
        location = None
        result = self.try_parse_internal(value if value else "", value, error_builder)
        errors = error_builder.build()
        if result and value:
            location = Location(value)
        return result[0], location, errors
    

    def try_parse_internal(self, value : str, original_str : Optional[str], error_builder : LocationParsingErrorBuilder) -> Tuple[bool, Optional[Location]]:
        location = None
        
        if(value is not type(str)):
            test = value

        if not value:
            error_builder.add_error(LocationValidationResult.NULL_OR_WHITE_SPACE, "Invalid location: contains only whitespace")
            return False, location

        if value.isspace():
            error_builder.add_error(LocationValidationResult.NULL_OR_WHITE_SPACE, "Invalid location: contains only whitespace")
            return False, location

        original_span = value
        prev_digit_index = None
        digit_start_index = None
        number = None
        char_dict = {}

        assert len(LocationGroup) == 5

        for i, ch in enumerate(value):
            if ch.isdigit():
                if digit_start_index is None and i != 0:
                    error_builder.add_error(LocationValidationResult.INVALID, f"Invalid location: numeric location should start before location code(s) in location: '{original_str or original_span}'")
                    return False, location
                if prev_digit_index is not None and prev_digit_index != (i - 1):
                    error_builder.add_error(LocationValidationResult.INVALID, f"Invalid location: cannot have multiple separated digits in location: '{original_str or original_span}'")
                    return False, location
                if digit_start_index is None:
                    number = int(ch)
                    digit_start_index = i
                else:
                    num = int(value[digit_start_index:i+1])
                    number = num

                prev_digit_index = i
            else:
                group = self._reversed_groups.get(ch) 
                if not group:
                    invalid_chars = ','.join(set(c for c in original_str or original_span if not c.isdigit() and c not in self._location_codes))
                    error_builder.add_error(LocationValidationResult.INVALID_CODE, f"Invalid location code: '{original_str or original_span}' with invalid location code(s): {invalid_chars}")
                    return False, location

                if group in char_dict and char_dict[group] != ch:
                    error_builder.add_error(LocationValidationResult.INVALID, f"Invalid location: Multiple '{group}' values. Got both '{char_dict[group]}' and '{ch}' in '{original_str or original_span}'")
                    return False, location
                char_dict[group] = ch

        location = Location(original_str or original_span)
        return True, location
    
    @staticmethod
    def try_parse_int(span : str, start : int, length : int) -> Tuple[bool, int]:
        try:
            if start + length > len(span):
                return False, 0
            number = int(span[start:start + length])
            return True, number
        except ValueError:
            return False, 0
        

class LocationCharDict:
    def __init__(self, size : int):
        self._table = [None] * size

    def __getitem__(self, key : LocationGroup) -> Optional[str]:
        index = key.value - 1 
        if index >= len(self._table):
            raise Exception(f"Unsupported code: {key}")
        return self._table[index]

    def __setitem__(self, key : LocationGroup, value) -> None:
        index = key.value - 1
        if index >= len(self._table):
            raise Exception(f"Unsupported code: {key}")
        self._table[index] = value

    def try_add(self, key : LocationGroup, value : str) -> Tuple[bool, Optional[str]]:
        current_value = self[key]
        if current_value is not None:
            return False, current_value
        self[key] = value
        return True, None


    


