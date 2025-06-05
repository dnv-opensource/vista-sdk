"""Relative locations data transfer objects."""

from __future__ import annotations

from pydantic import BaseModel, Field


class RelativeLocationsDto(BaseModel):
    """Data transfer object for relative locations."""
    code: str = Field(..., alias="code")
    name: str = Field(..., alias="name")
    definition: str | None = Field(None, alias="definition")


class LocationsDto(BaseModel):
    """Data transfer object for locations data."""
    vis_version: str = Field(..., alias="visRelease")
    items: list[RelativeLocationsDto] = Field(..., alias="items")
