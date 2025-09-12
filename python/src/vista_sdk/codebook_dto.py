"""Data trandfer object for the condebooks and codebook."""

from pydantic import BaseModel, Field


class CodebookDto(BaseModel):
    """Codebook DTO."""

    name: str = Field(..., alias="name")
    values: dict[str, list[str]] = Field(..., alias="values")


class CodebooksDto(BaseModel):
    """Codebooks DTO."""

    vis_release: str = Field(..., alias="visRelease")
    items: list[CodebookDto] = Field(..., alias="items")
