"""This module implements the client for accessing VIS and related data."""

from __future__ import annotations

import gzip
import importlib.resources as pkg_resources
import json
import logging
from dataclasses import dataclass

from vista_sdk.codebook_dto import CodebooksDto
from vista_sdk.gmod_dto import GmodDto
from vista_sdk.gmod_versioning_dto import GmodVersioningDto
from vista_sdk.locations_dto import LocationsDto

# Configure logger
logger = logging.getLogger(__name__)


@dataclass(frozen=True)
class Client:
    """Client for accessing vessel information structures (VIS) and related data."""

    @staticmethod
    def get_locations(vis_version: str) -> LocationsDto | None:
        """Retrieve locations data for the specified VISTA version."""
        resource_name = f"locations-vis-{vis_version}.json.gz"
        try:
            with (
                pkg_resources.path(
                    "vista_sdk.resources", resource_name
                ) as resource_path,
                gzip.open(resource_path, "rt") as gzip_file,
            ):
                data = json.load(gzip_file)

            return LocationsDto(**data)
        except FileNotFoundError as err:
            raise FileNotFoundError(
                f"File: {resource_name} at given path was not found"
            ) from err

    @staticmethod
    def get_gmod(vis_version: str) -> GmodDto:
        """Retrieve GMOD data for the specified VISTA version."""
        resource_name = f"gmod-vis-{vis_version}.json.gz"
        try:
            with (
                pkg_resources.path(
                    "vista_sdk.resources", resource_name
                ) as resource_path,
                gzip.open(resource_path, "rt") as gzip_file,
            ):
                data = json.load(gzip_file)
            return GmodDto(**data)
        except FileNotFoundError as err:
            raise FileNotFoundError(
                f"File: {resource_name} at given path was not found"
            ) from err

    @staticmethod
    def get_gmod_versioning(vis_version: str) -> GmodVersioningDto:
        """Retrieve GMOD Versioning data for the specified VISTA version."""
        resource_name: str = f"gmod-vis-versioning-{vis_version}.json.gz"

        logger.debug(f"Retrieving GMOD versioning data for {vis_version}")

        try:
            with (
                pkg_resources.path(
                    "vista_sdk.resources", resource_name
                ) as resource_path,
                gzip.open(resource_path, "rt") as gzip_file,
            ):
                data = json.load(gzip_file)

            logger.debug(f"Successfully loaded GMOD versioning data for {vis_version}")

            if "items" not in data:
                logger.error(
                    f"Missing 'items' key in GMOD versioning data for {vis_version}"
                )

            return GmodVersioningDto(visRelease=data["visRelease"], items=data["items"])
        except FileNotFoundError as err:
            logger.error(
                f"Failed to load GMOD versioning data: {resource_name} not found"
            )
            raise FileNotFoundError(
                f"File: {resource_name} at given path was not found"
            ) from err

    @staticmethod
    def get_codebooks(vis_version: str) -> CodebooksDto:
        """Retrieve codebooks data for the specified VISTA version."""
        resource_name = f"codebooks-vis-{vis_version}.json.gz"
        try:
            with (
                pkg_resources.path(
                    "vista_sdk.resources", resource_name
                ) as resource_path,
                gzip.open(resource_path, "rt") as gzip_file,
            ):
                data = json.load(gzip_file)
            return CodebooksDto(**data)
        except FileNotFoundError as err:
            raise FileNotFoundError(
                f"File: {resource_name} at given path was not found"
            ) from err

    @staticmethod
    def get_locations_test(vis_version: str) -> LocationsDto | None:
        """Retrieve test locations data for the specified VISTA version."""
        with pkg_resources.path(
            "vista_sdk.resources", f"locations-vis-{vis_version}.json.gz"
        ) as resource_path:
            if not resource_path.exists():
                raise FileNotFoundError(
                    f"Resource gmod-vis-{vis_version}.json.gz not found"
                )

            with gzip.open(resource_path, "rt") as file:
                data = json.load(file)

        return LocationsDto(**data)

    @staticmethod
    def get_gmod_test(vis_version: str) -> GmodDto:
        """Retrieve test GMOD data for the specified VISTA version."""
        with pkg_resources.path(
            "vista_sdk.resources", f"gmod-vis-{vis_version}.json.gz"
        ) as resource_path:
            if not resource_path.exists():
                raise FileNotFoundError(
                    f"Resource gmod-vis-{vis_version}.json.gz not found"
                )
            with gzip.open(resource_path, "rt") as file:
                data = json.load(file)

        return GmodDto(**data)

    @staticmethod
    def get_gmod_versioning_test(vis_version: str) -> GmodVersioningDto:
        """Retrieve test GMOD Versioning data for the specified Vista version."""
        with pkg_resources.path(
            "vista_sdk.resources", f"gmod-vis-versioning-{vis_version}.json.gz"
        ) as resource_path:
            if not resource_path.exists():
                raise FileNotFoundError(
                    f"Resource gmod-vis-versioning-{vis_version}.json.gz not found"
                )
            with gzip.open(resource_path, "rt") as file:
                data = json.load(file)
                # Add vis_version to the data before creating DTO
                data["vis_version"] = vis_version

        try:
            return GmodVersioningDto(
                visRelease=data["vis_version"], items=data["items"]
            )
        except FileNotFoundError as err:
            raise FileNotFoundError(
                f"File: {resource_path} at given path was not found"
            ) from err

    @staticmethod
    def get_codebooks_test(vis_version: str) -> CodebooksDto:
        """Retrieve test codebooks data for the specified VISTA version."""
        with pkg_resources.path(
            "vista_sdk.resources", f"codebooks-vis-{vis_version}.json.gz"
        ) as resource_path:
            if not resource_path.exists():
                raise FileNotFoundError(
                    f"Resource codebooks-vis-{vis_version}.json.gz not found"
                )
            with gzip.open(resource_path, "rt") as file:
                data = json.load(file)

        return CodebooksDto(**data)
