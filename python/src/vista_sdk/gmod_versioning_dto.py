"""Gmod Versioning Data Transfer Object (DTO) for Vista SDK.

This module defines the data structures for handling versioning changes in
General Maritime Object Data (GMOD).
"""

from __future__ import annotations

from collections.abc import Set

from pydantic import BaseModel, Field


class GmodNodeConversionDto(BaseModel):
    """DTO For GMOD Node Conversion."""

    operations: Set[str] = Field(..., alias="operation")
    source: str = Field(..., alias="source")
    target: str = Field(..., alias="target")
    old_assignment: str = Field(..., alias="old_assignment")
    new_assignment: str = Field(..., alias="new_assignment")
    delete_assignment: bool = Field(..., alias="delete_assignment")


class GmodVersioningAssignmentChangeDto(BaseModel):
    """DTO for change in GMOD Version."""

    old_assignment: str = Field(..., alias="old_assignment")
    current_assignment: str = Field(..., alias="current_assignment")


class GmodVersioningDto(BaseModel):
    """Data transfer object for GMOD Versioning."""

    vis_version: str = Field(..., alias="vis_version")
    items: dict[str, GmodNodeConversionDto] = Field(..., alias="items")
