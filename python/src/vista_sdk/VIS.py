from __future__ import annotations

import os
from abc import ABC, abstractmethod
from typing import Optional

from cachetools import TTLCache  # type: ignore
from dotenv import load_dotenv

from .Client import Client
from .Gmod import Gmod
from .GmodDto import GmodDto
from .Locations import Locations
from .LocationsDto import LocationsDto
from .VisVersions import VisVersion, VisVersionExtension

load_dotenv()
environment = os.getenv("ENVIRONMENT")


class IVIS(ABC):
    @abstractmethod
    def get_locations(self, vis_version: VisVersion): ...

    @abstractmethod
    def get_locations_map(self, vis_version: VisVersion): ...

    @abstractmethod
    def get_vis_versions(self): ...

    @abstractmethod
    def get_gmod(self, vis_version: VisVersion): ...

    @abstractmethod
    def get_gmods_map(self): ...


class VIS(IVIS):
    LatestVisVersion = VisVersion.v3_7a
    # TODO: DETTE MÅ SEES PÅ
    _locations_cache = TTLCache(maxsize=10, ttl=3600)  # TTL is in seconds
    _locations_dto_cache = TTLCache(maxsize=10, ttl=3600)
    _gmod_cache = TTLCache(maxsize=10, ttl=3600)
    _gmod_dto_cache = TTLCache(maxsize=10, ttl=3600)

    client = Client()

    def __new__(cls) -> VIS:
        if not hasattr(cls, "instance"):
            cls.instance = super(VIS, cls).__new__(cls)
        return cls.instance

    def get_gmod_dto(self, vis_version: VisVersion) -> GmodDto:
        if vis_version in self._gmod_dto_cache:
            return self._gmod_dto_cache[vis_version]

        def load_and_cache():
            dto = self.load_gmod_dto(vis_version)
            if dto is None:
                raise Exception("Invalid state")

            self._gmod_dto_cache[vis_version] = dto
            return dto

        return load_and_cache()

    def load_gmod_dto(self, vis_version: VisVersion) -> Optional[GmodDto]:
        vis_version_str = VisVersionExtension.to_version_string(vis_version)
        if environment == "local":
            return self.client.get_gmod_test(vis_version_str)
        return self.client.get_gmod(vis_version_str)

    def get_gmod(self, vis_version: VisVersion) -> Gmod:
        if not VisVersionExtension.is_valid(vis_version):
            raise ValueError(f"Invalid VIS version: {vis_version}")
        if vis_version not in self._gmod_cache:
            self._gmod_cache[vis_version] = self.create_gmod(vis_version)

        return self._gmod_cache[vis_version]

    def create_gmod(self, vis_version: VisVersion) -> Gmod:
        from .Gmod import Gmod

        dto = self.get_gmod_dto(vis_version)
        return Gmod(vis_version, dto)

    def get_gmods_map(self) -> dict[VisVersion, Gmod]:
        return {version: self.get_gmod(version) for version in VisVersion}

    def get_locations(self, vis_version: VisVersion) -> Locations:
        if vis_version in self._locations_cache:
            return self._locations_cache[vis_version]
        dto = self.get_locations_dto(vis_version)
        location = Locations(vis_version, dto)
        self._locations_cache[vis_version] = location
        return location

    def get_locations_dto(self, vis_version: VisVersion) -> LocationsDto:
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

    def get_locations_map(self, vis_versions):
        invalid_versions = [
            v for v in vis_versions if v.name not in VisVersion.__members__
        ]
        if invalid_versions:
            raise ValueError(
                "Invalid VIS versions provided: "
                f"{', '.join(map(str, invalid_versions))}"
            )

        return {version: self.get_locations(version) for version in vis_versions}

    def get_vis_versions(self):
        return list(VisVersion)

    @staticmethod
    def is_iso_string(span: str) -> bool:
        return all(VIS.match_ascii_decimal(ord(char)) for char in span)

    @staticmethod
    def match_ascii_decimal(code: int) -> bool:
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
