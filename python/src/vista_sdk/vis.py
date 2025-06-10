"""Vessel Information Structure (VIS) implementation module.

This module provides the main VIS interface and implementation for accessing and
managing vessel information structures, including General Maritime Object Data
(GMOD) and locations.
"""

from __future__ import annotations

import os
from abc import ABC, abstractmethod

from cachetools import TTLCache
from dotenv import load_dotenv

from vista_sdk.client import Client
from vista_sdk.gmod import Gmod
from vista_sdk.gmod_dto import GmodDto
from vista_sdk.locations import Locations
from vista_sdk.locations_dto import LocationsDto
from vista_sdk.vis_version import VisVersion, VisVersionExtension

load_dotenv()
environment = os.getenv("ENVIRONMENT")


class IVIS(ABC):
    """Interface defining the VIS (Vessel Information Structure) operations.

    This abstract base class defines the required operations for any VIS implementation.
    """

    @abstractmethod
    def get_locations(self, vis_version: VisVersion) -> Locations:
        """Get locations for a specific VIS version."""
        ...

    @abstractmethod
    def get_locations_map(self, vis_version: VisVersion) -> dict[VisVersion, Locations]:
        """Get a mapping of VIS versions to their locations."""
        ...

    @abstractmethod
    def get_vis_versions(self) -> list[VisVersion]:
        """Get all available VIS versions."""
        ...

    @abstractmethod
    def get_gmod(self, vis_version: VisVersion) -> Gmod:
        """Get GMOD for a specific VIS version."""
        ...

    @abstractmethod
    def get_gmods_map(self) -> dict[VisVersion, Gmod]:
        """Get a mapping of VIS versions to their GMODs."""
        ...


class VIS(IVIS):
    """VIS (Vessel Information Structure) implementation with caching.

    This class implements the IVIS interface and provides caching mechanisms for
    locations and GMOD data. It follows the singleton pattern to ensure only one
    instance exists.
    """

    LatestVisVersion = VisVersion.v3_7a
    _locations_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)  # TTL is in seconds
    _locations_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)

    client = Client()

    def __new__(cls) -> VIS:
        """Create or return the singleton instance of VIS."""
        if not hasattr(cls, "instance"):
            cls.instance = super().__new__(cls)
        return cls.instance

    def get_gmod_dto(self, vis_version: VisVersion) -> GmodDto:
        """Get GMOD DTO for a specific VIS version with caching."""
        if vis_version in self._gmod_dto_cache:
            return self._gmod_dto_cache[vis_version]

        def load_and_cache() -> GmodDto:
            dto = self.load_gmod_dto(vis_version)
            if dto is None:
                raise Exception("Invalid state")
            self._gmod_dto_cache[vis_version] = dto
            return dto

        return load_and_cache()

    def load_gmod_dto(self, vis_version: VisVersion) -> GmodDto | None:
        """Load GMOD DTO from the client."""
        vis_version_str = VisVersionExtension.to_version_string(vis_version)
        if environment == "local":
            return self.client.get_gmod_test(vis_version_str)
        return self.client.get_gmod(vis_version_str)

    def get_gmod(self, vis_version: VisVersion) -> Gmod:
        """Get GMOD for a specific VIS version with caching."""
        if not VisVersionExtension.is_valid(vis_version):
            raise ValueError(f"Invalid VIS version: {vis_version}")
        if vis_version not in self._gmod_cache:
            self._gmod_cache[vis_version] = self.create_gmod(vis_version)

        return self._gmod_cache[vis_version]

    def create_gmod(self, vis_version: VisVersion) -> Gmod:
        """Create a new GMOD instance."""
        from .gmod import Gmod

        dto = self.get_gmod_dto(vis_version)
        return Gmod(vis_version, dto)

    def get_gmods_map(self) -> dict[VisVersion, Gmod]:
        """Get a mapping of all VIS versions to their GMODs."""
        return {version: self.get_gmod(version) for version in VisVersion}

    def get_locations(self, vis_version: VisVersion) -> Locations:
        """Get locations for a specific VIS version with caching."""
        if vis_version in self._locations_cache:
            return self._locations_cache[vis_version]
        dto = self.get_locations_dto(vis_version)
        location = Locations(vis_version, dto)
        self._locations_cache[vis_version] = location
        return location

    def get_locations_dto(self, vis_version: VisVersion) -> LocationsDto:
        """Get locations DTO for a specific VIS version with caching."""
        if vis_version in self._locations_dto_cache:
            return self._locations_dto_cache[vis_version]
        if environment == "local":
            dto = self.client.get_locations_test(
                VisVersionExtension.to_version_string(vis_version)
            )
        else:
            dto = self.client.get_locations(
                VisVersionExtension.to_version_string(vis_version)
            )
        if dto is None:
            raise Exception("Invalid state")

        self._locations_dto_cache[vis_version] = dto
        return dto

    def get_locations_map(self, vis_version: VisVersion) -> dict[VisVersion, Locations]:
        """Get a mapping of a single VIS version to its locations."""
        if vis_version.name not in VisVersion.__members__:
            raise ValueError(f"Invalid VIS version provided: {vis_version}")

        return {vis_version: self.get_locations(vis_version)}

    def get_vis_versions(self) -> list[VisVersion]:
        """Get all available VIS versions."""
        return list(VisVersion)

    @staticmethod
    def is_iso_string(span: str) -> bool:
        """Check if a string follows ISO string format."""
        return all(VIS.match_ascii_decimal(ord(char)) for char in span)

    @staticmethod
    def match_ascii_decimal(code: int) -> bool:
        """Check if an ASCII code matches allowed decimal values.

        Allowed ranges:
        - Numbers (48-57)
        - Uppercase letters A-Z (65-90)
        - Lowercase letters a-z (97-122)
        - Special characters: "-", ".", "_", "~"
        """
        # Number
        if 48 <= code <= 57:
            return True
        # Large character A-Z
        if 65 <= code <= 90:
            return True
        # Small character a-z
        if 97 <= code <= 122:
            return True
        # ["-", ".", "_", "~"] respectively
        return code in (45, 46, 95, 126)
