"""This module defines the ImoNumber class, which represents an IMO number.

IMO stands for International Maritime Organization.
"""


class ImoNumber:
    """Represents an IMO (International Maritime Organization) number.

    An IMO number is made of the three letters "IMO" followed by a seven-digit number.
    This consists of a six-digit sequential unique number followed by a check digit.
    """

    def __init__(self, value: int | str) -> None:
        """Initialize an ImoNumber.

        Args:
            value: Either an integer or string representation of the IMO number

        Raises:
            ValueError: If the IMO number is invalid
        """
        if isinstance(value, str):
            parsed_value = self._parse_string(value)
            if parsed_value is None:
                raise ValueError(f"Invalid IMO number: {value}")
            self._value = parsed_value
        elif isinstance(value, int):
            if not self.is_valid(value):
                raise ValueError(f"Invalid IMO number: {value}")
            self._value = value
        else:
            raise TypeError("IMO number must be int or str")

    @classmethod
    def parse(cls, value: str) -> "ImoNumber":
        """Parse a string into an ImoNumber.

        Args:
            value: String representation of the IMO number

        Returns:
            ImoNumber instance

        Raises:
            ValueError: If the string cannot be parsed as a valid IMO number
        """
        return cls(value)

    @classmethod
    def try_parse(cls, value: str) -> "ImoNumber | None":
        """Try to parse a string into an ImoNumber.

        Args:
            value: String representation of the IMO number

        Returns:
            ImoNumber instance if successful, None otherwise
        """
        try:
            return cls(value)
        except (ValueError, TypeError):
            return None

    def _parse_string(self, value: str) -> int | None:
        """Parse string value, handling 'IMO' prefix."""
        value = value.strip()

        number_part = value[3:] if value.upper().startswith("IMO") else value

        try:
            num = int(number_part)
            return num if self.is_valid(num) else None
        except ValueError:
            return None

    @staticmethod
    def is_valid(imo_number: int) -> bool:
        """Validate an IMO number using its check digit.

        The integrity of an IMO number can be verified using its check digit.
        This is done by multiplying each of the first six digits by a factor
        of 2 to 7 corresponding to their position from right to left.
        The rightmost digit of this sum is the check digit.

        For example,
        for IMO 9074729: (9x7) + (0x6) + (7x5) + (4x4) + (7x3) + (2x2) = 139

        Args:
            imo_number: The IMO number to validate

        Returns:
            True if valid, False otherwise
        """
        if imo_number < 1000000 or imo_number > 9999999:
            return False

        digits = ImoNumber._get_digits(imo_number)

        check_digit = 0
        for i in range(1, len(digits)):
            check_digit += (i + 1) * digits[i]

        return imo_number % 10 == check_digit % 10

    @staticmethod
    def _get_digits(number: int) -> list[int]:
        """Extract digits from number in reverse order (least significant first)."""
        digits = []
        current = number

        while current > 0:
            if current < 10:
                digits.append(current)
                break
            next_val = current // 10
            digit = current - next_val * 10
            digits.append(digit)
            current = next_val

        return digits

    def __str__(self) -> str:
        """Return string representation with IMO prefix."""
        return f"IMO{self._value}"

    def __repr__(self) -> str:
        """Return detailed string representation."""
        return f"ImoNumber({self._value})"

    def __int__(self) -> int:
        """Convert to integer."""
        return self._value

    def __eq__(self, other) -> bool:  # noqa: ANN001
        """Check equality with another ImoNumber."""
        if isinstance(other, ImoNumber):
            return self._value == other._value
        return False

    def __hash__(self) -> int:
        """Return hash of the IMO number."""
        return hash(self._value)

    @property
    def value(self) -> int:
        """Get the numeric value of the IMO number."""
        return self._value
