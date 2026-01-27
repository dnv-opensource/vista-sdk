"""Ship ID implementation for transport module."""

from __future__ import annotations

from collections.abc import Callable
from typing import TypeVar

from vista_sdk.imo_number import ImoNumber

T = TypeVar("T")


class ShipId:
    """Represents a ship identifier that can be either an IMO number or other ID."""

    def __init__(self, value: ImoNumber | str) -> None:
        """Initialize ShipId with either IMO number or string ID.

        Args:
            value: Either an ImoNumber or string identifier
        """
        if isinstance(value, ImoNumber):
            self._tag = 1
            self._imo_number: ImoNumber | None = value
            self._other_id: str | None = None
        else:
            self._tag = 2
            self._imo_number = None
            self._other_id = value

    @property
    def is_imo_number(self) -> bool:
        """Check if this ShipId contains an IMO number."""
        return self._tag == 1

    @property
    def is_other_id(self) -> bool:
        """Check if this ShipId contains an other ID."""
        return self._tag == 2

    @property
    def imo_number(self) -> ImoNumber | None:
        """Get the IMO number if this ShipId contains one."""
        return self._imo_number if self._tag == 1 else None

    @property
    def other_id(self) -> str | None:
        """Get the other ID if this ShipId contains one."""
        return self._other_id if self._tag == 2 else None

    def match(
        self, on_imo_number: Callable[[ImoNumber], T], on_other_id: Callable[[str], T]
    ) -> T:
        """Pattern match on the ShipId type and return result.

        Args:
            on_imo_number: Function to call if ShipId contains IMO number
            on_other_id: Function to call if ShipId contains other ID

        Returns:
            Result of the matching function

        Raises:
            ValueError: If ShipId is in invalid state
        """
        if self._tag == 1:
            return on_imo_number(self._imo_number)  # type: ignore
        if self._tag == 2:
            return on_other_id(self._other_id)  # type: ignore
        raise ValueError("Tried to match on invalid ShipId")

    def switch(
        self,
        on_imo_number: Callable[[ImoNumber], None],
        on_other_id: Callable[[str], None],
    ) -> None:
        """Pattern match on the ShipId type and execute action.

        Args:
            on_imo_number: Function to call if ShipId contains IMO number
            on_other_id: Function to call if ShipId contains other ID

        Raises:
            ValueError: If ShipId is in invalid state
        """
        if self._tag == 1:
            on_imo_number(self._imo_number)  # type: ignore
        elif self._tag == 2:
            on_other_id(self._other_id)  # type: ignore
        else:
            raise ValueError("Tried to switch on invalid ShipId")

    def __str__(self) -> str:
        """Return string representation of ShipId.

        Returns:
            String representation - IMO number as string for IMO type,
            or the other ID directly for other type
        """
        if self._tag == 1:
            # In ISO-19848, IMO number as ShipID should be prefixed with "IMO"
            return str(self._imo_number)
        if self._tag == 2:
            return self._other_id  # type: ignore
        raise ValueError("Invalid state exception")

    @classmethod
    def parse(cls, value: str) -> ShipId:
        """Parse a string into a ShipId.

        Args:
            value: String to parse

        Returns:
            ShipId instance

        Raises:
            ValueError: If value is None or empty
        """
        if not value:
            raise ValueError("Value cannot be None or empty")

        # In ISO-19848, IMO number as ShipID should be prefixed with "IMO"
        if (
            value.upper().startswith("IMO")
            and (imo := ImoNumber.try_parse(value)) is not None
        ):
            return cls(imo)
        return cls(value)

    @classmethod
    def from_imo_number(cls, imo_number: ImoNumber) -> ShipId:
        """Create ShipId from IMO number.

        Args:
            imo_number: The IMO number

        Returns:
            ShipId instance containing the IMO number
        """
        return cls(imo_number)

    @classmethod
    def from_other_id(cls, other_id: str) -> ShipId:
        """Create ShipId from other ID string.

        Args:
            other_id: The other ID string

        Returns:
            ShipId instance containing the other ID
        """
        return cls(other_id)

    def __eq__(self, other: object) -> bool:
        """Check equality with another ShipId."""
        if not isinstance(other, ShipId):
            return False

        if self._tag != other._tag:
            return False

        if self._tag == 1:
            return self._imo_number == other._imo_number
        return self._other_id == other._other_id

    def __hash__(self) -> int:
        """Return hash of ShipId."""
        if self._tag == 1:
            return hash(("imo", self._imo_number))
        return hash(("other", self._other_id))

    def __repr__(self) -> str:
        """Return detailed string representation."""
        if self._tag == 1:
            return f"ShipId(imo_number={self._imo_number!r})"
        return f"ShipId(other_id={self._other_id!r})"
