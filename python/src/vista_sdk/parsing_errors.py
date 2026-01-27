"""Parsing errors for location validation.

This module defines the ParsingErrors class, which encapsulates a collection of
parsing errors encountered during location validation. It provides methods to check
for errors, retrieve error types, and format error messages.
"""

from __future__ import annotations

from collections.abc import Iterator
from typing import TypeAlias

# Type alias for error entries (type, message)
ErrorEntry: TypeAlias = tuple[str, str]


class ParsingErrors:
    """Encapsulates parsing errors encountered during location validation.

    This class manages a collection of parsing errors, where each error consists of
    a LocationValidationResult type and an associated message.
    """

    def __init__(self, errors: list[ErrorEntry] | None = None) -> None:
        """Initialize ParsingErrors with an optional list of errors.

        Args:
            errors: Optional list of error tuples (type, message).
        """
        self._errors: list[ErrorEntry] = errors if errors is not None else []

    @property
    def has_errors(self) -> bool:
        """Check if there are any parsing errors."""
        return len(self._errors) > 0

    def has_error_type(self, error_type: str) -> bool:
        """Check if a specific error type exists in the collection."""
        return any(e[0] == error_type for e in self._errors)

    def __str__(self) -> str:
        """Get a formatted string representation of the errors."""
        if not self.has_errors:
            return "Success"

        error_messages = ["Parsing errors:"]
        for error_type, message in self._errors:
            error_messages.append(f"\t{error_type} - {message}")
        return "\n".join(error_messages)

    def __eq__(self, other: object) -> bool:
        """Compare two ParsingErrors instances for equality."""
        if isinstance(other, ParsingErrors):
            return self._errors == other._errors
        return False

    def __hash__(self) -> int:
        """Get a hash value for the ParsingErrors instance."""
        return hash(tuple(self._errors))

    def __iter__(self) -> Iterator[ErrorEntry]:
        """Get an iterator over the errors."""
        return iter(self._errors)

    def __next__(self) -> ErrorEntry:
        """Get the next error from the iterator."""
        return next(self.__iter__())

    @classmethod
    def empty(cls) -> ParsingErrors:
        """Create an empty ParsingErrors instance."""
        return cls()
