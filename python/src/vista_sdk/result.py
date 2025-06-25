"""This module defines the result types used in validation processes."""

from dataclasses import dataclass


class ValidateResult:
    """Base class for validation results."""

    pass


@dataclass(frozen=True)
class Ok(ValidateResult):
    """Represents a successful validation result."""

    pass


@dataclass(frozen=True)
class Invalid(ValidateResult):
    """Represents a failed validation with error messages."""

    messages: list[str]
