# =============================================================================
# AUTO-GENERATED FILE - DO NOT EDIT MANUALLY
# =============================================================================
#
# This file is generated from the resource files in resources/
#
# To regenerate after adding new VIS versions:
#
#   cd python
#   uv run python src/vista_sdk/source_generator/vis_versions_generator.py
#
# =============================================================================

"""Module providing VIS version enumeration and utilities."""

from __future__ import annotations

import enum


class VisVersion(enum.Enum):
    """Enumeration of VIS versions.

    Represents the various versions of the Vessel Information Structure (VIS).
    Values are integers for proper ordering comparison.
    Use str(version) to get the version string (e.g., '3-4a').
    """

    v3_4a = 0
    v3_5a = 1
    v3_6a = 2
    v3_7a = 3
    v3_8a = 4
    v3_9a = 5
    v3_10a = 6

    def __str__(self) -> str:
        """Return the version string representation."""
        return _VERSION_TO_STRING[self]


_VERSION_TO_STRING: dict[VisVersion, str] = {
    VisVersion.v3_4a: "3-4a",
    VisVersion.v3_5a: "3-5a",
    VisVersion.v3_6a: "3-6a",
    VisVersion.v3_7a: "3-7a",
    VisVersion.v3_8a: "3-8a",
    VisVersion.v3_9a: "3-9a",
    VisVersion.v3_10a: "3-10a",
}


class VisVersionExtension:
    """Utility class for VIS version string manipulation."""

    @staticmethod
    def is_valid(version: object) -> bool:
        """Check if a version is valid."""
        return isinstance(version, VisVersion)


class VisVersions:
    """Utility class for VIS version management."""

    @staticmethod
    def all_versions() -> list[VisVersion]:
        """Get all available VIS versions."""
        return list(VisVersion)

    @staticmethod
    def try_parse(version_str: str) -> VisVersion | None:
        """Try to parse a string into a VisVersion enum."""
        version_map = {
            "3-4a": VisVersion.v3_4a,
            "3-5a": VisVersion.v3_5a,
            "3-6a": VisVersion.v3_6a,
            "3-7a": VisVersion.v3_7a,
            "3-8a": VisVersion.v3_8a,
            "3-9a": VisVersion.v3_9a,
            "3-10a": VisVersion.v3_10a,
        }
        if version_str not in version_map:
            raise ValueError(f"Invalid VisVersion string: {version_str}")
        return version_map[version_str]

    @staticmethod
    def parse(version_str: str) -> VisVersion:
        """Parse a string into a VisVersion enum."""
        version = VisVersions.try_parse(version_str)
        if version is None:
            raise ValueError(f"Invalid VisVersion string: {version_str}")
        return version
