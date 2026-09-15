"""MQTT-formatted LocalId implementation.

This module provides an MQTT-formatted version of the LocalId class
for use in MQTT topics and payloads.
"""

from __future__ import annotations

import io
from typing import TYPE_CHECKING

from vista_sdk.gmod_path import GmodPath
from vista_sdk.local_id import LocalId
from vista_sdk.metadata_tag import MetadataTag
from vista_sdk.vis_version import VisVersion

if TYPE_CHECKING:
    from vista_sdk.local_id_builder import LocalIdBuilder


class MqttLocalId:
    """MQTT-formatted version of the LocalId class.

    This class provides a version of LocalId that formats the string representation
    according to MQTT topic formatting rules.
    """

    # Class variables
    _internal_separator = "_"

    NAMING_RULE = LocalId.NAMING_RULE

    def __init__(self, builder: LocalIdBuilder) -> None:
        """Initialize a new MqttLocalId from a LocalIdBuilder.

        Args:
            builder: The LocalIdBuilder to create the MqttLocalId from

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
        """Get the builder for this MqttLocalId."""
        return self._builder

    @property
    def vis_version(self) -> VisVersion:
        """Get the VIS version for this MqttLocalId."""
        return self._builder.vis_version  # type: ignore

    @property
    def primary_item(self) -> GmodPath:
        """Get the primary item for this MqttLocalId."""
        return self._builder.primary_item  # type: ignore

    @property
    def secondary_item(self) -> GmodPath | None:
        """Get the secondary item for this MqttLocalId."""
        return self._builder.secondary_item

    @property
    def quantity(self) -> MetadataTag | None:
        """Get the quantity metadata tag for this MqttLocalId."""
        return self._builder.quantity

    @property
    def content(self) -> MetadataTag | None:
        """Get the content metadata tag for this MqttLocalId."""
        return self._builder.content

    @property
    def calculation(self) -> MetadataTag | None:
        """Get the calculation metadata tag for this MqttLocalId."""
        return self._builder.calculation

    @property
    def state(self) -> MetadataTag | None:
        """Get the state metadata tag for this MqttLocalId."""
        return self._builder.state

    @property
    def command(self) -> MetadataTag | None:
        """Get the command metadata tag for this MqttLocalId."""
        return self._builder.command

    @property
    def type(self) -> MetadataTag | None:
        """Get the type metadata tag for this MqttLocalId."""
        return self._builder.type

    @property
    def position(self) -> MetadataTag | None:
        """Get the position metadata tag for this MqttLocalId."""
        return self._builder.position

    @property
    def detail(self) -> MetadataTag | None:
        """Get the detail metadata tag for this MqttLocalId."""
        return self._builder.detail

    def __eq__(self, other: object) -> bool:
        """Compare this MqttLocalId with another for equality."""
        if not isinstance(other, MqttLocalId):
            return False

        return self._builder == other._builder

    def __hash__(self) -> int:
        """Get the hash code for this MqttLocalId."""
        return hash(self._builder)

    def __repr__(self) -> str:
        """Get the official string representation of this MqttLocalId."""
        return f"MqttLocalId({self!s})"

    def __str__(self) -> str:
        """Get the string representation of the MqttLocalId.

        Returns:
            The string representation of the MqttLocalId
        """
        # Create a string buffer
        buffer = io.StringIO()

        # Add naming rule
        buffer.write(f"{self.NAMING_RULE}/")

        # Add VIS version
        buffer.write("vis-")
        buffer.write(str(self.vis_version))
        buffer.write("/")

        # Add primary and secondary items
        self._append_primary_item(buffer)
        self._append_secondary_item(buffer)

        # Add metadata tags
        self._append_meta(buffer, self.quantity)
        self._append_meta(buffer, self.content)
        self._append_meta(buffer, self.calculation)
        self._append_meta(buffer, self.state)
        self._append_meta(buffer, self.command)
        self._append_meta(buffer, self.type)
        self._append_meta(buffer, self.position)
        self._append_meta(buffer, self.detail)

        # Get the final string
        result = buffer.getvalue()

        # Remove trailing separator if present
        if result.endswith("/"):
            result = result[:-1]

        buffer.close()
        return result

    def _append_path(self, buffer: io.StringIO, path: GmodPath) -> None:
        """Append a GmodPath to the buffer using MQTT formatting.

        Args:
            buffer: The string buffer to append to
            path: The GmodPath to append
        """
        # Convert the path to string using the MQTT separator
        path_str = str(path).replace("/", self._internal_separator)
        buffer.write(path_str)
        buffer.write("/")

    def _append_primary_item(self, buffer: io.StringIO) -> None:
        """Append the primary item to the buffer.

        Args:
            buffer: The string buffer to append to
        """
        self._append_path(buffer, self.primary_item)

    def _append_secondary_item(self, buffer: io.StringIO) -> None:
        """Append the secondary item to the buffer.

        Args:
            buffer: The string buffer to append to
        """
        if self.secondary_item is None:
            buffer.write("_/")
        else:
            self._append_path(buffer, self.secondary_item)

    def _append_meta(self, buffer: io.StringIO, tag: MetadataTag | None) -> None:
        """Append a metadata tag to the buffer.

        Args:
            buffer: The string buffer to append to
            tag: The metadata tag to append
        """
        if tag is None:
            buffer.write("_/")
        else:
            # For MQTT format, we need to use the full tag string with prefix
            # Use to_string method but capture output without trailing separator
            tag_builder: list[str] = []
            tag.to_string(tag_builder)  # No separator since we add our own
            tag_str = "".join(tag_builder)
            buffer.write(tag_str)
