"""Gmod Versioning Data Transfer Object (DTO) for Vista SDK.

This module defines the data structures for handling versioning changes in
Generic Product Model (GMOD).
"""

from __future__ import annotations

from collections.abc import Set

from pydantic import BaseModel, Field


class GmodNodeConversionDto(BaseModel):
    """DTO For GMOD Node Conversion."""

    operations: Set[str] = Field(..., alias="operations")
    source: str = Field(..., alias="source")
    target: str | None = Field(None, alias="target")
    old_assignment: str | None = Field(None, alias="oldAssignment")
    new_assignment: str | None = Field(None, alias="newAssignment")
    delete_assignment: bool | None = Field(None, alias="deleteAssignment")


class GmodVersioningAssignmentChangeDto(BaseModel):
    """DTO for change in GMOD Version."""

    old_assignment: str = Field(..., alias="oldAssignment")
    current_assignment: str = Field(..., alias="currentAssignment")


class GmodVersioningDto(BaseModel):
    """Data transfer object for GMOD Versioning."""

    vis_version: str = Field(..., alias="visRelease")
    items: dict[str, GmodNodeConversionDto] = Field(..., alias="items")
