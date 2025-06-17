"""GMOD versioning functionality for converting between VIS versions."""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum

from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_versioning_dto import GmodNodeConversionDto, GmodVersioningDto
from vista_sdk.vis_version import VisVersion


class ConversionType(Enum):
    """Enum for conversion types."""

    change_code = "changeCode"
    merge = "merge"
    move = "move"
    assignment_change = 20
    assignment_delete = 21


@dataclass
class GmodNodeConversion:
    """Node conversion data."""

    operations: set[ConversionType]
    source: str
    target: str | None
    old_assignment: str | None
    new_assignment: str | None
    delete_assignment: bool | None


class GmodVersioningNode:
    """Node versioning information."""

    def __init__(
        self, vis_version: VisVersion, dto: dict[str, GmodNodeConversionDto]
    ) -> None:
        """Init class."""
        self.vis_version = vis_version
        self._versioning_node_changes: dict[str, GmodNodeConversion] = {}

        for code, node_dto in dto.items():
            self._versioning_node_changes[code] = GmodNodeConversion(
                operations=set(map(self._parse_conversion_type, node_dto.operations)),
                source=node_dto.source,
                target=node_dto.target,
                old_assignment=node_dto.old_assignment,
                new_assignment=node_dto.new_assignment,
                delete_assignment=node_dto.delete_assignment,
            )

    def try_get_code_changes(self, code: str) -> GmodNodeConversion | None:
        """Get code changes if they exist."""
        return self._versioning_node_changes.get(code)

    def _parse_conversion_type(self, type_str: str) -> ConversionType:
        match type_str:
            case "changeCode":
                return ConversionType.change_code
            case "merge":
                return ConversionType.merge
            case "move":
                return ConversionType.move
            case "assignmentChange":
                return ConversionType.assignment_change
            case "assignmentDelete":
                return ConversionType.assignment_delete
            case _:
                raise ValueError(f"Invalid conversion type: {type_str}")


class GmodVersioning:
    """Handles conversion of GMOD elements between different VIS versions."""

    def __init__(self, dto: dict[str, GmodVersioningDto]) -> None:
        """Initialize with versioning data."""
        self._versionings_map: dict[VisVersion, GmodVersioningNode] = {}

        for version_str, version_dto in dto.items():
            parsed_version = VisVersion[f"v{version_str.replace('-', '_')}"]
            versioning_node = GmodVersioningNode(parsed_version, version_dto.items)
            self._versionings_map[parsed_version] = versioning_node

    def convert_node(
        self,
        source_version: VisVersion,
        source_node: GmodNode,
        target_version: VisVersion,
    ) -> GmodNode | None:
        """Convert a node from source to target version."""
        self._validate_source_and_target_versions(source_version, target_version)

        node: GmodNode | None = source_node
        current_version = source_version

        while current_version.value < target_version.value:
            if node is None:
                break

            # Get next version in sequence
            next_version = self._get_next_version(current_version)
            if next_version is None or next_version.value > target_version.value:
                break

            node = self._convert_node_internal(current_version, node, next_version)
            current_version = next_version

        return node

    def _get_next_version(self, version: VisVersion) -> VisVersion | None:
        """Get the next version in the sequence."""
        version_sequence = {
            VisVersion.v3_4a: VisVersion.v3_5a,
            VisVersion.v3_5a: VisVersion.v3_6a,
            VisVersion.v3_6a: VisVersion.v3_7a,
            VisVersion.v3_7a: VisVersion.v3_8a,
            VisVersion.v3_8a: None,
        }
        return version_sequence.get(version)

    def _convert_node_internal(
        self,
        source_version: VisVersion,
        source_node: GmodNode,
        target_version: VisVersion,
    ) -> GmodNode | None:
        """Convert node between adjacent versions."""
        from vista_sdk.vis import VIS

        self._validate_source_and_target_version_pair(source_version, target_version)
        next_code = source_node.code

        versioning_node = self._versionings_map.get(target_version)
        if versioning_node:
            change = versioning_node.try_get_code_changes(source_node.code)
            if change and change.target is not None:
                next_code = change.target

        vis = VIS()
        target_gmod = vis.get_gmod(target_version)
        found, target_node = target_gmod.try_get_node(next_code)

        if not found or target_node is None:
            return None

        location_str = source_node.location

        result = target_node.try_with_location(location_str=location_str.__str__())
        if source_node.location is not None and result.location != source_node.location:
            raise Exception("Failed to set location")
        return result

    def convert_path(  # noqa : C901
        self,
        source_version: VisVersion,
        source_path: GmodPath,
        target_version: VisVersion,
    ) -> GmodPath | None:
        """Convert a path from source to target version."""
        from vista_sdk.vis import VIS

        vis = VIS()
        # Convert the endnodes first
        target_end_node: GmodNode | None = self.convert_node(
            source_version=source_version,
            source_node=source_path.node,
            target_version=target_version,
        )

        if target_end_node is None:
            return None

        # Handle root node case
        if target_end_node.is_root():
            return GmodPath([target_end_node], target_end_node)

        # Get full path of nodes
        source_nodes = source_path.get_full_path()
        converted_nodes: list[tuple[GmodNode, GmodNode | None]] = []

        # Convert each node in the path
        for index, source_node in source_nodes:  # noqa : B007
            converted = self.convert_node(
                source_version=source_version,
                source_node=source_node,
                target_version=target_version,
            )
            converted_nodes.append((source_node, converted))

        print(f"converted_noded: {converted_nodes}")

        # Check if any conversions
        if any(conv[1] is None for conv in converted_nodes):
            return None

        # Build target path
        target_path_nodes: list[GmodNode] = []
        target_gmod = vis.get_gmod(target_version)

        for source_node, target_node in converted_nodes:  # noqa : B007
            if target_node is None:
                continue

            if not target_path_nodes:
                target_path_nodes.append(target_node)
                continue

            # Add helper nodes if needed
            prev_node = target_path_nodes[-1]
            if prev_node.is_child(target_node):
                target_path_nodes.append(target_node)
            else:
                # Find common parent
                common_parent = target_gmod.find_common_parent(prev_node, target_node)
                if common_parent is None:
                    return None

                # Add nodes to complete the path
                target_path_nodes.extend(
                    self._get_path_to_node(common_parent, target_node)
                )

        if not target_path_nodes:
            return None

        return GmodPath(
            target_path_nodes[:-1],  # Parent nodes
            target_path_nodes[-1],  # End node
        )

    def _get_path_to_node(
        self, start_node: GmodNode, end_node: GmodNode
    ) -> list[GmodNode]:
        """Get the path of nodes from start to end node."""
        path: list[GmodNode] = []
        current = end_node

        while current != start_node:
            path.insert(0, current)
            if not current.parents:
                return []
            current = current.parents[0]

        path.insert(0, start_node)
        return path

    def _validate_source_and_target_versions(
        self, source_version: VisVersion, target_version: VisVersion
    ) -> None:
        """Validate version parameters."""
        if not source_version.value:
            raise ValueError(f"Invalid source VIS Version: {source_version.value}")
        if not target_version.value:
            raise ValueError(f"Invalid target VIS Version: {target_version.value}")
        if source_version.value >= target_version.value:
            raise ValueError(
                f"Source version must be less than target version. Source version: "
                f"{source_version.value}, target version: {target_version.value}"
            )

    def _validate_source_and_target_version_pair(
        self, source_version: VisVersion, target_version: VisVersion
    ) -> None:
        """Validate version pair."""
        if source_version.value >= target_version.value:
            raise ValueError(
                f"Source version must be less than target version. Source version: "
                f"{source_version.value}, target version: {target_version.value}"
            )

    def __str__(self) -> str:
        """Return string representation of GmodVersioning.

        Returns:
            String containing version mappings and number of conversion rules
        """
        version_info = []
        for version, node in self._versionings_map.items():
            num_rules = len(node._versioning_node_changes)
            version_info.append(
                f"Version {version.value}: {num_rules} conversion rules"
            )

        return (
            f"GmodVersioning(\n"
            f"  versions: {len(self._versionings_map)}\n"
            f"  mappings:\n    " + "\n    ".join(version_info) + "\n)"
        )
