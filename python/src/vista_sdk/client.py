"""This module implements the client for accessing VIS and related data."""

from __future__ import annotations

import gzip
import importlib.resources as pkg_resources
import json
from dataclasses import dataclass
from pathlib import Path

from vista_sdk.gmod_dto import GmodDto
from vista_sdk.locations_dto import LocationsDto


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
        except FileNotFoundError:
            return None

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
        except FileNotFoundError:
            raise Exception("Invalid state") from None

    @staticmethod
    def get_locations_test(vis_version: str) -> LocationsDto | None:
        """Retrieve test locations data for the specified VISTA version."""
        pattern = f"locations-vis-{vis_version}.json.gz"
        files = list(Path("./resources").glob(pattern))
        if len(files) != 1:
            return None

        locations_resource_name = files[0]
        with gzip.open(locations_resource_name, "rt") as file:
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
