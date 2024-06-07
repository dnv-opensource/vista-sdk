from __future__ import annotations
from dataclasses import dataclass
import gzip
import json
from typing import Optional
from glob import glob
from .GmodDto import GmodDto
from .LocationsDto import LocationsDto

@dataclass(frozen=True)
class Client:

    @staticmethod
    def get_locations(vis_version: str) -> Optional[LocationsDto]:
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
    def get_gmod(vis_version : str) -> GmodDto:
        pattern = f"./resources/gmod-vis-{vis_version}.json.gz"
        files = glob(pattern)
        if len(files) != 1:
            raise Exception("Invalid state")

        locations_resource_name = files[0]
        with gzip.open(locations_resource_name, 'rt') as file:
            data = json.load(file)

        gmod_dto = GmodDto(**data)  
        return gmod_dto
    
