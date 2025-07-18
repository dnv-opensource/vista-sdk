"""This module defines the UniversalId class and related protocols."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .imo_number import ImoNumber
    from .local_id import LocalId
    from .parsing_errors import ParsingErrors

from typing import TYPE_CHECKING, Protocol

if TYPE_CHECKING:
    from .imo_number import ImoNumber
    from .local_id import LocalId
    from .parsing_errors import ParsingErrors
    from .universal_id_builder import UniversalIdBuilder


class IUniversalId(Protocol):
    """Protocol for Universal ID implementations."""

    @property
    def imo_number(self) -> ImoNumber:
        """Get the IMO number."""
        ...

    @property
    def local_id(self) -> LocalId:
        """Get the Local ID."""
        ...


class UniversalId(IUniversalId):
    """Represents a Universal ID consisting of an IMO number and Local ID."""

    def __init__(self, builder: UniversalIdBuilder) -> None:
        """Initialize a UniversalId from a builder.

        Args:
            builder: The UniversalIdBuilder to build from

        Raises:
            ValueError: If the builder is not in a valid state
        """
        if not builder.is_valid:
            raise ValueError("Invalid UniversalId state")

        self._builder = builder
        # Build the local ID once and cache it
        if builder.local_id is None:
            raise ValueError("Local ID is required but was None")
        self._local_id = builder.local_id.build()

    @property
    def imo_number(self) -> ImoNumber:
        """Get the IMO number."""
        if self._builder.imo_number is None:
            raise RuntimeError("Invalid ImoNumber")
        return self._builder.imo_number

    @property
    def local_id(self) -> LocalId:
        """Get the Local ID."""
        return self._local_id

    def __eq__(self, other: object) -> bool:
        """Check equality with another UniversalId."""
        if not isinstance(other, UniversalId):
            return False
        return self._builder == other._builder

    @classmethod
    def parse(cls, universal_id_str: str) -> UniversalId:
        """Parse a string into a UniversalId.

        Args:
            universal_id_str: String representation of the Universal ID

        Returns:
            UniversalId instance

        Raises:
            ValueError: If the string cannot be parsed as a valid Universal ID
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder import UniversalIdBuilder  # noqa: PLC0415

        return UniversalIdBuilder.parse(universal_id_str).build()

    @classmethod
    def try_parse(cls, universal_id_str: str) -> tuple[bool, UniversalId | None]:
        """Try to parse a string into a UniversalId.

        Args:
            universal_id_str: String representation of the Universal ID

        Returns:
            A tuple of (success, universal_id). If parsing succeeds, success
            will be True and universal_id will contain the parsed Universal ID.
            If parsing fails, success will be False and universal_id will be None.
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder import UniversalIdBuilder  # noqa: PLC0415

        builder = UniversalIdBuilder.try_parse_simple(universal_id_str)
        if builder is None:
            return False, None

        try:
            return True, builder.build()
        except ValueError:
            return False, None

    @classmethod
    def try_parse_with_errors(
        cls, universal_id_str: str
    ) -> tuple[bool, ParsingErrors, UniversalId | None]:
        """Try to parse a string into a UniversalId with detailed error information.

        Args:
            universal_id_str: String representation of the Universal ID

        Returns:
            A tuple of (success, errors, universal_id). If parsing succeeds, success
            will be True, errors will be empty, and universal_id will contain the
            parsed Universal ID. If parsing fails, success will be False, errors
            will contain the parsing errors, and universal_id will be None.
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder import UniversalIdBuilder  # noqa: PLC0415

        success, errors, builder = UniversalIdBuilder.try_parse_with_errors(
            universal_id_str
        )
        if not success or builder is None:
            return success, errors, None

        try:
            return True, errors, builder.build()
        except ValueError:
            # If building fails, add that as an error
            return False, errors, None

    def __str__(self) -> str:
        """Return string representation of the Universal ID."""
        return str(self._builder)

    def __hash__(self) -> int:
        """Return hash of the Universal ID."""
        return hash(self._builder)

    def __repr__(self) -> str:
        """Return detailed string representation."""
        return (
            f"UniversalId(imo_number={self.imo_number!r}, local_id={self.local_id!r})"
        )
