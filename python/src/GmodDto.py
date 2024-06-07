from pydantic import BaseModel, Field
from typing import Optional, Dict, List

class GmodNodeDto(BaseModel):
    category: str = Field(..., alias='category')
    type: str = Field(..., alias='type')
    code: str = Field(..., alias='code')
    name: str = Field("", alias='name')
    common_name: Optional[str] = Field(None, alias='commonName')
    definition: Optional[str] = Field(None, alias='definition')
    common_definition: Optional[str] = Field(None, alias='commonDefinition')
    install_substructure: Optional[bool] = Field(None, alias='installSubstructure')
    normal_assignment_names: Optional[Dict[str, str]] = Field(None, alias='normalAssignmentNames')

class GmodDto(BaseModel):
    vis_version: str = Field(..., alias='visRelease')
    items: List[GmodNodeDto] = Field(..., alias='items')
    relations: List[List[str]] = Field(..., alias='relations')


