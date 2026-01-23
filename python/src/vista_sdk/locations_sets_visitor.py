"""Locations module for handling and parsing location codes in the VISTA SDK."""

from __future__ import annotations

from vista_sdk.gmod_node import GmodNode
from vista_sdk.locations import Location


class LocationSetsVisitor:
    """Visitor for processing sets of locations in the VISTA SDK."""

    current_parent_start: int = -1

    def visit(
        self, node: GmodNode, i: int, parents: list[GmodNode], target: GmodNode
    ) -> tuple[int, int, Location | None] | None:
        """Visit a node and determine if it is part of a location set."""
        from vista_sdk.gmod import Gmod

        is_parent = Gmod.is_potential_parent(node.metadata.type)
        is_target_node = i == len(parents)

        if self.current_parent_start == -1:
            return self._handle_first_parent(node, i, is_parent, is_target_node)

        return self._handle_existing_parent(
            node, i, parents, target, is_parent, is_target_node
        )

    def _handle_first_parent(
        self, node: GmodNode, i: int, is_parent: bool, is_target_node: bool
    ) -> tuple[int, int, Location | None] | None:
        """Handle the first parent node."""
        if is_parent:
            self.current_parent_start = i
        if node.is_individualizable(is_target_node):
            return (i, i, node.location)
        return None

    def _handle_existing_parent(
        self,
        node: GmodNode,
        i: int,
        parents: list[GmodNode],
        target: GmodNode,
        is_parent: bool,
        is_target_node: bool,
    ) -> tuple[int, int, Location | None] | None:
        """Handle nodes that are part of an existing parent set."""
        nodes = None
        if is_parent or is_target_node:
            if self.current_parent_start + 1 == i:
                if node.is_individualizable(is_target_node):
                    nodes = (i, i, node.location)
            else:
                nodes = self._collect_nodes(parents, target, i)
            self.current_parent_start = i
            if nodes and self._has_leaf_node(nodes, parents, target):
                return nodes

        if is_target_node and node.is_individualizable(is_target_node):
            return (i, i, node.location)
        return None

    def _collect_nodes(
        self, parents: list[GmodNode], target: GmodNode, i: int
    ) -> tuple[int, int, Location | None] | None:
        """Collect nodes that are part of a set."""
        nodes = None
        skipped_one = -1
        has_composition = False
        for j in range(self.current_parent_start + 1, i + 1):
            set_node = parents[j] if j < len(parents) else target
            if not set_node.is_individualizable(j == len(parents), is_in_set=True):
                if nodes is not None:
                    skipped_one = j
                continue

            if (
                nodes
                and nodes[2] is not None
                and set_node.location is not None
                and nodes[2] != set_node.location
            ):
                raise Exception(
                    f"Mapping error: different locations in "
                    "the same nodeset:"
                    f" {nodes[2]}, {set_node.location}"
                )

            if skipped_one != -1:
                raise Exception("Can't skip in the middle of individualizable set")

            if set_node.is_function_composition:
                has_composition = True

            location = nodes[2] if nodes and nodes[2] is not None else set_node.location
            start = nodes[0] if nodes else j
            end = j
            nodes = (start, end, location)

        if nodes and nodes[0] == nodes[1] and has_composition:
            nodes = None
        return nodes

    def _has_leaf_node(
        self,
        nodes: tuple[int, int, Location | None],
        parents: list[GmodNode],
        target: GmodNode,
    ) -> bool:
        """Check if the nodes set contains a leaf node."""
        for j in range(nodes[0], nodes[1] + 1):
            set_node = parents[j] if j < len(parents) else target
            if set_node.is_leaf_node or j == len(parents):
                return True
        return False
