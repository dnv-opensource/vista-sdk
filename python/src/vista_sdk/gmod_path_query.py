"""Vista SDK: Gmod Path Query Module."""

from abc import ABC
from collections.abc import Callable
from itertools import chain
from typing import TypeVar

from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.locations import Location
from vista_sdk.vis import VIS

T = TypeVar("T", bound="GmodPathQueryBuilder")


class GmodPathQuery:
    """A query for matching GmodPath objects based on specified criteria."""

    def __init__(self, builder: "GmodPathQueryBuilder") -> None:
        """Initialize a GmodPathQuery with a builder."""
        self.builder = builder

    def match(self, other: GmodPath | None) -> bool:
        """Check if the query matches the provided GmodPath."""
        return self.builder.match(other)


class NodeItem:
    """Represents a node item in the GmodPathQueryBuilder."""

    def __init__(self, node: GmodNode, locations: set[Location]) -> None:
        """Initialize a NodeItem with a GmodNode and locations."""
        self.node = node
        self.locations = locations
        self.match_all_locations = False
        self.ignore_in_matching = False

    def clone(self) -> "NodeItem":
        """Create a deep copy of this NodeItem."""
        cloned = NodeItem(self.node, set(self.locations))
        cloned.match_all_locations = self.match_all_locations
        cloned.ignore_in_matching = self.ignore_in_matching
        return cloned


class GmodPathQueryBuilder(ABC):  # noqa: B024
    """A builder for creating GmodPathQuery objects."""

    def __init__(self) -> None:
        """Initialize a GmodPathQueryBuilder."""
        self._filter: dict[str, NodeItem] = {}

    @staticmethod
    def empty() -> "Nodes":
        """Create an empty Nodes query builder."""
        return Nodes()

    @staticmethod
    def from_path(path: GmodPath) -> "Path":
        """Create a Path query builder from a GmodPath."""
        return Path(path)

    def build(self) -> GmodPathQuery:
        """Build and return a GmodPathQuery from the current builder state."""
        return GmodPathQuery(self)

    def _ensure_path_version(self, path: GmodPath) -> GmodPath:
        p: GmodPath = path
        vis = VIS()
        if p.node.vis_version is None:
            raise ValueError("GmodPath must have a valid VIS version")

        # Don't convert if already at or newer than latest version
        if p.node.vis_version.value >= vis.latest_vis_version.value:
            return p

        # Attempt version conversion
        converted_path = vis.convert_path(p.node.vis_version, p, vis.latest_vis_version)

        # If conversion fails, fall back to original
        if converted_path is None:
            raise ValueError(f"Failed to convert GmodPath({p!s}) to latest VIS version")

        return converted_path

    def _ensure_node_version(self, node: GmodNode) -> GmodNode:
        n = node
        vis = VIS()
        if n.vis_version is None:
            raise ValueError("GmodNode must have a valid VIS version")

        # Don't convert if already at or newer than latest version
        if n.vis_version.value >= vis.latest_vis_version.value:
            return n

        # Only convert if the target version is actually newer
        converted_node = vis.convert_node(n.vis_version, node, vis.latest_vis_version)
        if converted_node is None:
            raise ValueError(f"Failed to convert GmodNode({n!s}) to latest VIS version")

        return converted_node

    def match(self, other: GmodPath | None) -> bool:  # noqa : C901
        """Check if the query matches the provided GmodPath."""
        if other is None:
            return False
        target = self._ensure_path_version(other)

        target_nodes: dict[str, list[Location]] = {}
        nodes = list(chain(target.parents, [target.node]))
        for node in nodes:
            if node.code not in target_nodes:
                target_nodes[node.code] = []

            if node.location is not None:
                target_nodes[node.code].append(node.location)

        for _, item in self._filter.items():
            node = self._ensure_node_version(item.node)

            # Skip nodes marked as ignorable
            if item.ignore_in_matching:
                continue

            if node.code not in target_nodes:
                return False

            potential_locations = target_nodes[node.code]

            if item.match_all_locations:
                continue

            if len(item.locations) > 0:
                if len(potential_locations) == 0:
                    return False
                if not any(loc in item.locations for loc in potential_locations):
                    return False
            else:
                if len(potential_locations) > 0:
                    return False

        return True


class Path(GmodPathQueryBuilder):
    """A query builder for GmodPath objects with specific nodes."""

    def __init__(self, path: GmodPath) -> None:
        """Initialize a Path query builder with a GmodPath."""
        super().__init__()
        self._set_nodes: dict[str, GmodNode] = {}
        self.gmod_path = self._ensure_path_version(path)

        for s in self.gmod_path.individualizable_sets:
            set_node = s.nodes[-1]  # Last node in Python
            self._set_nodes[set_node.code] = set_node

            locations: set[Location] = set()
            if s.location is not None:
                locations.add(s.location)

            self._filter[set_node.code] = NodeItem(set_node, locations)

        self._nodes: dict[str, GmodNode] = {
            node.code: node for _, node in self.gmod_path.get_full_path()
        }

    def _clone(self) -> "Path":
        """Create a shallow copy of this Path with deep-copied filter."""
        cloned = object.__new__(Path)
        cloned._filter = {k: v.clone() for k, v in self._filter.items()}
        cloned._set_nodes = dict(self._set_nodes)
        cloned._nodes = dict(self._nodes)
        cloned.gmod_path = self.gmod_path
        return cloned

    def with_node(
        self,
        select: Callable[[dict[str, GmodNode]], GmodNode],
        match_all_locations: bool = False,
        *locations: Location | None,
    ) -> "Path":
        """Add a node to the query with optional locations."""
        node = select(self._set_nodes)

        if node.code not in self._filter:
            raise Exception("Expected to find a filter on the node in the path")

        cloned = self._clone()
        item = cloned._filter[node.code]

        if locations:
            item.locations = {loc for loc in locations if loc is not None}
        else:
            item.locations = set()
            item.match_all_locations = match_all_locations

        return cloned

    def with_any_node_before(
        self,
        select: Callable[[dict[str, GmodNode]], GmodNode],
    ) -> "Path":
        """Mark all nodes before the selected node as ignorable in matching."""
        node = select(self._nodes)
        return self._with_any_node_before_internal(node)

    def _with_any_node_before_internal(self, node: GmodNode) -> "Path":
        """Internal method to mark nodes before the given node as ignorable."""
        full_path = self.gmod_path.get_full_path()
        if not any(path_node.code == node.code for _, path_node in full_path):
            raise ValueError(f"Node {node.code} is not in the path")

        cloned = self._clone()

        for _, path_node in full_path:
            if path_node.code == node.code:
                break
            if path_node.code not in cloned._filter:
                continue
            cloned._filter[path_node.code].ignore_in_matching = True

        return cloned

    def without_locations(self) -> "Path":
        """Remove all locations from the query."""
        cloned = self._clone()

        for item in cloned._filter.values():
            item.locations = set()
            item.match_all_locations = True

        return cloned


class Nodes(GmodPathQueryBuilder):
    """A query builder for GmodNode objects with specific locations."""

    def __init__(self) -> None:
        """Initialize a Nodes query builder."""
        super().__init__()

    def _clone(self) -> "Nodes":
        """Create a shallow copy of this Nodes with deep-copied filter."""
        cloned = Nodes()
        cloned._filter = {k: v.clone() for k, v in self._filter.items()}
        return cloned

    def with_node(
        self,
        node: GmodNode,
        match_all_locations: bool = False,
        *locations: Location | None,
    ) -> "Nodes":
        """Add a node to the query with optional locations."""
        n = self._ensure_node_version(node)
        cloned = self._clone()

        if locations:
            new_locations = {loc for loc in locations if loc}
            if n.code in cloned._filter:
                cloned._filter[n.code].locations = new_locations
            else:
                cloned._filter[n.code] = NodeItem(n, new_locations)
        else:
            if n.code in cloned._filter:
                item = cloned._filter[n.code]
                item.locations = set()
                item.match_all_locations = match_all_locations
            else:
                item = NodeItem(n, set())
                item.match_all_locations = match_all_locations
                cloned._filter[n.code] = item

        return cloned
