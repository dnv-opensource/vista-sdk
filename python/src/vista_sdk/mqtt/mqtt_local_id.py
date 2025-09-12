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

if TYPE_CHECKING:
    from vista_sdk.local_id_builder import LocalIdBuilder


class MqttLocalId(LocalId):
    """MQTT-formatted version of the LocalId class.

    This class provides a version of LocalId that formats the string representation
    according to MQTT topic formatting rules.
    """

    # Class variables
    _internal_separator = "_"

    def __init__(self, builder: LocalIdBuilder) -> None:
        """Initialize a new MqttLocalId from a LocalIdBuilder.

        Args:
            builder: The LocalIdBuilder to create the MqttLocalId from
        """
        super().__init__(builder)

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
        # Remove the 'v' prefix and replace underscores with hyphens
        version_name = self.vis_version.name
        if version_name.startswith("v"):
            version_name = version_name[1:]
        buffer.write(version_name.replace("_", "-"))
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
            tag.to_string(tag_builder, "")  # No separator since we add our own
            tag_str = "".join(tag_builder)
            buffer.write(tag_str)
            buffer.write("/")
