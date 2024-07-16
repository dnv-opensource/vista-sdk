from pydantic import BaseModel, Field
from typing import Optional, Dict

class GmodVersioningNodeChangesDto(BaseModel):
    next_vis_version: Optional[str] = Field(None, alias="nextVisVersion")
    next_code: Optional[str] = Field(None, alias="nextCode")
    previous_vis_version: Optional[str] = Field(None, alias="previousVisVersion")
    previous_code: Optional[str] = Field(None, alias="previousCode")

class GmodVersioningDto(BaseModel):
    items: Dict[str, Dict[str, GmodVersioningNodeChangesDto]] = Field(..., alias="items")