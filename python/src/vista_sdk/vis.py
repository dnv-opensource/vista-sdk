"""Vessel Information Structure (VIS) implementation module.

This module provides the main VIS interface and implementation for accessing and
managing vessel information structures, including Generic Product Model
(GMOD) and locations.
"""

from __future__ import annotations

import os
from abc import ABC, abstractmethod

from cachetools import TTLCache
from dotenv import load_dotenv

from vista_sdk.client import Client
from vista_sdk.codebook_dto import CodebooksDto
from vista_sdk.codebooks import Codebooks
from vista_sdk.gmod import Gmod
from vista_sdk.gmod_dto import GmodDto
from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_versioning import GmodVersioning
from vista_sdk.gmod_versioning_dto import GmodVersioningDto
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_builder import LocalIdBuilder
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
    def get_locations_map(
        self, vis_versions: list[VisVersion]
    ) -> dict[VisVersion, Locations]:
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
    def get_gmods_map(self, vis_versions: list[VisVersion]) -> dict[VisVersion, Gmod]:
        """Get a mapping of VIS versions to their GMODs."""
        ...

    @abstractmethod
    def get_codebooks(self, vis_version: VisVersion) -> Codebooks:
        """Get codebooks for a specific VIS version."""
        ...

    @abstractmethod
    def get_codebooks_map(
        self, vis_versions: list[VisVersion]
    ) -> dict[VisVersion, Codebooks]:
        """Get a mapping of VIS versions to their codebooks."""
        ...

    @abstractmethod
    def convert_node(
        self,
        source_version: VisVersion,
        source_node: GmodNode,
        target_version: VisVersion,
    ) -> GmodNode | None:
        """Convert a node from one version to another."""
        ...

    @abstractmethod
    def convert_path(
        self,
        source_version: VisVersion,
        source_path: GmodPath,
        target_version: VisVersion,
    ) -> GmodPath | None:
        """Convert a path form one version to another."""
        ...

    @abstractmethod
    def convert_local_id(
        self, source_local_id: LocalId, target_version: VisVersion
    ) -> LocalId | None:
        """Convert a LocalId from one version to another."""
        ...

    @abstractmethod
    def convert_local_id_builder(
        self, source_local_id_builder: LocalIdBuilder, target_version: VisVersion
    ) -> LocalIdBuilder | None:
        """Convert a LocalIdBuilder from one version to another."""
        ...


class VIS(IVIS):
    """VIS (Vessel Information Structure) implementation with caching.

    This class implements the IVIS interface and provides caching mechanisms for
    locations and GMOD data. It follows the singleton pattern to ensure only one
    instance exists.
    """

    latest_vis_version = VisVersion.v3_10a
    _locations_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)  # TTL is in seconds
    _locations_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_versioning_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_versioning_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _codebooks_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _codebooks_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _versioning_key = "versioning"

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
        return self.client.get_gmod(str(vis_version))

    def get_gmod(self, vis_version: VisVersion) -> Gmod:
        """Get GMOD for a specific VIS version with caching."""
        if not VisVersionExtension.is_valid(vis_version):
            raise ValueError(f"Invalid VIS version: {vis_version}")
        if vis_version not in self._gmod_cache:
            self._gmod_cache[vis_version] = self.create_gmod(vis_version)

        return self._gmod_cache[vis_version]

    def get_gmods_map(self, vis_versions: list[VisVersion]) -> dict[VisVersion, Gmod]:
        """Get a mapping of VIS versions to their GMODs."""
        return {version: self.get_gmod(version) for version in vis_versions}

    def create_gmod(self, vis_version: VisVersion) -> Gmod:
        """Create a new GMOD instance."""
        from .gmod import Gmod

        dto = self.get_gmod_dto(vis_version)
        return Gmod(vis_version, dto)

    def get_gmod_versioning_dto(self) -> dict[str, GmodVersioningDto]:
        """Get GMOD versioning DTO with caching."""
        if self._versioning_key in self._gmod_versioning_dto_cache:
            return self._gmod_versioning_dto_cache[self._versioning_key]

        versioning = self.client.get_gmod_versioning()
        self._gmod_versioning_dto_cache[self._versioning_key] = versioning
        return versioning

    def get_gmod_versioning(self) -> GmodVersioning:
        """Get GMOD versioning with caching."""
        if self._versioning_key in self._gmod_versioning_cache:
            return self._gmod_versioning_cache[self._versioning_key]
        # First try to get the versioning DTO
        dto = self.get_gmod_versioning_dto()
        versioning = GmodVersioning(dto)
        self._gmod_versioning_cache[self._versioning_key] = versioning
        return versioning

    def get_codebooks_dto(self, vis_version: VisVersion) -> CodebooksDto:
        """Get codebooks DTO for a specific VIS version with caching."""
        if vis_version in self._codebooks_dto_cache:
            return self._codebooks_dto_cache[vis_version]

        dto = self.client.get_codebooks(str(vis_version))

        if dto is None:
            raise Exception("Invalid state")

        self._codebooks_dto_cache[vis_version] = dto
        return dto

    def get_codebooks(self, vis_version: VisVersion) -> Codebooks:
        """Get codebooks for a specific VIS version with caching."""
        if vis_version in self._codebooks_cache:
            return self._codebooks_cache[vis_version]

        dto = self.get_codebooks_dto(vis_version)
        codebooks = Codebooks(vis_version, dto)
        self._codebooks_cache[vis_version] = codebooks
        return codebooks

    def get_codebooks_map(
        self, vis_versions: list[VisVersion]
    ) -> dict[VisVersion, Codebooks]:
        """Get a mapping of VIS versions to their codebooks."""
        return {version: self.get_codebooks(version) for version in vis_versions}

    def get_locations_dto(self, vis_version: VisVersion) -> LocationsDto:
        """Get locations DTO for a specific VIS version with caching."""
        if vis_version in self._locations_dto_cache:
            return self._locations_dto_cache[vis_version]

        dto = self.client.get_locations(str(vis_version))
        if dto is None:
            raise Exception("Invalid state")

        self._locations_dto_cache[vis_version] = dto
        return dto

    def get_locations(self, vis_version: VisVersion) -> Locations:
        """Get locations for a specific VIS version with caching."""
        if vis_version in self._locations_cache:
            return self._locations_cache[vis_version]
        dto = self.get_locations_dto(vis_version)
        location = Locations(vis_version, dto)
        self._locations_cache[vis_version] = location
        return location

    def get_locations_map(
        self, vis_versions: list[VisVersion]
    ) -> dict[VisVersion, Locations]:
        """Get a mapping of VIS versions to their locations."""
        return {
            vis_version: self.get_locations(vis_version) for vis_version in vis_versions
        }

    def get_vis_versions(self) -> list[VisVersion]:
        """Get all available VIS versions."""
        return list(VisVersion)

    def convert_node(
        self,
        source_version: VisVersion,
        source_node: GmodNode,
        target_version: VisVersion,
    ) -> GmodNode | None:
        """Convert a node from one version to another."""
        versioning = self.get_gmod_versioning()
        return versioning.convert_node(source_version, source_node, target_version)

    def convert_path(
        self,
        source_version: VisVersion,
        source_path: GmodPath,
        target_version: VisVersion,
    ) -> GmodPath | None:
        """Convert a path form one version to another."""
        versioning = self.get_gmod_versioning()
        return versioning.convert_path(source_version, source_path, target_version)

    def convert_local_id(
        self,
        source_local_id: LocalId,
        target_version: VisVersion,
    ) -> LocalId | None:
        """Convert a LocalId from one version to another."""
        # If versions match, no conversion needed
        if source_local_id.vis_version == target_version:
            return source_local_id

        # First try with versioning
        versioning = self.get_gmod_versioning()
        return versioning.convert_local_id_instance(source_local_id, target_version)

    def convert_local_id_builder(
        self,
        source_local_id_builder: LocalIdBuilder,
        target_version: VisVersion,
    ) -> LocalIdBuilder | None:
        """Convert a LocalIdBuilder from one version to another."""
        # Check if the builder has a version
        if source_local_id_builder.vis_version is None:
            return None

        # Try to get versioning data, which may return empty versioning if no file
        versioning = self.get_gmod_versioning()
        # Try conversion using versioning data
        return versioning.convert_local_id(source_local_id_builder, target_version)

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

    @staticmethod
    def match_iso_local_id_string(value: str) -> bool:
        """Check if a local ID string follows ISO string format.

        Rules according to: "ISO19848 5.2.1, Note 1" and "RFC3986 2.3 - Unreserved characters"
        Allows forward slashes as path separators in addition to ISO string characters.
        """  # noqa: E501
        for char in value:
            if char == "/":
                continue
            if not VIS.is_iso_string(char):
                return False
        return True
