from __future__ import annotations
from dataclasses import dataclass
import gzip
import json
from typing import Optional
from glob import glob
import importlib.resources as pkg_resources
from .GmodDto import GmodDto
from .LocationsDto import LocationsDto

@dataclass(frozen=True)
class Client:

    @staticmethod
    def get_locations(vis_version: str) -> Optional[LocationsDto]:
        resource_name = f"locations-vis-{vis_version}.json.gz"
        try:
            with pkg_resources.path('vista_sdk.resources', resource_name) as resource_path:
                with gzip.open(resource_path, 'rt') as gzip_file:
                    data = json.load(gzip_file)
            locations_dto = LocationsDto(**data)
            return locations_dto
        except FileNotFoundError:
            return None

    @staticmethod
    def get_gmod(vis_version: str) -> GmodDto:
        resource_name = f"gmod-vis-{vis_version}.json.gz"
        try:
            with pkg_resources.path('vista_sdk.resources', resource_name) as resource_path:
                with gzip.open(resource_path, 'rt') as gzip_file:
                    data = json.load(gzip_file)
            gmod_dto = GmodDto(**data)
            return gmod_dto
        except FileNotFoundError:
            raise Exception("Invalid state")
    
    @staticmethod
    def get_locations_test(vis_version: str) -> Optional[LocationsDto]:
        pattern = f"./resources/locations-vis-{vis_version}.json.gz"
        files = glob(pattern)
        if len(files) != 1:
            return None

        locations_resource_name = files[0]
        with gzip.open(locations_resource_name, 'rt') as file:
            data = json.load(file)

        locations_dto = LocationsDto(**data) 
        return locations_dto
    
    @staticmethod
    def get_gmod_test(vis_version : str) -> GmodDto:
        pattern = f"./resources/gmod-vis-{vis_version}.json.gz"
        files = glob(pattern)
        if len(files) != 1:
            raise Exception("Invalid state")

        locations_resource_name = files[0]
        with gzip.open(locations_resource_name, 'rt') as file:
            data = json.load(file)

        gmod_dto = GmodDto(**data)  
        return gmod_dto
    
