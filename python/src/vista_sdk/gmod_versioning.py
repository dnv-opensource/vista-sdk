"""GMOD versioning functionality for converting between VIS versions."""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum

from vista_sdk.gmod import Gmod
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

    def convert_path(
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

        # Get GMOD instances
        target_gmod = vis.get_gmod(target_version)
        # source_gmod = vis.get_gmod(source_version)

        # Get full path of nodes
        source_nodes = source_path.get_full_path()
        qualifying_nodes = [
            (
                source_node,
                self.convert_node(source_version, source_node[1], target_version),
            )
            for source_node in source_nodes
        ]

        # Check if any conversions failed
        if any(conv[1] is None for conv in qualifying_nodes):
            return None

        potential_parents: list[GmodNode] = [
            n[1] for n in qualifying_nodes[:-1] if n[1] is not None
        ]
        if GmodPath.is_valid(potential_parents, target_end_node):
            return GmodPath(potential_parents, target_end_node)

        # Filter out None values for _build_path
        filtered_qualifying_nodes: list[tuple[GmodNode, GmodNode]] = [
            (src[1], tgt) for (src, tgt) in qualifying_nodes if tgt is not None
        ]

        # Build path using complex logic
        path = self._build_path(filtered_qualifying_nodes, target_gmod, target_end_node)
        if not path:
            return None

        # Validate final path
        return self._validate_final_path(path[:-1], path[-1], source_path)

    def _build_path(  # noqa : C901
        self,
        qualifying_nodes: list[tuple[GmodNode, GmodNode]],
        target_gmod: Gmod,
        target_end_node: GmodNode,
    ) -> list[GmodNode]:
        """Build valid path from qualifying nodes."""
        path: list[GmodNode] = []

        for i, (source_node, target_node) in enumerate(qualifying_nodes):
            if i > 0 and target_node.code == qualifying_nodes[i - 1][1].code:
                continue

            code_changed = source_node.code != target_node.code
            source_normal_assignment = getattr(source_node, "product_type", None)
            target_normal_assignment = getattr(target_node, "product_type", None)

            normal_assignment_changed = (
                source_normal_assignment is not None
                and target_normal_assignment is not None
                and source_normal_assignment.code != target_normal_assignment.code
            )

            if code_changed:
                self._add_to_path(target_gmod, path, target_node)
            elif normal_assignment_changed:
                was_deleted = (
                    source_normal_assignment is not None
                    and target_normal_assignment is None
                )

                if not code_changed:
                    self._add_to_path(target_gmod, path, target_node)

                if was_deleted:
                    if target_node.code == target_end_node.code and i + 1 < len(
                        qualifying_nodes
                    ):
                        next_node = qualifying_nodes[i + 1][1]
                        if next_node.code != target_node.code:
                            raise Exception("Normal assignment end node was deleted")
                    continue

                if (
                    target_node.code != target_end_node.code
                    and target_normal_assignment is not None
                ):
                    self._add_to_path(target_gmod, path, target_normal_assignment)
                    i += 1
            elif not code_changed and not normal_assignment_changed:
                self._add_to_path(target_gmod, path, target_node)

            if path and path[-1].code == target_end_node.code:
                break

        if not path or path[-1].code != target_end_node.code:
            raise ValueError(f"Failed to build complete path to {target_end_node.code}")

        return path

    def _add_to_path(
        self,
        target_gmod: Gmod,
        path: list[GmodNode],
        node: GmodNode,
    ) -> None:
        """Add node to path with proper parent-child relationship verification."""
        try:
            if not path:
                path.append(node)
                return

            prev = path[-1]
            if prev.is_child(node):
                path.append(node)
                return

            # Traverse backwards to find valid connection point
            for j in range(len(path) - 1, -1, -1):
                parent = path[j]
                current_parents = path[: j + 1]
                exists, remaining = self.path_exists_between(
                    current_parents, node, target_gmod, include_remaining=True
                )

                if not exists:
                    # Check asset function node constraint
                    has_other_asset_nodes = any(
                        n.is_asset_function_node is True and n.code != parent.code
                        for n in current_parents
                    )
                    if not has_other_asset_nodes:
                        raise ValueError("Cannot remove last asset function node")
                    path.pop(j)
                    continue

                # Handle location propagation with individualizable check
                nodes_to_add = []
                if node.location is not None:
                    for remaining_node in remaining:
                        if not remaining_node.is_individualizable(False, True):
                            nodes_to_add.append(remaining_node)
                        else:
                            nodes_to_add.append(
                                remaining_node.try_with_location(str(node.location))
                            )
                else:
                    nodes_to_add.extend(remaining)

                path.extend(nodes_to_add)
                break

            path.append(node)

        except Exception as e:
            raise ValueError(f"Error adding node to path: {e}") from e

    def path_exists_between(
        self,
        current_parents: list[GmodNode],
        target_node: GmodNode,
        target_gmod: Gmod,
        include_remaining: bool = False,
    ) -> tuple[bool, list[GmodNode]]:
        """Check if path exists between current parents and target node."""
        if not current_parents:
            return False, []

        # Find path through common parent
        last_parent = current_parents[-1]
        if last_parent.is_child(target_node):
            return True, [target_node] if include_remaining else []

        common_parent = target_gmod.find_common_parent(last_parent, target_node)
        if not common_parent:
            return False, []

        remaining_nodes = self._get_path_to_node(common_parent, target_node)
        return True, remaining_nodes

    def _validate_final_path(
        self, parents: list[GmodNode], end_node: GmodNode, source_path: GmodPath
    ) -> GmodPath:
        """Validate and create final path."""
        if not GmodPath.is_valid(parents, end_node):
            raise Exception(f"Did not end up with valid path for {source_path}")
        return GmodPath(parents, end_node)

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
        if not source_version or not source_version.value:
            raise ValueError(f"Invalid source VIS Version: {source_version}")
        if not target_version or not target_version.value:
            raise ValueError(f"Invalid target VIS Version: {target_version}")

        source_num: float = 0.0
        target_num: float = 0.0
        if "_" in source_version.value or "_" in target_version.value:
            print("'_' in version")
            source_num = float(
                source_version.value.replace("_", ".").replace("a", "").replace("v", "")
            )
            target_num = float(
                target_version.value.replace("_", ".").replace("a", "").replace("v", "")
            )

        elif "-" in source_version.value or "-" in target_version.value:
            source_num = float(
                source_version.value.replace("-", ".").replace("a", "").replace("v", "")
            )

            target_num = float(
                target_version.value.replace("-", ".").replace("a", "").replace("v", "")
            )
        else:
            raise ValueError(
                f"Could not convert version to integer: {source_version.value},"
                f" and {target_version.value}"
            )

        if source_num >= target_num:
            raise ValueError(
                f"Source version ({source_version.value}) must be less than "
                f"target version ({target_version.value})"
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
