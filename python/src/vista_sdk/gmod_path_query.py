"""Vista SDK: Gmod Path Query Module."""

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

    def __init__(self, node: str, locations: set[Location]) -> None:
        """Initialize a NodeItem with a node code and locations."""
        self.node = node
        self.locations = locations
        self.match_all_locations = False


class GmodPathQueryBuilder:
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
        if p.node.vis_version.value >= vis.LatestVisVersion.value:
            return p

        # Attempt version conversion
        converted_path = vis.convert_path(p.node.vis_version, p, vis.LatestVisVersion)

        # If conversion fails, fall back to original
        if converted_path is None:
            return p

        # Verify that conversion preserved the essential structure
        # Check if important nodes with locations were lost
        try:
            original_nodes_with_locations = {
                node.code: node.location
                for node in [*list(p.parents), p.node]
                if node.location is not None
            }

            converted_nodes_with_locations = {
                node.code: node.location
                for node in [*list(converted_path.parents), converted_path.node]
                if node.location is not None
            }

            # If we lost any important nodes with locations, use original
            for code, location in original_nodes_with_locations.items():
                if code not in converted_nodes_with_locations:
                    # Lost a node with location during conversion
                    return p
                if converted_nodes_with_locations[code] != location:
                    # Location changed during conversion
                    return p

        except Exception:
            # Any error in validation, use original
            return p

        return converted_path

    def _ensure_node_version(self, node: GmodNode) -> GmodNode:
        n = node
        vis = VIS()
        if n.vis_version is None:
            raise ValueError("GmodNode must have a valid VIS version")

        # Don't convert if already at or newer than latest version
        if n.vis_version.value >= vis.LatestVisVersion.value:
            return n

        # Only convert if the target version is actually newer
        converted_node = vis.convert_node(n.vis_version, node, vis.LatestVisVersion)
        if converted_node is None:
            # Fallback to original node if conversion fails
            return n

        # Verify essential properties are preserved
        if converted_node.code != node.code or converted_node.location != node.location:
            # Node changed in unexpected ways, use original
            return n

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

        for code, item in self._filter.items():
            if code not in target_nodes:
                return False

            potential_locations = target_nodes[code]

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

            self._filter[set_node.code] = NodeItem(set_node.code, locations)

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

        item = self._filter[node.code]

        if locations:
            item.locations = {loc for loc in locations if loc is not None}
        else:
            item.locations = set()
            item.match_all_locations = match_all_locations

        return self

    def without_locations(self) -> "Path":
        """Remove all locations from the query."""
        for item in self._filter.values():
            item.locations = set()
            item.match_all_locations = True

        return self


class Nodes(GmodPathQueryBuilder):
    """A query builder for GmodNode objects with specific locations."""

    def __init__(self) -> None:
        """Initialize a Nodes query builder."""
        super().__init__()

    def with_node(
        self,
        node: GmodNode,
        match_all_locations: bool = False,
        *locations: Location | None,
    ) -> "Nodes":
        """Add a node to the query with optional locations."""
        n = self._ensure_node_version(node)

        if locations:
            new_locations = {loc for loc in locations if loc}
            if n.code in self._filter:
                self._filter[n.code].locations = new_locations
            else:
                self._filter[n.code] = NodeItem(n.code, new_locations)
        else:
            if n.code in self._filter:
                item = self._filter[n.code]
                item.locations = set()
                item.match_all_locations = match_all_locations
            else:
                item = NodeItem(n.code, set())
                item.match_all_locations = match_all_locations
                self._filter[n.code] = item

        return self
