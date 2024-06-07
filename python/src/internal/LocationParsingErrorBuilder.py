from __future__ import annotations
from enum import Enum

class LocationValidationResult(Enum):
    INVALID = 1
    INVALID_CODE = 2
    INVALID_ORDER = 3
    NULL_OR_WHITE_SPACE = 4
    VALID = 5

class LocationParsingErrorBuilder:
    def __init__(self):
        self._errors = []


    def add_error(self, name : LocationValidationResult, message : str) -> LocationParsingErrorBuilder:
        self._errors.append((name, message))
        return self

    @property
    def has_error(self) -> bool:
        return len(self._errors) > 0

    @staticmethod
    def create() -> LocationParsingErrorBuilder:
        return LocationParsingErrorBuilder()

    def build(self) -> ParsingErrors:
        return ParsingErrors(self._errors) if self._errors else ParsingErrors()

class ParsingErrors:
    empty = []

    def __init__(self, errors : list = []):
        self.errors = [(error[0], error[1]) for error in errors]

    def __repr__(self):
        return f"ParsingErrors({self.errors})"
    

