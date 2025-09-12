"""This module defines the UniversalIdBuilder class for building Universal IDs."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from .imo_number import ImoNumber
    from .local_id_builder import LocalIdBuilder
    from .parsing_errors import ParsingErrors
    from .universal_id import UniversalId
    from .vis_version import VisVersion


class UniversalIdBuilder:
    """Builder class for creating Universal IDs.

    A Universal ID consists of an IMO number and a Local ID.
    """

    NAMING_ENTITY = "data.dnv.com"

    def __init__(
        self,
        imo_number: ImoNumber | None = None,
        local_id: LocalIdBuilder | None = None,
    ) -> None:
        """Initialize a UniversalIdBuilder.

        Args:
            imo_number: The IMO number for the Universal ID
            local_id: The Local ID builder for the Universal ID
        """
        self._imo_number = imo_number
        self._local_id = local_id

    @property
    def imo_number(self) -> ImoNumber | None:
        """Get the IMO number."""
        return self._imo_number

    @property
    def local_id(self) -> LocalIdBuilder | None:
        """Get the Local ID builder."""
        return self._local_id

    @property
    def is_valid(self) -> bool:
        """Check if the Universal ID builder is in a valid state."""
        return (
            self._imo_number is not None
            and self._local_id is not None
            and self._local_id.is_valid
        )

    @classmethod
    def create(cls, version: VisVersion) -> UniversalIdBuilder:
        """Create a new UniversalIdBuilder with the specified VIS version.

        Args:
            version: The VIS version to use

        Returns:
            A new UniversalIdBuilder instance
        """
        # Import at runtime to avoid circular imports
        from .local_id_builder import LocalIdBuilder

        local_id = LocalIdBuilder.create(version)
        return cls().with_local_id(local_id)

    def __eq__(self, other: object) -> bool:
        """Check equality with another UniversalIdBuilder."""
        if not isinstance(other, UniversalIdBuilder):
            return False
        return (
            self._imo_number == other._imo_number and self._local_id == other._local_id
        )

    def build(self) -> UniversalId:
        """Build the Universal ID.

        Returns:
            A Universal ID instance

        Raises:
            ValueError: If the builder is not in a valid state
        """
        # Import at runtime to avoid circular imports
        from .universal_id import UniversalId

        return UniversalId(self)

    def with_local_id(self, local_id: LocalIdBuilder) -> UniversalIdBuilder:
        """Return a new builder with the specified Local ID.

        Args:
            local_id: The Local ID builder to set

        Returns:
            A new UniversalIdBuilder instance

        Raises:
            ValueError: If the local_id is invalid
        """
        builder, succeeded = self.try_with_local_id(local_id)
        if not succeeded:
            raise ValueError("Invalid local_id")
        return builder

    def without_local_id(self) -> UniversalIdBuilder:
        """Return a new builder without a Local ID.

        Returns:
            A new UniversalIdBuilder instance with no Local ID
        """
        return UniversalIdBuilder(self._imo_number, None)

    def try_with_local_id(
        self, local_id: LocalIdBuilder | None
    ) -> tuple[UniversalIdBuilder, bool]:
        """Try to set the Local ID.

        Args:
            local_id: The Local ID builder to set

        Returns:
            A tuple of (new_builder, success_flag)
        """
        if local_id is None:
            return self, False

        return UniversalIdBuilder(self._imo_number, local_id), True

    def try_with_local_id_simple(
        self, local_id: LocalIdBuilder | None
    ) -> UniversalIdBuilder:
        """Try to set the Local ID, returning just the builder.

        Args:
            local_id: The Local ID builder to set

        Returns:
            A new UniversalIdBuilder instance
        """
        builder, _ = self.try_with_local_id(local_id)
        return builder

    def with_imo_number(self, imo_number: ImoNumber) -> UniversalIdBuilder:
        """Return a new builder with the specified IMO number.

        Args:
            imo_number: The IMO number to set

        Returns:
            A new UniversalIdBuilder instance

        Raises:
            ValueError: If the imo_number is invalid
        """
        builder, succeeded = self.try_with_imo_number(imo_number)
        if not succeeded:
            raise ValueError("Invalid imo_number")
        return builder

    def try_with_imo_number_simple(
        self, imo_number: ImoNumber | None
    ) -> UniversalIdBuilder:
        """Try to set the IMO number, returning just the builder.

        Args:
            imo_number: The IMO number to set

        Returns:
            A new UniversalIdBuilder instance
        """
        builder, _ = self.try_with_imo_number(imo_number)
        return builder

    def try_with_imo_number(
        self, imo_number: ImoNumber | None
    ) -> tuple[UniversalIdBuilder, bool]:
        """Try to set the IMO number.

        Args:
            imo_number: The IMO number to set

        Returns:
            A tuple of (new_builder, success_flag)
        """
        if imo_number is None:
            return self, False

        return UniversalIdBuilder(imo_number, self._local_id), True

    def without_imo_number(self) -> UniversalIdBuilder:
        """Return a new builder without an IMO number.

        Returns:
            A new UniversalIdBuilder instance with no IMO number
        """
        return UniversalIdBuilder(None, self._local_id)

    def __hash__(self) -> int:
        """Return hash of the Universal ID builder."""
        return hash((self._imo_number, self._local_id))

    def __str__(self) -> str:
        """Return string representation of the Universal ID.

        Returns:
            String representation of the Universal ID

        Raises:
            RuntimeError: If the builder is not in a valid state
        """
        if self._imo_number is None:
            raise RuntimeError("Invalid Universal Id state: Missing IMO Number")
        if self._local_id is None:
            raise RuntimeError("Invalid Universal Id state: Missing LocalId")

        naming_entity = self.NAMING_ENTITY
        result = f"{naming_entity}/{self._imo_number}"

        # Append the local ID string representation
        result += str(self._local_id)

        return result

    def __repr__(self) -> str:
        """Return detailed string representation."""
        return (
            f"UniversalIdBuilder(imo_number={self._imo_number!r},"
            f" local_id={self._local_id!r})"
        )

    @classmethod
    def parse(cls, universal_id_str: str) -> UniversalIdBuilder:
        """Parse a string into a UniversalIdBuilder.

        Args:
            universal_id_str: String representation of the Universal ID

        Returns:
            UniversalIdBuilder instance

        Raises:
            ValueError: If the string cannot be parsed as a valid Universal ID
        """
        # Import at runtime to avoid circular imports
        from vista_sdk.universal_id_builder_parser import UniversalIdBuilderParser  # noqa: I001

        return UniversalIdBuilderParser.parse(universal_id_str)

    @classmethod
    def try_parse_simple(cls, universal_id: str) -> UniversalIdBuilder | None:
        """Try to parse a string into a UniversalIdBuilder.

        Args:
            universal_id: String representation of the Universal ID

        Returns:
            UniversalIdBuilder instance if successful, None otherwise
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder_parser import UniversalIdBuilderParser

        success, builder = UniversalIdBuilderParser.try_parse(universal_id)
        return builder if success else None

    @classmethod
    def try_parse_with_errors(
        cls, universal_id: str
    ) -> tuple[bool, ParsingErrors, UniversalIdBuilder | None]:
        """Try to parse a string into a UniversalIdBuilder with detailed errors.

        Args:
            universal_id: String representation of the Universal ID

        Returns:
            A tuple of (success, errors, builder)
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder_parser import UniversalIdBuilderParser

        return UniversalIdBuilderParser.try_parse_with_errors(universal_id)

    @classmethod
    def try_parse(cls, universal_id: str) -> tuple[bool, UniversalIdBuilder | None]:
        """Try to parse a string into a UniversalIdBuilder.

        Args:
            universal_id: String representation of the Universal ID

        Returns:
            A tuple of (success, builder)
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder_parser import UniversalIdBuilderParser

        return UniversalIdBuilderParser.try_parse(universal_id)

    @classmethod
    def try_parse_with_errors_v2(
        cls, universal_id: str
    ) -> tuple[bool, ParsingErrors, UniversalIdBuilder | None]:
        """Try to parse a string into a UniversalIdBuilder with detailed errors (v2).

        Args:
            universal_id: String representation of the Universal ID

        Returns:
            A tuple of (success, errors, builder)
        """
        # Import at runtime to avoid circular imports
        from .universal_id_builder_parser import UniversalIdBuilderParser

        return UniversalIdBuilderParser.try_parse_with_errors(universal_id)
