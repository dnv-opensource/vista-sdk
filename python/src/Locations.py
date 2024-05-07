from dataclasses import dataclass, field
from enum import Enum
from typing import Dict, List, Optional, Tuple

from .LocationsDto import LocationsDto, RelativeLocationsDto
from .internal.LocationParsingErrorBuilder import LocationParsingErrorBuilder

class LocationGroup(Enum):
    NUMBER = 0
    SIDE = 1
    VERTICAL = 2
    TRANSVERSE = 3
    LONGITUDINAL = 4

class Location:
    def __init__(self, value):
        self._value = value

    @property
    def value(self):
        return self._value

    def __str__(self):
        return self._value

    def __repr__(self):
        return f"Location(value={self._value!r})"

    @staticmethod
    def from_string(s):
        return Location(s)

class RelativeLocation:
    def __init__(self, code, name, location, definition=None):
        self._code = code
        self._name = name
        self._location = location
        self._definition = definition

    @property
    def code(self):
        return self._code

    @property
    def name(self):
        return self._name

    @property
    def location(self):
        return self._location

    @property
    def definition(self):
        return self._definition

    def __hash__(self):
        return hash(self._code)

    def __eq__(self, other):
        if isinstance(other, RelativeLocation):
            return self._code == other._code
        return NotImplemented


class LocationValidationResult(Enum):
    INVALID = 1
    INVALID_CODE = 2
    INVALID_ORDER = 3
    NULL_OR_WHITE_SPACE = 4
    VALID = 5

class Locations:
    def __init__(self, version, dto):
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

        self._groups = {k: tuple(v) for k, v in groups.items()}

    def determine_group_by_code(self, code):
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
    def relative_locations(self):
        return self._relative_locations.copy()

    @property
    def groups(self):
        return self._groups
    
    def parse_location(self, location_str):
        error_builder = LocationParsingErrorBuilder.create()
        location = None

        success, location = self.try_parse_internal(location_str if location_str else "", location_str, error_builder)
        if not success:
            error_details = error_builder.build()
            raise ValueError(f"Invalid value for location: '{location_str}', errors: {error_details}")

        return location

    def try_parse(self, value):
        error_builder = LocationParsingErrorBuilder.create()
        location = None

        if self.try_parse_internal(value if value else "", value, error_builder):
            location = Location(value)

        return location is not None, location

    def try_parse_with_errors(self, value):
        error_builder = LocationParsingErrorBuilder.create()
        location = None
        result = self.try_parse_internal(value if value else "", value, error_builder)
        errors = error_builder.build()
        if result:
            location = Location(value)
        return result, location, errors
    

    def try_parse_internal(self, span, original_str, error_builder):
        location = None
        print("Span:", span)
        print("Original String:", original_str)
        print("Errors:", error_builder.build())

        if not span:
            error_builder.add_error("NullOrWhiteSpace", "Invalid location: contains only whitespace")
            return False, location

        if span.isspace():
            error_builder.add_error("NullOrWhiteSpace", "Invalid location: contains only whitespace")
            return False, location

        original_span = span
        prev_digit_index = None
        digit_start_index = None
        number = None
        char_dict = {}

        assert len(LocationGroup) == 5

        for i, ch in enumerate(span):
            if ch.isdigit():
                if digit_start_index is None and i != 0:
                    error_builder.add_error("Invalid", f"Invalid location: numeric location should start before location code(s) in location: '{original_str or original_span}'")
                    return False, location
                if prev_digit_index is not None and prev_digit_index != (i - 1):
                    error_builder.add_error("Invalid", f"Invalid location: cannot have multiple separated digits in location: '{original_str or original_span}'")
                    return False, location
                if digit_start_index is None:
                    number = int(ch)
                    digit_start_index = i
                else:
                    num = int(span[digit_start_index:i+1])
                    number = num

                prev_digit_index = i
            else:
                group = self._reversed_groups.get(ch) 
                if not group:
                    invalid_chars = ','.join(set(c for c in original_str or original_span if not c.isdigit() and c not in self._location_codes))
                    error_builder.add_error("InvalidCode", f"Invalid location code: '{original_str or original_span}' with invalid location code(s): {invalid_chars}")
                    return False, location

                if group in char_dict and char_dict[group] != ch:
                    error_builder.add_error("Invalid", f"Invalid location: Multiple '{group}' values. Got both '{char_dict[group]}' and '{ch}' in '{original_str or original_span}'")
                    return False, location
                char_dict[group] = ch

        location = original_str or original_span
        return True, location
    
    def try_parse_int(self, span, start, length):
        try:
            if start + length > len(span):
                return False, 0
            number = int(span[start:start + length])
            return True, number
        except ValueError:
            return False, 0
        

class LocationCharDict:
    def __init__(self, size):
        self._table = [None] * size

    def __getitem__(self, key):
        index = key.value - 1 
        if index >= len(self._table):
            raise Exception(f"Unsupported code: {key}")
        return self._table[index]

    def __setitem__(self, key, value):
        index = key.value - 1
        if index >= len(self._table):
            raise Exception(f"Unsupported code: {key}")
        self._table[index] = value

    def try_add(self, key, value):
        current_value = self[key]
        if current_value is not None:
            return False, current_value
        self[key] = value
        return True, None


    


