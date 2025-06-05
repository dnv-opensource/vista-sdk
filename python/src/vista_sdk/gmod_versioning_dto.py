"""Gmod Versioning Data Transfer Object (DTO) for Vista SDK.

This module defines the data structures for handling versioning changes in
General Maritime Object Data (GMOD).
"""

from __future__ import annotations

from pydantic import BaseModel, Field


class GmodVersioningNodeChangesDto(BaseModel):
    """Data transfer object for changes in a GMOD node."""

    next_vis_version: str | None = Field(None, alias="nextVisVersion")
    next_code: str | None = Field(None, alias="nextCode")
    previous_vis_version: str | None = Field(None, alias="previousVisVersion")
    previous_code: str | None = Field(None, alias="previousCode")


class GmodVersioningDto(BaseModel):
    """Data transfer object for GMOD versioning changes."""

    items: dict[str, dict[str, GmodVersioningNodeChangesDto]] = Field(
        ..., alias="items"
    )
