"""Codebooks class for managing collections of codebooks in the VISTA SDK."""

from __future__ import annotations

from collections.abc import Iterator
from typing import Any

from vista_sdk.codebook import Codebook, MetadataTag
from vista_sdk.codebook_names import CodebookName
from vista_sdk.vis_version import VisVersion


class CodebooksDto:
    """Data Transfer Object for Codebooks."""

    def __init__(self, items: list[dict[str, Any]]) -> None:
        """Initialize with a list of codebook data items.

        Args:
            items: List of dictionaries containing codebook data
        """
        self.items = items


class Codebooks:
    """Collection of codebooks with indexed access and enumeration capabilities."""

    def __init__(self, vis_version: VisVersion, dto: CodebooksDto) -> None:
        """Initialize with VIS version and codebooks data.

        Args:
            vis_version: The VIS version
            dto: Data transfer object containing codebook data
        """
        self.vis_version = vis_version

        # Create an array to hold all codebooks, indexed by enum value - 1
        self._codebooks: list[Codebook | None] = [None] * len(list(CodebookName))

        # Process each codebook from the DTO
        for type_dto in dto.items:
            type_obj = Codebook.from_dto(type_dto)
            self._codebooks[type_obj.name.value - 1] = type_obj

        # Create and add the details codebook with empty values
        details_dto = {"name": "detail", "values": {}}
        details_codebook = Codebook.from_dto(details_dto)
        self._codebooks[details_codebook.name.value - 1] = details_codebook

    def __getitem__(self, name: CodebookName) -> Codebook:
        """Get a codebook by name using indexer syntax."""
        index = name.value - 1
        if index >= len(self._codebooks):
            raise ValueError(f"Invalid codebook name: {name}")

        codebook = self._codebooks[index]
        if codebook is None:
            raise ValueError(f"Codebook not found: {name}")
        return codebook

    def try_create_tag(
        self, name: CodebookName, value: str | None
    ) -> MetadataTag | None:
        """Try to create a metadata tag for the given codebook and value."""
        return self[name].try_create_tag(value)

    def create_tag(self, name: CodebookName, value: str) -> MetadataTag:
        """Create a metadata tag for the given codebook and value."""
        return self[name].create_tag(value)

    def get_codebook(self, name: CodebookName) -> Codebook:
        """Get a codebook by name."""
        return self[name]

    def __iter__(self) -> Iterator[tuple[CodebookName, Codebook]]:
        """Iterate over all codebooks as (name, codebook) tuples."""
        for codebook in self._codebooks:
            if codebook is not None:
                yield (codebook.name, codebook)
