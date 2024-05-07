import gzip
import json
import os
from typing import Optional
from glob import glob

from src.GmodDto import GmodDto
from src.LocationsDto import LocationsDto

class Client:

    def get_locations(self, vis_version: str) -> Optional[LocationsDto]:
        directory = "./resources"
        pattern = f"{directory}/locations-vis-{vis_version}.json.gz"
        files = glob(pattern)
        if len(files) != 1:
            return None

        locations_resource_name = files[0]
        with gzip.open(locations_resource_name, 'rt') as file:
            data = json.load(file)

        locations_dto = LocationsDto(**data) 
        return locations_dto
    
    def get_gmod(self, vis_version : str) -> Optional[GmodDto]:
        directory = "./resources"
        pattern = f"{directory}/gmod-vis-{vis_version}.json.gz"
        files = glob(pattern)
        if len(files) != 1:
            return None 

        locations_resource_name = files[0]
        with gzip.open(locations_resource_name, 'rt') as file:
            data = json.load(file)

        gmod_dto = GmodDto(**data)  
        return gmod_dto

