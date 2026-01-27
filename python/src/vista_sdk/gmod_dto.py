"""Data transfer objects for Generic Product Model (GMOD)."""

from __future__ import annotations

from pydantic import BaseModel, Field


class GmodNodeDto(BaseModel):
    """Data transfer object for a GMOD node.

    Represents a node in the Generic Product Model (GMOD) tree.
    """

    category: str = Field(..., alias="category")
    type: str = Field(..., alias="type")
    code: str = Field(..., alias="code")
    name: str = Field("", alias="name")
    common_name: str | None = Field(None, alias="commonName")
    definition: str | None = Field(None, alias="definition")
    common_definition: str | None = Field(None, alias="commonDefinition")
    install_substructure: bool | None = Field(None, alias="installSubstructure")
    normal_assignment_names: dict[str, str] | None = Field(
        None, alias="normalAssignmentNames"
    )


class GmodDto(BaseModel):
    """Data transfer object for GMOD data.

    Represents the complete Generic Product Model structure.
    """

    vis_version: str = Field(..., alias="visRelease")
    items: list[GmodNodeDto] = Field(..., alias="items")
    relations: list[list[str]] = Field(..., alias="relations")
