from abc import ABC, abstractmethod
from typing import Optional
from cachetools import TTLCache
from .GmodDto import GmodDto
from .Locations import Locations
from .VisVersions import VisVersion, VisVersionExtension
from .Client import Client
from .Gmod import Gmod

class IVIS(ABC):
    @abstractmethod
    def get_locations(self, vis_version : VisVersion):
        pass

    @abstractmethod
    def get_locations_map(self, vis_version):
        pass

    @abstractmethod
    def get_vis_versions(self):
        pass

    @abstractmethod
    def get_gmod(self, vis_version : VisVersion):
        pass

    @abstractmethod
    def get_gmods_map(self, vis_versions):
        pass


class VIS(IVIS):
    LatestVisVersion = VisVersion.v3_7a
    """TODO: DETTE MÅ SEES PÅ"""
    _instance = None
    _locations_cache = TTLCache(maxsize=10, ttl=3600)  # TTL is in seconds
    _locations_dto_cache = TTLCache(maxsize=10, ttl=3600)
    _gmod_cache = TTLCache(maxsize=10, ttl=3600) 
    _gmod_dto_cache = TTLCache(maxsize=10, ttl=3600)
    client = Client()
    


    @classmethod
    def instance(cls):
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance
    
    def get_gmod_dto(self, vis_version: VisVersion):
      
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
        return self.client.get_gmod(vis_version_str)
    
    def get_gmod(self, vis_version: VisVersion) -> Gmod:
        if (not VisVersionExtension.is_valid(vis_version)):
            raise ValueError(f"Invalid VIS version: {vis_version}")
        if vis_version not in self._gmod_cache:
            self._gmod_cache[vis_version] = self.create_gmod(vis_version)

        return self._gmod_cache[vis_version]
    
    def create_gmod(self, vis_version : VisVersion):
       
        dto = self.get_gmod_dto(vis_version) 
        if dto is None:
            raise Exception("Failed to load GmodDto") 

        return Gmod(vis_version, dto) 


    def get_gmods_map(self, vis_versions):
        invalid_versions = [v for v in vis_versions if not v.name in VisVersion.__members__]
        if invalid_versions:
            raise ValueError(f"Invalid VIS versions provided: {', '.join(map(str, invalid_versions))}")
        
        return {version: self.get_gmod(version) for version in vis_versions}


    def get_locations(self, vis_version : VisVersion):
        if vis_version in self._locations_cache:
            print(vis_version)
            return self._locations_cache[vis_version]
        dto = self.get_locations_dto(vis_version)
        location = Locations(vis_version, dto)
        self._locations_cache[vis_version] = location
        return location

    def get_locations_dto(self, vis_version : VisVersion):
        if vis_version in self._locations_dto_cache:
            return self._locations_dto_cache[vis_version]
        
        dto = self.client.get_locations(VisVersionExtension.to_version_string(vis_version))
        print(dto)
        if dto is None:
            raise Exception("Invalid state")
        
        self._locations_dto_cache[vis_version] = dto
        return dto

    def get_locations_map(self, vis_versions):
        invalid_versions = [v for v in vis_versions if not v.name in VisVersion.__members__]
        if invalid_versions:
            raise ValueError(f"Invalid VIS versions provided: {', '.join(map(str, invalid_versions))}")
        
        return {version: self.get_locations(version) for version in vis_versions}

    def get_vis_versions(self):
        return list(VisVersion)
    
if __name__ == "__main__":
    vis = VIS.instance()
    print(vis.get_locations(VisVersion.v3_4a))
   