"""Vessel Information Structure (VIS) implementation module.

This module provides the main VIS interface and implementation for accessing and
managing vessel information structures, including General Maritime Object Data
(GMOD) and locations.
"""

from __future__ import annotations

import logging
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

    LatestVisVersion = VisVersion.v3_10a
    _locations_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)  # TTL is in seconds
    _locations_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_versioning_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _gmod_versioning_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _codebooks_dto_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)
    _codebooks_cache: TTLCache = TTLCache(maxsize=10, ttl=3600)

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
        return self.client.get_gmod(vis_version_str)

    def get_gmod(self, vis_version: VisVersion) -> Gmod:
        """Get GMOD for a specific VIS version with caching."""
        if not VisVersionExtension.is_valid(vis_version):
            raise ValueError(f"Invalid VIS version: {vis_version}")
        if vis_version not in self._gmod_cache:
            self._gmod_cache[vis_version] = self.create_gmod(vis_version)

        return self._gmod_cache[vis_version]

    def get_gmods_map(self) -> dict[VisVersion, Gmod]:
        """Get a mapping of all VIS versions to their GMODs."""
        return {version: self.get_gmod(version) for version in VisVersion}

    def create_gmod(self, vis_version: VisVersion) -> Gmod:
        """Create a new GMOD instance."""
        from .gmod import Gmod

        dto = self.get_gmod_dto(vis_version)
        return Gmod(vis_version, dto)

    def get_gmod_versioning_dto(self, vis_version: VisVersion) -> GmodVersioningDto:
        """Get GMOD versioning DTO with caching."""
        try:
            if not VisVersionExtension.is_valid(vis_version):
                raise ValueError(f"Invalid VIS version: {vis_version}")

            if vis_version in self._gmod_versioning_dto_cache:
                return self._gmod_versioning_dto_cache[vis_version]

            version = VisVersionExtension.to_version_string(vis_version)

            dto = self.client.get_gmod_versioning(vis_version=version)

            if dto is None:
                raise Exception(f"Failed to load versioning DTO for version {version}")

            if not isinstance(dto, GmodVersioningDto):
                dto = GmodVersioningDto(**dto)

            self._gmod_versioning_dto_cache[vis_version] = dto
            return dto
        except Exception as e:
            raise ValueError(
                f"Error getting GMOD versioning DTO for version {vis_version}"
            ) from e

    def get_gmod_versioning(self, vis_version: VisVersion) -> GmodVersioning:
        """Get GMOD versioning with caching."""
        if not VisVersionExtension.is_valid(vis_version):
            raise ValueError(f"Invalid VIS version: {vis_version}")

        if vis_version in self._gmod_versioning_cache:
            return self._gmod_versioning_cache[vis_version]

        try:
            # First try to get the versioning DTO
            dto = self.get_gmod_versioning_dto(vis_version)
            versioning = GmodVersioning({vis_version.value: dto})
            self._gmod_versioning_cache[vis_version] = versioning
            return versioning
        except FileNotFoundError:
            versioning = GmodVersioning({})
            self._gmod_versioning_cache[vis_version] = versioning
            return versioning
        except Exception as e:
            logging.warning(f"Error getting GMOD versioning for {vis_version}: {e}")
            versioning = GmodVersioning({})
            self._gmod_versioning_cache[vis_version] = versioning
            return versioning

    def get_codebooks_dto(self, vis_version: VisVersion) -> CodebooksDto:
        """Get codebooks DTO for a specific VIS version with caching."""
        if vis_version in self._codebooks_dto_cache:
            return self._codebooks_dto_cache[vis_version]

        dto = self.client.get_codebooks(
            VisVersionExtension.to_version_string(vis_version)
        )

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

    def get_codebooks_map(self) -> dict[VisVersion, Codebooks]:
        """Get a mapping of VIS versions to their codebooks."""
        return {version: self.get_codebooks(version) for version in VisVersion}

    def get_locations_dto(self, vis_version: VisVersion) -> LocationsDto:
        """Get locations DTO for a specific VIS version with caching."""
        if vis_version in self._locations_dto_cache:
            return self._locations_dto_cache[vis_version]

        dto = self.client.get_locations(
            VisVersionExtension.to_version_string(vis_version)
        )
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

    def get_locations_map(self, vis_version: VisVersion) -> dict[VisVersion, Locations]:
        """Get a mapping of a single VIS version to its locations."""
        if vis_version.name not in VisVersion.__members__:
            raise ValueError(f"Invalid VIS version provided: {vis_version}")

        return {vis_version: self.get_locations(vis_version)}

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
        # The versioning data should be loaded based on target version transitions
        # We need to build a versioning map that contains all the conversion rules
        versioning_data = {}

        # Collect versioning DTOs for all intermediate versions
        current_version = source_version
        while current_version._value_ < target_version._value_:
            next_version = self._get_next_version(current_version)
            if next_version is None:
                break
            try:
                dto = self.get_gmod_versioning_dto(next_version)
                versioning_data[next_version.value] = dto
            except Exception:
                # No versioning file for this version, skip
                logging.debug(f"No versioning file for {next_version}")
            current_version = next_version

        from vista_sdk.gmod_versioning import GmodVersioning

        versioning = GmodVersioning(versioning_data)
        return versioning.convert_node(source_version, source_node, target_version)

    def _get_next_version(self, version: VisVersion) -> VisVersion | None:
        """Get the next version in the sequence."""
        version_sequence = {
            VisVersion.v3_4a: VisVersion.v3_5a,
            VisVersion.v3_5a: VisVersion.v3_6a,
            VisVersion.v3_6a: VisVersion.v3_7a,
            VisVersion.v3_7a: VisVersion.v3_8a,
            VisVersion.v3_8a: None,
        }
        return version_sequence.get(version)

    def convert_path(
        self,
        source_version: VisVersion,
        source_path: GmodPath,
        target_version: VisVersion,
    ) -> GmodPath | None:
        """Convert a path form one version to another."""
        try:
            # Build versioning data for all intermediate versions
            versioning_data = {}
            current_version = source_version
            while current_version._value_ < target_version._value_:
                next_version = self._get_next_version(current_version)
                if next_version is None:
                    break
                try:
                    dto = self.get_gmod_versioning_dto(next_version)
                    versioning_data[next_version.value] = dto
                except Exception:
                    logging.debug(f"No versioning file for {next_version}")
                current_version = next_version

            from vista_sdk.gmod_versioning import GmodVersioning

            versioning = GmodVersioning(versioning_data)
            return versioning.convert_path(source_version, source_path, target_version)
        except Exception as e:
            print(f"Error converting path: {e}")
            return None

    def _direct_convert_local_id(
        self, source_local_id: LocalId, target_version: VisVersion
    ) -> LocalId:
        """Directly convert a LocalId to a new version without versioning."""
        # Convert the builder and build a new LocalId
        converted_builder = self._direct_convert_local_id_builder(
            source_local_id.builder, target_version
        )
        return converted_builder.build()

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
        try:
            versioning = self.get_gmod_versioning(source_local_id.vis_version)
            converted = versioning.convert_local_id_instance(
                source_local_id, target_version
            )
            if converted:
                return converted
        except FileNotFoundError as e:
            # File not found, will try direct conversion
            logging.debug("Versioning file not found: %s. Trying direct conversion.", e)
        except Exception as e:
            # Other errors, will try direct conversion
            logging.debug(
                "Error in versioning conversion: %s. Trying direct conversion.", e
            )

        # If we get here, either versioning failed or an exception occurred
        # Try direct conversion as a fallback
        try:
            return self._direct_convert_local_id(source_local_id, target_version)
        except Exception as e:
            logging.debug("Direct conversion failed: %s", e)
            return None

    def _direct_convert_local_id_builder(
        self, source: LocalIdBuilder, target_version: VisVersion
    ) -> LocalIdBuilder:
        """Directly convert a LocalIdBuilder to a new version without versioning."""
        # Create a new builder with the target version
        builder = LocalIdBuilder.create(target_version)

        # Copy all properties from the original
        if source.primary_item:
            builder = builder.with_primary_item(source.primary_item)
        if source.secondary_item:
            builder = builder.with_secondary_item(source.secondary_item)

        # Copy verbose mode and then all metadata tags
        return (
            builder.with_verbose_mode(source.verbose_mode)
            .try_with_metadata_tag(source.quantity)[0]
            .try_with_metadata_tag(source.content)[0]
            .try_with_metadata_tag(source.calculation)[0]
            .try_with_metadata_tag(source.state)[0]
            .try_with_metadata_tag(source.command)[0]
            .try_with_metadata_tag(source.type)[0]
            .try_with_metadata_tag(source.position)[0]
            .try_with_metadata_tag(source.detail)[0]
        )

    def convert_local_id_builder(
        self,
        source_local_id_builder: LocalIdBuilder,
        target_version: VisVersion,
    ) -> LocalIdBuilder | None:
        """Convert a LocalIdBuilder from one version to another."""
        # Check if the builder has a version
        if source_local_id_builder.vis_version is None:
            return None

        try:
            # Try to get versioning data, which may return empty versioning if no file
            versioning = self.get_gmod_versioning(source_local_id_builder.vis_version)
            # Try conversion using versioning data
            converted = versioning.convert_local_id(
                source_local_id_builder, target_version
            )

            # If conversion succeeded, return it
            if converted is not None:
                return converted

            # If conversion failed but versions differ, try direct conversion
            if source_local_id_builder.vis_version != target_version:
                return self._direct_convert_local_id_builder(
                    source_local_id_builder, target_version
                )

            # If no conversion was possible, return None
            return None

        except Exception as e:
            # Conversion failed, log and return None
            logging.debug("Error converting local ID builder: %s", e)
            return None

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
