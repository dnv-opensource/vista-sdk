"""LocalIdItems module to handle primary and secondary path items in a Local ID.

This module provides the LocalIdItems class that encapsulates the primary
and secondary items of a Local ID with methods to format them.
"""

from __future__ import annotations

from dataclasses import dataclass

from vista_sdk.gmod_path import GmodPath


@dataclass(frozen=True)
class LocalIdItems:
    """Encapsulates the primary and secondary items of a Local ID.

    This class holds GmodPath objects for the primary and secondary items,
    and provides methods to format them as part of a Local ID string.
    """

    primary_item: GmodPath | None = None
    secondary_item: GmodPath | None = None

    def append(self, builder: list[str], verbose_mode: bool) -> None:
        """Append the string representation of this LocalIdItems to a builder.

        Args:
            builder: A list of strings used as a string builder
            verbose_mode: Whether to include common names in the output
        """
        if self.primary_item is None and self.secondary_item is None:
            return

        if self.primary_item is not None:
            builder.append(str(self.primary_item))
            builder.append("/")

        if self.secondary_item is not None:
            builder.append("sec/")
            builder.append(str(self.secondary_item))
            builder.append("/")

        if verbose_mode:
            if self.primary_item is not None:
                for depth, name in self.primary_item.get_common_names():
                    builder.append("~")
                    location = self.primary_item[depth].location
                    location_str = str(location) if location is not None else None
                    self._append_common_name(builder, name, location_str)
                    builder.append("/")

            if self.secondary_item is not None:
                prefix = "~for."
                for depth, name in self.secondary_item.get_common_names():
                    builder.append(prefix)
                    if prefix != "~":
                        prefix = "~"

                    location = self.secondary_item[depth].location
                    location_str = str(location) if location is not None else None
                    self._append_common_name(builder, name, location_str)
                    builder.append("/")

    @staticmethod
    def _append_common_name(
        builder: list[str], common_name: str, location: str | None
    ) -> None:
        """Append a formatted common name to the builder.

        Args:
            builder: A list of strings used as a string builder
            common_name: The common name to format and append
            location: Optional location to append
        """
        from vista_sdk.vis import VIS  # noqa: PLC0415

        result_chars = []
        prev = None

        for char in common_name:
            if char == "/":
                continue
            if prev == " " and char == " ":
                continue

            current = char
            if char == " ":
                current = "."
            else:
                match = VIS().is_iso_string(char)
                current = "." if not match else char.lower()

            if current == "." and prev == ".":
                continue
            result_chars.append(current)
            prev = current

        builder.append("".join(result_chars))

        if location is not None and len(location) > 0:
            builder.append(".")
            builder.append(location)
