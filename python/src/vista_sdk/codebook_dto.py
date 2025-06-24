"""Data trandfer object for the condebooks and codebook."""

from pydantic import Field


class CodeBookDto:
    """Codebook DTO."""

    name: str = Field(..., alias="name")
    values: dict[str, list[str]] = Field(..., alias="values")


class CodeBooksDto:
    """Codebooks DTO."""

    vis_release: str = Field(..., alias="visRelease")
    items: list[CodeBookDto] = Field(..., alias="items")
