"""Module providing VIS version enumeration and utilities."""

from __future__ import annotations

import enum


class VisVersion(enum.Enum):
    """Enumeration of VIS versions.

    Represents the various versions of the Vessel Information Structure (VIS).
    """

    v3_4a = "3-4a"
    v3_8a = "3-8a"
    v3_6a = "3-6a"
    v3_5a = "3-5a"
    v3_7a = "3-7a"


class VisVersionExtension:
    """Utility class for VIS version string manipulation."""

    @staticmethod
    def to_version_string(version: VisVersion, builder: list[str] | None = None) -> str:
        """Convert a VisVersion enum to its string representation."""
        version_map = {
            VisVersion.v3_4a: "3-4a",
            VisVersion.v3_8a: "3-8a",
            VisVersion.v3_6a: "3-6a",
            VisVersion.v3_5a: "3-5a",
            VisVersion.v3_7a: "3-7a",
        }
        v = version_map.get(version)
        if v is None:
            raise ValueError(f"Invalid VisVersion enum value: {version}")
        if builder is not None:
            builder.append(v)
        return v

    @staticmethod
    def to_string(version: VisVersion, builder: list[str] | None = None) -> str:
        """Convert a VisVersion enum to its string representation."""
        return VisVersionExtension.to_version_string(version, builder)

    @staticmethod
    def is_valid(version: object) -> bool:
        """Check if a version is valid."""
        return isinstance(version, VisVersion)


class VisVersions:
    """Utility class for VIS version management."""

    @staticmethod
    def all_versions() -> list[VisVersion]:
        """Get all available VIS versions."""
        return [
            version for version in VisVersion if VisVersions.try_parse(version.value)
        ]

    @staticmethod
    def try_parse(version_str: str) -> VisVersion | None:
        """Try to parse a string into a VisVersion enum."""
        version_map = {
            "3-4a": VisVersion.v3_4a,
            "3-8a": VisVersion.v3_8a,
            "3-6a": VisVersion.v3_6a,
            "3-5a": VisVersion.v3_5a,
            "3-7a": VisVersion.v3_7a,
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
