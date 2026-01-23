"""Value types for transport module data validation."""

from __future__ import annotations

from datetime import datetime


class Value:
    """Base class for typed values used in data validation."""

    def __init__(self) -> None:
        """Private constructor to prevent direct instantiation."""
        pass


class StringValue(Value):
    """String value type."""

    def __init__(self, value: str) -> None:
        """Initialize with string value."""
        super().__init__()
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another StringValue."""
        return isinstance(other, StringValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("string", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"StringValue({self.value!r})"


class CharValue(Value):
    """Character value type."""

    def __init__(self, value: str) -> None:
        """Initialize with character value."""
        super().__init__()
        if len(value) != 1:
            raise ValueError("Character value must be exactly one character")
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another CharValue."""
        return isinstance(other, CharValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("char", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"CharValue({self.value!r})"


class BooleanValue(Value):
    """Boolean value type."""

    def __init__(self, value: bool) -> None:
        """Initialize with boolean value."""
        super().__init__()
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another BooleanValue."""
        return isinstance(other, BooleanValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("boolean", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"BooleanValue({self.value!r})"


class IntegerValue(Value):
    """Integer value type."""

    def __init__(self, value: int) -> None:
        """Initialize with integer value."""
        super().__init__()
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another IntegerValue."""
        return isinstance(other, IntegerValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("integer", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"IntegerValue({self.value!r})"


class UnsignedIntegerValue(Value):
    """Unsigned integer value type."""

    def __init__(self, value: int) -> None:
        """Initialize with unsigned integer value."""
        super().__init__()
        if value < 0:
            raise ValueError("Unsigned integer must be non-negative")
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another UnsignedIntegerValue."""
        return isinstance(other, UnsignedIntegerValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("unsigned_integer", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"UnsignedIntegerValue({self.value!r})"


class LongValue(Value):
    """Long integer value type."""

    def __init__(self, value: int) -> None:
        """Initialize with long integer value."""
        super().__init__()
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another LongValue."""
        return isinstance(other, LongValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("long", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"LongValue({self.value!r})"


class DoubleValue(Value):
    """Double precision floating point value type."""

    def __init__(self, value: float) -> None:
        """Initialize with double value."""
        super().__init__()
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another DoubleValue."""
        return isinstance(other, DoubleValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("double", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"DoubleValue({self.value!r})"


class DecimalValue(Value):
    """Decimal value type."""

    def __init__(self, value: float) -> None:
        """Initialize with decimal value."""
        super().__init__()

        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another DecimalValue."""
        return isinstance(other, DecimalValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("decimal", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"DecimalValue({self.value!r})"


class DateTimeValue(Value):
    """DateTime value type."""

    def __init__(self, value: datetime) -> None:
        """Initialize with datetime value."""
        super().__init__()
        self.value = value

    def __eq__(self, other: object) -> bool:
        """Check equality with another DateTimeValue."""
        return isinstance(other, DateTimeValue) and self.value == other.value

    def __hash__(self) -> int:
        """Return hash of the value."""
        return hash(("datetime", self.value))

    def __repr__(self) -> str:
        """Return string representation."""
        return f"DateTimeValue({self.value!r})"


# Type alias for any value type
AnyValue = (
    StringValue
    | CharValue
    | BooleanValue
    | IntegerValue
    | UnsignedIntegerValue
    | LongValue
    | DoubleValue
    | DecimalValue
    | DateTimeValue
)
