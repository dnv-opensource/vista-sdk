"""Implementation of the Codebook class and related data structures for handling metadata tags in the VISTA SDK."""  # noqa: E501

from __future__ import annotations

from collections.abc import Iterator
from dataclasses import dataclass
from enum import IntEnum

from vista_sdk.codebook_dto import CodebookDto
from vista_sdk.codebook_names import CodebookName
from vista_sdk.metadata_tag import MetadataTag


class PositionValidationResult(IntEnum):
    """Enumeration of position validation results."""

    Invalid = 0
    InvalidOrder = 1
    InvalidGrouping = 2
    Valid = 100
    Custom = 101


class CodebookStandardValues:
    """Data structure representing standard values in a codebook."""

    def __init__(self, name: CodebookName, standard_values: set[str]) -> None:
        """Initialize the CodebookStandardValues with a name and a set of standard values."""  # noqa: E501
        self.name = name
        self.standard_values = standard_values

    def __iter__(self) -> Iterator[str]:
        """Return an iterator over the standard values."""
        return iter(self.standard_values)

    def __contains__(self, tag_value: str) -> bool:
        """Check if a tag value is a standard value in the codebook."""
        if self.name == CodebookName.Position and tag_value.isdigit():
            return True
        return tag_value in self.standard_values

    def __len__(self) -> int:
        """Return the number of standard values in the codebook."""
        return len(self.standard_values)


class CodebookGroups:
    """Data structure representing groups in a codebook."""

    def __init__(self, groups: set[str]) -> None:
        """Initialize the CodebookGroups with a set of groups."""
        self.groups = groups

    def __contains__(self, group: str) -> bool:
        """Check if a group is present in the codebook groups."""
        return group in self.groups

    def __iter__(self) -> Iterator[str]:
        """Return an iterator over the groups."""
        return iter(self.groups)

    def __len__(self) -> int:
        """Return the number of groups in the codebook."""
        return len(self.groups)


@dataclass
class Codebook:
    """Data structure representing a codebook for metadata tags."""

    name: CodebookName
    raw_data: dict[str, list[str]]
    _group_map: dict[str, str]
    _standard_values: CodebookStandardValues
    _groups: CodebookGroups

    @classmethod
    def from_dto(cls, dto: CodebookDto) -> Codebook:
        """Create a Codebook instance from a DTO (Data Transfer Object)."""
        name_map = {
            "positions": CodebookName.Position,
            "calculations": CodebookName.Calculation,
            "quantities": CodebookName.Quantity,
            "states": CodebookName.State,
            "contents": CodebookName.Content,
            "commands": CodebookName.Command,
            "types": CodebookName.Type,
            "functional_services": CodebookName.FunctionalServices,
            "maintenance_category": CodebookName.MaintenanceCategory,
            "activity_type": CodebookName.ActivityType,
            "detail": CodebookName.Detail,
        }

        if dto.name not in name_map:
            raise ValueError(f"Unknown metadata tag: {dto.name}")

        name = name_map[dto.name]
        group_map = {}

        data = [
            (value.strip(), group.strip())
            for group, values in dto.values.items()
            for value in values
            if value.strip() != "<number>"
        ]

        for value, group in data:
            group_map[value] = group

        value_set = {value for value, _ in data}
        group_set = {group for _, group in data}

        return cls(
            name=name,
            raw_data=dto.values,
            _group_map=group_map,
            _standard_values=CodebookStandardValues(name, value_set),
            _groups=CodebookGroups(group_set),
        )

    @property
    def groups(self) -> CodebookGroups:
        """Return the groups defined in the codebook."""
        return self._groups

    @property
    def standard_values(self) -> CodebookStandardValues:
        """Return the standard values defined in the codebook."""
        return self._standard_values

    def has_group(self, group: str) -> bool:
        """Check if a group is defined in the codebook."""
        return group in self._groups

    def has_standard_value(self, value: str) -> bool:
        """Check if a value is a standard value in the codebook."""
        return value in self._standard_values

    def try_create_tag(self, value: str | None) -> MetadataTag | None:
        """Try to create a metadata tag from a value, validating it against the codebook."""  # noqa: E501
        from vista_sdk.vis import VIS  # noqa: PLC0415

        if not value or not value.strip():
            return None

        is_custom = False

        if self.name == CodebookName.Position:
            position_validity = self.validate_position(value)
            if int(position_validity) < 100:
                return None
            if position_validity == PositionValidationResult.Custom:
                is_custom = True
        else:
            if not VIS.is_iso_string(value):
                return None
            if self.name != CodebookName.Detail and value not in self._standard_values:
                is_custom = True

        return MetadataTag(name=self.name, value=value.strip(), is_custom=is_custom)

    def create_tag(self, value: str) -> MetadataTag:
        """Create a metadata tag from a value, validating it against the codebook."""
        tag = self.try_create_tag(value)
        if tag is None:
            raise ValueError(
                f"Invalid value for metadata tag: codebook={self.name}, {value}"
            )
        return tag

    def validate_position(self, position: str) -> PositionValidationResult:
        """Validate a position string against the codebook's position rules."""
        from vista_sdk.vis import VIS  # noqa: PLC0415

        if not position or not position.strip() or not VIS.is_iso_string(position):
            return PositionValidationResult.Invalid

        if position.strip() != position:
            return PositionValidationResult.Invalid

        if position in self._standard_values:
            return PositionValidationResult.Valid

        if position.isdigit():
            return PositionValidationResult.Valid

        if "-" not in position:
            return PositionValidationResult.Custom

        positions = position.split("-")
        validations = [self.validate_position(pos) for pos in positions]

        if any(v < PositionValidationResult.Valid for v in validations):
            return max(validations)

        number_not_at_end = any(
            pos.isdigit() and i < len(positions) - 1 for i, pos in enumerate(positions)
        )

        positions_without_numbers = [pos for pos in positions if not pos.isdigit()]
        not_alphabetically_sorted = (
            sorted(positions_without_numbers) != positions_without_numbers
        )
        if number_not_at_end or not_alphabetically_sorted:
            return PositionValidationResult.InvalidOrder

        if all(v == PositionValidationResult.Valid for v in validations):
            groups = [
                "<number>" if pos.isdigit() else self._group_map[pos]
                for pos in positions
            ]
            groups_set = set(groups)

            if "DEFAULT_GROUP" not in groups and len(groups_set) != len(groups):
                return PositionValidationResult.InvalidGrouping

        return max(validations)
