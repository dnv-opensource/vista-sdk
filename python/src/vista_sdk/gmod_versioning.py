"""GMOD versioning functionality for converting between VIS versions."""

from __future__ import annotations

import logging
from dataclasses import dataclass
from enum import Enum

from vista_sdk.gmod import Gmod
from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_versioning_dto import GmodNodeConversionDto, GmodVersioningDto
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_builder import LocalIdBuilder
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
        """Convert a node from source to target version.

        Follows the C# implementation logic by processing version by version,
        applying conversion rules for each step.
        """
        self._validate_source_and_target_versions(source_version, target_version)

        node: GmodNode | None = source_node
        source = source_version

        while source._value_ < target_version._value_:
            if node is None:
                break

            target = self._get_next_version(source)
            if target is None:
                break

            node = self._convert_node_internal(source, node, target)
            source = target

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

        versioning_node = self._try_get_versioning_node(target_version)
        if versioning_node:
            change = versioning_node.try_get_code_changes(source_node.code)
            if change and change.target is not None:
                next_code = change.target

        vis = VIS()
        target_gmod = vis.get_gmod(target_version)

        found, target_node = target_gmod.try_get_node(next_code)
        if not found or target_node is None:
            found, target_node = target_gmod.try_get_node(source_node.code)
            if found and target_node is not None:
                logging.debug(
                    f"Using direct code mapping for {source_node.code} "
                    f"({source_version} -> {target_version})"
                )
            else:
                return None

        if source_node.location is not None:
            result = target_node.try_with_location(source_node.location.__str__())
            if result.location != source_node.location:
                raise Exception("Failed to set location")
            return result

        return target_node

    def convert_local_id(
        self, source_local_id: LocalIdBuilder, target_version: VisVersion
    ) -> LocalIdBuilder | None:
        """Convert a LocalIdBuilder from source to target version."""
        if source_local_id.vis_version is None:
            raise ValueError("Cannot convert local ID without a specific VIS version")

        target_local_id = LocalIdBuilder.create(target_version)

        if source_local_id.primary_item is not None:
            target_primary_item = self.convert_path(
                source_local_id.vis_version,
                source_local_id.primary_item,
                target_version,
            )
            if target_primary_item is None:
                return None
            target_local_id = target_local_id.with_primary_item(target_primary_item)

        if source_local_id.secondary_item is not None:
            target_secondary_item = self.convert_path(
                source_local_id.vis_version,
                source_local_id.secondary_item,
                target_version,
            )
            if target_secondary_item is None:
                return None
            target_local_id = target_local_id.with_secondary_item(target_secondary_item)

        # Copy over all metadata tags
        result = target_local_id.with_verbose_mode(source_local_id.verbose_mode)

        # Using tuple unpacking to handle the (builder, success)
        result, _ = result.try_with_metadata_tag(source_local_id.quantity)
        result, _ = result.try_with_metadata_tag(source_local_id.content)
        result, _ = result.try_with_metadata_tag(source_local_id.calculation)
        result, _ = result.try_with_metadata_tag(source_local_id.state)
        result, _ = result.try_with_metadata_tag(source_local_id.command)
        result, _ = result.try_with_metadata_tag(source_local_id.type)
        result, _ = result.try_with_metadata_tag(source_local_id.position)
        result, _ = result.try_with_metadata_tag(source_local_id.detail)

        return result

    def convert_local_id_instance(
        self, source_local_id: LocalId, target_version: VisVersion
    ) -> LocalId | None:
        """Convert a LocalId instance from source to target version."""
        builder = self.convert_local_id(source_local_id.builder, target_version)
        if builder is None:
            return None
        return builder.build()

    def convert_path(
        self,
        source_version: VisVersion,
        source_path: GmodPath,
        target_version: VisVersion,
    ) -> GmodPath | None:
        """Convert a GmodPath from source to target version."""
        from vista_sdk.vis import VIS

        vis = VIS()
        target_end_node = self.convert_node(
            source_version=source_version,
            source_node=source_path.node,
            target_version=target_version,
        )

        if target_end_node is None:
            return None

        if target_end_node.is_root():
            return GmodPath([], target_end_node, skip_verify=True)

        target_gmod = vis.get_gmod(target_version)

        qualifying_nodes = []
        for path_item in source_path.get_full_path():
            source_node = path_item[1]
            target_node = self.convert_node(source_version, source_node, target_version)
            if target_node is None:
                raise Exception("Could not convert node forward")
            qualifying_nodes.append((source_node, target_node))

        return self._build_path(
            qualifying_nodes, target_gmod, target_end_node, source_path
        )

    def _build_path(
        self,
        qualifying_nodes: list[tuple[GmodNode, GmodNode]],
        target_gmod: Gmod,
        target_end_node: GmodNode,
        source_path: GmodPath,
    ) -> GmodPath:
        """Build a path using C# approach for path conversion."""
        path: list[GmodNode] = []

        i = 0
        while i < len(qualifying_nodes):
            i = self._process_qualifying_node(
                qualifying_nodes, target_gmod, target_end_node, path, i
            )

        return self._finalize_path(path, source_path)

    def _process_qualifying_node(
        self,
        qualifying_nodes: list[tuple[GmodNode, GmodNode]],
        target_gmod: Gmod,
        target_end_node: GmodNode,
        path: list[GmodNode],
        i: int,
    ) -> int:
        """Process a single qualifying node and return the next index."""
        qualifying_node = qualifying_nodes[i]
        source_node = qualifying_node[0]
        target_node = qualifying_node[1]

        # Skip duplicate codes in sequence
        if i > 0 and target_node.code == qualifying_nodes[i - 1][1].code:
            return i + 1

        # Check what changed
        code_changed = source_node.code != target_node.code
        source_normal_assignment = source_node.product_type
        target_normal_assignment = target_node.product_type

        normal_assignment_changed = self._is_normal_assignment_changed(
            source_normal_assignment, target_normal_assignment
        )

        next_i = self._handle_node_changes(
            code_changed,
            normal_assignment_changed,
            source_normal_assignment,
            target_normal_assignment,
            target_node,
            target_end_node,
            qualifying_nodes,
            target_gmod,
            path,
            i,
        )

        # Break if we've reached the target end node
        if path and path[-1].code == target_end_node.code:
            return len(qualifying_nodes)

        return next_i

    def _is_normal_assignment_changed(
        self,
        source_normal_assignment: GmodNode | None,
        target_normal_assignment: GmodNode | None,
    ) -> bool:
        """Check if normal assignment has changed."""
        return (
            (source_normal_assignment is None and target_normal_assignment is not None)
            or (
                source_normal_assignment is not None
                and target_normal_assignment is None
            )
            or (
                source_normal_assignment is not None
                and target_normal_assignment is not None
                and source_normal_assignment.code != target_normal_assignment.code
            )
        )

    def _handle_node_changes(
        self,
        code_changed: bool,
        normal_assignment_changed: bool,
        source_normal_assignment: GmodNode | None,
        target_normal_assignment: GmodNode | None,
        target_node: GmodNode,
        target_end_node: GmodNode,
        qualifying_nodes: list[tuple[GmodNode, GmodNode]],
        target_gmod: Gmod,
        path: list[GmodNode],
        i: int,
    ) -> int:
        """Handle different types of node changes and return next index."""
        if code_changed:
            self._add_to_path(target_gmod, path, target_node)
            return i + 1

        if normal_assignment_changed:  # AC || AN || AD
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
                return i + 1  # Continue to next iteration

            if (
                target_node.code != target_end_node.code
                and target_normal_assignment is not None
            ):
                self._add_to_path(target_gmod, path, target_normal_assignment)
                # Skip next iteration
                return i + 2  # This matches C#'s single i++ increment

        # No changes - just add the target node
        if not code_changed and not normal_assignment_changed:
            self._add_to_path(target_gmod, path, target_node)

        return i + 1

    def _finalize_path(self, path: list[GmodNode], source_path: GmodPath) -> GmodPath:
        """Finalize and validate the constructed path."""
        if not path:
            raise Exception(f"Did not end up with valid path for {source_path}")

        potential_parents = path[:-1]
        final_node = path[-1]

        if not GmodPath.is_valid(potential_parents, final_node):
            raise Exception(f"Did not end up with valid path for {source_path}")

        return GmodPath(potential_parents, final_node)

    def _add_to_path(
        self,
        target_gmod: Gmod,
        path: list[GmodNode],
        node: GmodNode,
    ) -> None:
        """Add node to path with proper parent-child relationship verification.

        1. If path is empty, just add the node
        2. If the previous node is a child of the new node, just add it
        3. Otherwise, traverse backwards removing parents until we find a valid
           connection
        4. When we find a valid connection, add all missing intermediate nodes
        """
        if not path:
            path.append(node)
            return

        prev = path[-1]
        if prev.is_child(node):
            path.append(node)
            return

        # Traverse backwards removing parents until we find a valid connection
        for j in range(len(path) - 1, -1, -1):
            parent = path[j]
            current_parents = path[: j + 1]
            exists, remaining = target_gmod._path_exists_between(current_parents, node)

            if not exists:
                # Check asset function node constraint
                if not any(
                    n.is_asset_function_node() and n.code != parent.code
                    for n in current_parents
                ):
                    raise Exception("Tried to remove last asset function node")
                path.pop(j)
            else:
                # Found a valid connection - add all the missing intermediate nodes
                # Handle location propagation
                nodes = []
                if node.location is not None:
                    for n in remaining:
                        if not n.is_individualizable(False, True):
                            nodes.append(n)
                        else:
                            nodes.append(n.with_location(node.location.__str__()))
                else:
                    nodes.extend(remaining)
                path.extend(nodes)
                break

        # Add the target node
        path.append(node)

    def _validate_source_and_target_versions(
        self, source_version: VisVersion, target_version: VisVersion
    ) -> None:
        """Validate version parameters."""
        if not source_version or not source_version.value:
            raise ValueError(f"Invalid source VIS Version: {source_version}")
        if not target_version or not target_version.value:
            raise ValueError(f"Invalid target VIS Version: {target_version}")

        if source_version._value_ >= target_version._value_:
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
        # Check that target version is exactly one step higher
        next_version = self._get_next_version(source_version)
        if next_version != target_version:
            raise ValueError(
                f"Target version must be exactly one version higher than "
                f"source version. Source: {source_version.value}, "
                f"Target: {target_version.value}, "
                f"Expected: {next_version.value if next_version else 'None'}"
            )

    def _try_get_versioning_node(
        self, vis_version: VisVersion
    ) -> GmodVersioningNode | None:
        """Get the versioning node for a specific version if it exists."""
        return self._versionings_map.get(vis_version)

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
