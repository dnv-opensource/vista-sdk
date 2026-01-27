"""ISO19848 DTOs for transport module."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass
class DataChannelTypeNameDto:
    """Data channel type name DTO."""

    type: str
    description: str


@dataclass
class DataChannelTypeNamesDto:
    """Data channel type names DTO."""

    values: list[DataChannelTypeNameDto]


@dataclass
class FormatDataTypeDto:
    """Format data type DTO."""

    type: str
    description: str


@dataclass
class FormatDataTypesDto:
    """Format data types DTO."""

    values: list[FormatDataTypeDto]
