"""Implementation of the LocalId class.

This module provides the LocalId class which represents a complete and valid vessel
local identifier, as well as the LocalIdBuilder class which facilitates the construction
of LocalId objects.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from vista_sdk.gmod_path import GmodPath
from vista_sdk.metadata_tag import MetadataTag
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.vis_version import VisVersion

if TYPE_CHECKING:
    from vista_sdk.local_id_builder import LocalIdBuilder


class LocalId:
    """Represents a complete and valid vessel local identifier."""

    NAMING_RULE = "dnv-v2"

    def __init__(self, builder: LocalIdBuilder) -> None:
        """Initialize a LocalId with a builder.

        Args:
            builder: The builder containing the LocalId information.

        Raises:
            ValueError: If the builder is empty or invalid.
        """
        if builder.is_empty:
            raise ValueError("LocalId cannot be constructed from empty LocalIdBuilder")
        if not builder.is_valid:
            raise ValueError(
                "LocalId cannot be constructed from invalid LocalIdBuilder"
            )
        self._builder = builder

    @property
    def builder(self) -> LocalIdBuilder:
        """Get the builder for this LocalId."""
        return self._builder

    @property
    def vis_version(self) -> VisVersion:
        """Get the VIS version for this LocalId."""
        # Builder must have a valid VIS version at this point
        return self._builder.vis_version  # type: ignore

    @property
    def verbose_mode(self) -> bool:
        """Get whether verbose mode is enabled for this LocalId."""
        return self._builder.verbose_mode

    @property
    def primary_item(self) -> GmodPath:
        """Get the primary item for this LocalId."""
        # Builder must have a valid primary item at this point
        return self._builder.primary_item  # type: ignore

    @property
    def secondary_item(self) -> GmodPath | None:
        """Get the secondary item for this LocalId."""
        return self._builder.secondary_item

    @property
    def quantity(self) -> MetadataTag | None:
        """Get the quantity metadata tag for this LocalId."""
        return self._builder.quantity

    @property
    def content(self) -> MetadataTag | None:
        """Get the content metadata tag for this LocalId."""
        return self._builder.content

    @property
    def calculation(self) -> MetadataTag | None:
        """Get the calculation metadata tag for this LocalId."""
        return self._builder.calculation

    @property
    def state(self) -> MetadataTag | None:
        """Get the state metadata tag for this LocalId."""
        return self._builder.state

    @property
    def command(self) -> MetadataTag | None:
        """Get the command metadata tag for this LocalId."""
        return self._builder.command

    @property
    def type(self) -> MetadataTag | None:
        """Get the type metadata tag for this LocalId."""
        return self._builder.type

    @property
    def position(self) -> MetadataTag | None:
        """Get the position metadata tag for this LocalId."""
        return self._builder.position

    @property
    def detail(self) -> MetadataTag | None:
        """Get the detail metadata tag for this LocalId."""
        return self._builder.detail

    @property
    def has_custom_tag(self) -> bool:
        """Check if this LocalId has custom metadata tags."""
        return self._builder.has_custom_tag

    @property
    def metadata_tags(self) -> list[MetadataTag]:
        """Get all metadata tags for this LocalId."""
        return self._builder.metadata_tags

    def __eq__(self, other: object) -> bool:
        """Compare this LocalId with another for equality."""
        if not isinstance(other, LocalId):
            return False

        return self._builder == other._builder

    def __hash__(self) -> int:
        """Get the hash code for this LocalId."""
        return hash(self._builder)

    def __str__(self) -> str:
        """Convert this LocalId to its string representation."""
        return str(self._builder)

    @staticmethod
    def parse(local_id_str: str) -> LocalId:
        """Parse a local ID string into a LocalId.

        Args:
            local_id_str: The local ID string to parse.

        Returns:
            A LocalId containing the parsed information.

        Raises:
            ValueError: If the parsing fails.
        """
        from vista_sdk.local_id_builder_parsing import LocalIdBuilderParsing

        parser = LocalIdBuilderParsing()
        return parser.parse(local_id_str).build()

    @staticmethod
    def try_parse(local_id_str: str) -> tuple[bool, ParsingErrors, LocalId | None]:
        """Try to parse a local ID string into a LocalId.

        Args:
            local_id_str: The local ID string to parse.

        Returns:
            A tuple containing:
            - A boolean indicating whether the parsing succeeded.
            - The parsing errors encountered.
            - The resulting LocalId, or None if parsing failed.
        """
        from vista_sdk.local_id_builder_parsing import LocalIdBuilderParsing

        parser = LocalIdBuilderParsing()

        success, errors, builder = parser.try_parse(local_id_str)
        if not success or builder is None:
            return False, errors, None

        try:
            return True, errors, builder.build()
        except ValueError:
            return False, errors, None
