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
    def get_gmod_versioning() -> dict[str, GmodVersioningDto]:
        """Retrieve GMOD Versioning data for all available VIS versions.

        Returns:
            Dictionary mapping source version strings to their GmodVersioningDto.
        """
        result: dict[str, GmodVersioningDto] = {}

        # Get all versioning files from resources
        try:
            resources_path = pkg_resources.files("vista_sdk.resources")
            versioning_files = [
                f.name
                for f in resources_path.iterdir()
                if f.name.startswith("gmod-vis-versioning-")
                and f.name.endswith(".json.gz")
            ]
        except Exception as err:
            logger.error(f"Failed to list GMOD versioning resources: {err}")
            raise

        for resource_name in versioning_files:
            # Extract version from filename: gmod-vis-versioning-3-4a.json.gz -> 3-4a
            version = resource_name.replace("gmod-vis-versioning-", "").replace(
                ".json.gz", ""
            )

            try:
                with (
                    pkg_resources.path(
                        "vista_sdk.resources", resource_name
                    ) as resource_path,
                    gzip.open(resource_path, "rt") as gzip_file,
                ):
                    data = json.load(gzip_file)

                if "items" not in data:
                    logger.warning(
                        f"Missing 'items' key in GMOD versioning data for {version}"
                    )
                    continue

                result[version] = GmodVersioningDto(
                    visRelease=data["visRelease"], items=data["items"]
                )
                logger.debug(f"Loaded GMOD versioning data for {version}")

            except FileNotFoundError:
                logger.warning(f"GMOD versioning file not found: {resource_name}")
            except Exception as err:
                logger.error(f"Failed to load GMOD versioning for {version}: {err}")

        return result

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
