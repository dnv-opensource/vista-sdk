from typing import Optional, List
from pydantic import BaseModel, Field

class RelativeLocationsDto(BaseModel):
    code: str = Field(..., alias='code')
    name: str = Field(..., alias='name')
    definition: Optional[str] = Field(None, alias='definition')

class LocationsDto(BaseModel):
    vis_version: str = Field(..., alias='visRelease')
    items: List[RelativeLocationsDto] = Field(..., alias='items')
