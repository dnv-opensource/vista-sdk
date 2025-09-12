"""This module provides a builder for parsing errors related to location validation."""

from __future__ import annotations

from enum import Enum
from typing import ClassVar


class LocationValidationResult(Enum):
    """Enum representing the result of location validation."""

    INVALID = 1
    INVALID_CODE = 2
    INVALID_ORDER = 3
    NULL_OR_WHITE_SPACE = 4
    VALID = 5


class LocationParsingErrorBuilder:
    """Builder for constructing parsing errors related to location validation."""

    def __init__(self) -> None:
        """Initialize the LocationParsingErrorBuilder with an empty error list."""
        self._errors: list[tuple[LocationValidationResult, str]] = []

    def add_error(
        self, name: LocationValidationResult, message: str
    ) -> LocationParsingErrorBuilder:
        """Add an error to the builder."""
        self._errors.append((name, message))
        return self

    @property
    def has_error(self) -> bool:
        """Check if there are any errors in the builder."""
        return len(self._errors) > 0

    @staticmethod
    def create() -> LocationParsingErrorBuilder:
        """Create a new instance of LocationParsingErrorBuilder."""
        return LocationParsingErrorBuilder()

    def build(self) -> ParsingErrors:
        """Build and return the ParsingErrors object."""
        return ParsingErrors(self._errors) if self._errors else ParsingErrors()


class ParsingErrors:
    """Data structure representing parsing errors related to location validation."""

    empty: ClassVar[list[tuple[LocationValidationResult, str]]] = []

    def __init__(self, errors: list = []) -> None:  # noqa : B006
        """Initialize ParsingErrors with a list of errors."""
        self.errors = [(error[0], error[1]) for error in errors]

    def __repr__(self) -> str:
        """Return a string representation of the ParsingErrors."""
        return f"ParsingErrors({self.errors})"
