"""Gmod Individualizable Set and Path Classes."""

from __future__ import annotations

from abc import ABC
from collections import deque
from collections.abc import Generator
from dataclasses import dataclass
from typing import overload

from vista_sdk.gmod_node import GmodNode
from vista_sdk.locations import Location, Locations
from vista_sdk.locations_sets_visitor import LocationSetsVisitor
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis_version import VisVersion


class GmodIndividualizableSet:
    """A set of individualizable nodes in a GmodPath."""

    def __init__(self, nodes: list[int], path: GmodPath) -> None:
        """Initialize the GmodIndividualizableSet with nodes and a path."""
        if not nodes:
            raise Exception("GmodIndividualizableSet can't be empty")
        if any(
            not path[i].is_individualizable(i == path.length - 1, len(nodes) > 1)
            for i in nodes
        ):
            raise Exception("GmodIndividualizableSet nodes must be individualizable")
        if len({path[i].location for i in nodes}) != 1:
            raise Exception("GmodIndividualizableSet nodes have different locations")
        if not any(path[i] == path.node or path[i].is_leaf_node for i in nodes):
            raise Exception(
                "GmodIndividualizableSet has no nodes that are part of short path"
            )

        self._nodes = nodes
        self._path = path
        self._path._parents = list(path.parents)
        self._path.node = path.node

    @property
    def nodes(self) -> list[GmodNode]:
        """Return the nodes in the individualizable set."""
        if self._path is None:
            raise ValueError(
                "Attempting to access nodes on a non-initialized or cleared path"
            )
        return [self._path[i] for i in self._nodes]

    @property
    def node_indices(self) -> list[int]:
        """Return the indices of the nodes in the individualizable set."""
        return self._nodes

    @property
    def location(self) -> Location | None:
        """Return the location of the first node in the individualizable set."""
        if self._path is None:
            raise ValueError(
                "Attempting to access nodes on a non-initialized or cleared path"
            )
        return self._path[self._nodes[0]].location

    @location.setter
    def location(self, value: str) -> None:
        if self._path is None:
            raise ValueError(
                "Attempting to access nodes on a non-initialized or cleared path"
            )
        for i in self._nodes:
            node = self._path[i]
            if value is None:
                self._path[i] = node.without_location()
            else:
                self._path[i] = node.with_location(value)

    def build(self) -> GmodPath:
        """Return the GmodPath associated with this individualizable set."""
        if self._path is None:
            raise Exception("Tried to build individualizable set twice")
        return self._path

    def __str__(self) -> str:
        """Return a string representation of the individualizable set."""
        if self._path is None:
            raise ValueError(
                "Attempting to access nodes on a non-initialized or cleared path"
            )
        return "/".join(
            str(self._path[i])
            for i, _ in enumerate(self._nodes)
            if self._path[i].is_leaf_node() or i == len(self._nodes) - 1
        )


class GmodPath:
    """Represents a path in the Gmod hierarchy."""

    def __init__(
        self, parents: list[GmodNode], node: GmodNode, skip_verify: bool = True
    ) -> None:
        """Initialize the GmodPath with parents and a node."""
        self._parents: list[GmodNode] = []
        if not skip_verify:
            if not parents:
                raise ValueError(
                    f"Invalid gmod path - no parents, and {node.code}"
                    "is not the root of gmod"
                )
            if parents and not parents[0].is_root():
                raise ValueError(
                    "Invalid gmod path - first parent should be root of gmod (VE),"
                    f"but was {parents[0].code}"
                )

            child_codes = {parents[0].code}
            for i in range(len(parents) - 1):
                parent = parents[i]
                child = parents[i + 1]
                if not parent.is_child(child):
                    raise ValueError(
                        f"Invalid gmod path - {child.code} not child of {parent.code}"
                    )

                if child.code in child_codes:
                    raise ValueError(
                        f"Recursion in gmod path argument for code: {child.code}"
                    )
                child_codes.add(child.code)

            if not parents[-1].is_child(node):
                raise ValueError(
                    f"Invalid gmod path - {node.code} not child of {parents[-1].code}"
                )

        self._parents = parents
        self.node = node

    @property
    def parents(self) -> list[GmodNode]:
        """Return the list of parent nodes in the GmodPath."""
        return self._parents

    def set_parents(self, value: list[GmodNode]) -> None:
        """Set the list of parent nodes in the GmodPath."""
        self._parents = value

    @property
    def length(self) -> int:
        """Return the length of the GmodPath, including the node itself."""
        return len(self.parents) + 1

    @property
    def is_mappable(self) -> bool:
        """Check if the GmodPath is mappable."""
        return self.node.is_mappable

    def __getitem__(self, depth: int) -> GmodNode:
        """Get the GmodNode at a specific depth in the GmodPath."""
        if depth < 0 or depth > len(self.parents):
            raise IndexError("Index out of range for GmodPath indexer")
        return self.parents[depth] if depth < len(self.parents) else self.node

    def __setitem__(self, depth: int, value: GmodNode) -> None:
        """Set the GmodNode at a specific depth in the GmodPath."""
        if depth < 0 or depth >= len(self.parents):
            raise IndexError("Index out of range for GmodPath indexer")
        if depth == len(self.parents):
            self.node = value
        else:
            self.parents[depth] = value

    @property
    def individualizable_sets(self) -> list[GmodIndividualizableSet]:
        """Return a list of individualizable sets in the GmodPath."""
        visitor = LocationSetsVisitor()
        result = []
        for i in range(self.length):
            node = self[i]
            set_info = visitor.visit(node, i, self.parents, self.node)
            if set_info:
                start, end, _ = set_info
                nodes = list(range(start, end + 1))
                result.append(GmodIndividualizableSet(nodes, self))
        return result

    @property
    def is_individualizable(self) -> bool:
        """Check if the GmodPath is individualizable."""
        visitor = LocationSetsVisitor()
        for i in range(self.length):
            node = self[i]
            if visitor.visit(node, i, self.parents, self.node):
                return True
        return False

    @staticmethod
    def is_valid(parents: list[GmodNode], node: GmodNode) -> tuple[bool, int]:
        """Check if the GmodPath is valid."""
        missing_link_at = -1

        if len(parents) == 0:
            return False, missing_link_at

        if len(parents) > 0 and not parents[0].is_root():
            return False, missing_link_at

        for i in range(len(parents)):
            parent = parents[i]
            next_index = i + 1
            child = parents[next_index] if next_index < len(parents) else node
            if not parent.is_child(child):
                missing_link_at = i
                return False, missing_link_at

        return True, missing_link_at

    def without_locations(self) -> GmodPath:
        """Return a new GmodPath without locations in the parents and node."""
        new_parents = [parent.without_location() for parent in self.parents]
        new_node = self.node.without_location()
        return GmodPath(new_parents, new_node)

    def __str__(self) -> str:
        """Return a string representation of the GmodPath."""
        return "/".join(
            [parent.__str__() for parent in self.parents if parent.is_leaf_node()]
            + [self.node.__str__()]
        )

    def to_full_path_string(self) -> str:
        """Return the full path as a string representation."""
        return "/".join([node[1].__str__() for node in self.get_full_path()])

    def to_string_dump(self) -> str:
        """Return a string representation of the GmodPath with additional metadata."""
        parts = []
        for depth, node in enumerate(self.get_full_path()):
            if depth == 0:
                continue
            if depth > 1:
                parts.append(" | ")
            parts.append(node[1].code)
            if node[1].metadata.name:
                parts.append(f"/N:{node[1].metadata.name}")
            if node[1].metadata.common_name:
                parts.append(f"/CN:{node[1].metadata.common_name}")
            normal_assignment_name = self.get_normal_assignment_name(depth)
            if normal_assignment_name:
                parts.append(f"/NAN:{normal_assignment_name}")
        return "".join(parts)

    def __eq__(self, other: object) -> bool:
        """Check if two GmodPath instances are equal."""
        if not isinstance(other, GmodPath):
            return NotImplemented

        if other is None:
            return False
        if len(self.parents) != len(other.parents):
            return False

        for _, (parent, other_parent) in enumerate(
            zip(self.parents, other.parents, strict=False)
        ):
            if parent != other_parent:
                return False

        return not self.node != other.node

    def __hash__(self) -> int:
        """Return a hash of the GmodPath."""
        return hash((*self.parents, self.node))

    class Enumerator:
        """An enumerator for GmodPath that allows iteration over the path nodes."""

        def __init__(self, path: GmodPath, from_depth: int | None = None) -> None:
            """Initialize the enumerator with a GmodPath and an optional starting depth."""  # noqa: E501
            self._path = path
            self._current_index = -1
            self._current_node: GmodNode | None = None
            self._from_depth = from_depth

            if from_depth is not None:
                if from_depth < 0 or from_depth > len(self._path.parents):
                    raise IndexError("from_depth out of range")

                self._current_index = from_depth - 1

        def __iter__(self) -> GmodPath.Enumerator:
            """Return the enumerator itself."""
            return self

        def __next__(self) -> tuple[int, GmodNode]:
            """Return the next index and node in the path."""
            if self._current_index < len(self._path.parents):
                self._current_index += 1
                if self._current_index == len(self._path.parents):
                    self._current_node = self._path.node
                else:
                    self._current_node = self._path.parents[self._current_index]
                return self._current_index, self._current_node
            raise StopIteration

        def reset(self) -> None:
            """Reset the enumerator to the initial state."""
            self._current_index = (
                self._from_depth - 1 if self._from_depth is not None else -1
            )

    def get_full_path(self) -> Generator[tuple[int, GmodNode], None, None]:
        """Yield the full path as a generator of tuples (depth, GmodNode)."""
        for i in range(len(self.parents)):
            yield (i, self.parents[i])
        yield (len(self.parents), self.node)

    def get_full_path_from(self, from_depth: int) -> GmodPath.Enumerator:
        """Get an enumerator for the full path starting from a specific depth."""
        if from_depth < 0 or from_depth > len(self.parents):
            raise IndexError("fromDepth is out of allowed range")
        return self.Enumerator(self, from_depth)

    def get_normal_assignment_name(self, node_depth: int) -> str | None:
        """Get the normal assignment name for a node at a specific depth."""
        node = self[node_depth]
        normal_assignment_names = node.metadata.normal_assignment_names

        if not normal_assignment_names:
            return None

        for i in range(len(self.parents) - 1, -1, -1):
            child = self[i]
            name = normal_assignment_names.get(child.code)
            if name:
                return name

        return None

    def get_common_names(self) -> Generator[tuple[int, str], None, None]:
        """Yield common names for nodes in the path."""
        for depth, node in self.get_full_path():
            is_target = depth == len(self.parents)
            if not (node.is_leaf_node() or is_target) or not node.is_function_node():
                continue

            name = node.metadata.common_name or node.metadata.name
            normal_assignment_names = node.metadata.normal_assignment_names

            if normal_assignment_names:
                assignment = normal_assignment_names.get(self.node.code)
                if assignment:
                    name = assignment

                for i in range(len(self.parents) - 1, depth - 1, -1):
                    assignment = normal_assignment_names.get(self.parents[i].code)
                    if assignment:
                        name = assignment

            yield (depth, name)

    def to_verbose_string(
        self, space_delimiter: str = " ", end_delimiter: str = "/"
    ) -> str:
        """Convert the GmodPath to a verbose string representation."""
        from vista_sdk.vis import VIS  # noqa: PLC0415

        builder = []
        for depth, common_name in self.get_common_names():
            location = str(self[depth].location)
            prev = None
            for ch in common_name:
                if ch == "/":
                    continue
                if prev == " " and ch == " ":
                    continue

                current = ch
                if ch == " ":
                    current = space_delimiter
                else:
                    match = VIS.is_iso_string(ch)
                    current = space_delimiter if not match else ch

                if current == "." and prev == ".":
                    continue
                builder.append(current)
                prev = current

            if location and location != "None":
                builder.append(space_delimiter)
                builder.append(location)
            builder.append(end_delimiter)
        builder.pop(-1)
        return "".join(builder)

    @dataclass(frozen=True)
    class PathNode:
        """A node in the GmodPath."""

        code: str
        location: Location | None = None

    @dataclass
    class ParseContext:
        """Context for parsing GmodPath."""

        parts: deque[GmodPath.PathNode]
        to_find: GmodPath.PathNode
        locations: dict[str, Location] | None = None
        path: GmodPath | None = None

    @staticmethod
    @overload
    def parse(item: str, arg: VisVersion) -> GmodPath: ...

    @staticmethod
    @overload
    def parse(item: str, arg, arg2: Locations) -> GmodPath: ...  # noqa: ANN001

    @staticmethod
    def parse(item: str, arg=None, arg2=None):
        """Parse a string into a GmodPath based on the provided arguments."""
        from vista_sdk.gmod import Gmod  # noqa: PLC0415

        if type(arg) is VisVersion and arg2 is None:
            path = GmodPath.try_parse(item, arg=arg)[1]
            if not path:
                raise ValueError("Couldn't parse path")
            return path
        if type(arg) is Gmod and type(arg2) is Locations:
            result = GmodPath.parse_internal(item, arg, arg2)
            if isinstance(result, GmodParsePathResult.Ok):
                return result.path
            if isinstance(result, GmodParsePathResult.Err):
                raise ValueError(result.error)
        raise Exception("Unexpected result")

    @staticmethod
    @overload
    def try_parse(
        item: str | None, arg: VisVersion
    ) -> tuple[bool, GmodPath | None]: ...

    @staticmethod
    @overload
    def try_parse(
        item: str | None,
        arg: Locations,
        gmod,  # noqa: ANN001
    ) -> tuple[bool, GmodPath | None]: ...

    @staticmethod
    def try_parse(
        item: str | None,
        arg: VisVersion | Locations | None = None,
        gmod=None,
    ) -> tuple[bool, GmodPath | None]:
        """Try to parse a string into a GmodPath based on the provided arguments."""
        from vista_sdk.gmod import Gmod  # noqa: PLC0415
        from vista_sdk.vis import VIS  # noqa: PLC0415

        if type(arg) is VisVersion and gmod is None:
            gmod = VIS().get_gmod(arg)
            locations = VIS().get_locations(arg)
            return GmodPath.try_parse(item, arg=locations, gmod=gmod)
        if type(gmod) is Gmod and type(arg) is Locations:
            result = GmodPath.parse_internal(item, gmod, arg)
            if isinstance(result, GmodParsePathResult.Ok):
                return True, result.path
            if isinstance(result, GmodParsePathResult.Err):
                return False, None
            raise Exception("Unexpected result during path parsing")
        raise ValueError("Invalid arguments")

    @staticmethod
    def parse_internal(  # noqa: C901
        item: str | None,
        gmod,  # noqa: ANN001
        locations: Locations,
    ) -> GmodParsePathResult.Ok | GmodParsePathResult.Err:
        """Parse a string into a GmodPath using the provided Gmod and Locations."""
        if gmod.vis_version != locations.vis_version:
            return GmodParsePathResult.Err(
                "Got different VIS versions for Gmod and Locations arguments"
            )

        if not item or item.isspace():
            return GmodParsePathResult.Err("Item is empty")

        item = item.strip().lstrip("/")
        parts: deque[GmodPath.PathNode] = deque()

        try:
            for part_str in item.split("/"):
                if not part_str:
                    return GmodParsePathResult.Err("Found empty code in path")
                if "-" in part_str:
                    code, loc_str = part_str.split("-")
                    success, node = gmod.try_get_node(code)
                    if not success or not node:
                        return GmodParsePathResult.Err(
                            f"Failed to get GmodNode for {code}"
                        )

                    location_success, location = locations.try_parse(loc_str)
                    if not location_success:
                        return GmodParsePathResult.Err(
                            f"Failed to parse location - {loc_str}"
                        )
                    parts.append(GmodPath.PathNode(code, location))

                else:
                    success, node = gmod.try_get_node(part_str)
                    if not success or not node:
                        return GmodParsePathResult.Err(
                            f"Failed to get GmodNode for {part_str}"
                        )
                    parts.append(GmodPath.PathNode(part_str))

            if not parts:
                return GmodParsePathResult.Err("Failed to find any parts")

            to_find: GmodPath.PathNode = parts.popleft()
            (result, base_node) = gmod.try_get_node(to_find.code)
            if not result or not base_node:
                return GmodParsePathResult.Err("Failed to find base node")

            context = GmodPath.ParseContext(parts=parts, to_find=to_find)

            def traverse_handler(  # noqa: C901
                context: GmodPath.ParseContext,
                parents: list[GmodNode],
                current: GmodNode,
            ) -> TraversalHandlerResult:
                from vista_sdk.gmod import Gmod  # noqa: PLC0415

                to_find = context.to_find
                found = current.code == to_find.code

                if not found and Gmod.is_leaf_node(current.metadata.full_type):
                    return TraversalHandlerResult.SKIP_SUBTREE

                if not found:
                    return TraversalHandlerResult.CONTINUE

                if to_find.location is not None:
                    if context.locations is None:
                        context.locations = {}
                    context.locations[to_find.code] = to_find.location

                if len(context.parts) > 0:
                    context.to_find = context.parts.popleft()
                    return TraversalHandlerResult.CONTINUE

                path_parents: list[GmodNode] = []
                for parent in parents:
                    if context.locations and parent.code in context.locations:
                        path_parents.append(
                            parent.with_location(context.locations[parent.code].value)
                        )
                    else:
                        path_parents.append(parent)
                end_node = (
                    current.with_location(to_find.location.value)
                    if to_find.location
                    else current
                )

                start_node: GmodNode | None = None

                if len(path_parents) > 0 and len(path_parents[0].parents) == 1:
                    start_node = path_parents[0].parents[0]
                else:
                    start_node = (
                        end_node.parents[0] if len(end_node.parents) == 1 else None
                    )

                if not start_node or len(start_node.parents) > 1:
                    return TraversalHandlerResult.STOP

                while len(start_node.parents) == 1:
                    path_parents.insert(0, start_node)
                    start_node = start_node.parents[0]
                    if len(start_node.parents) > 1:
                        return TraversalHandlerResult.STOP
                if gmod.root_node:
                    path_parents.insert(0, gmod.root_node)

                visitor = LocationSetsVisitor()
                for i in range(len(path_parents) + 1):
                    n: GmodNode = path_parents[i] if i < len(path_parents) else end_node
                    set_result = visitor.visit(n, i, path_parents, end_node)
                    if set_result is None:
                        if n.location is not None:
                            return TraversalHandlerResult.STOP
                        continue
                    if set_result:
                        start, end, location = set_result
                    else:
                        start, end, location = 0, 0, None
                    if start == end:
                        continue
                    for j in range(start, end + 1):
                        if j < len(path_parents):
                            path_parents[j] = path_parents[j].clone(location=location)
                        else:
                            end_node = end_node.clone(location=location)

                context.path = GmodPath(path_parents, end_node)
                return TraversalHandlerResult.STOP

            gmod.traverse(args1=context, args2=base_node, args3=traverse_handler)

            if context.path:
                return GmodParsePathResult.Ok(context.path)
            return GmodParsePathResult.Err("Failed to find path after traversal")

        except Exception as e:
            return GmodParsePathResult.Err(f"Error parsing path: {e!s}")

    @staticmethod
    @overload
    def try_parse_full_path(
        path_str: str, *, arg: VisVersion
    ) -> tuple[bool, GmodPath]: ...

    @staticmethod
    @overload
    def try_parse_full_path(
        path_str: str,
        gmod,  # noqa: ANN001
        arg: Locations,
    ) -> tuple[bool, GmodPath]: ...

    @staticmethod
    def try_parse_full_path(
        path_str: str, gmod=None, arg=None
    ) -> tuple[bool, GmodPath | None]:
        """Try to parse a full path string into a GmodPath."""
        from vista_sdk.gmod import Gmod  # noqa: PLC0415
        from vista_sdk.vis import VIS  # noqa: PLC0415

        if type(arg) is VisVersion and gmod is None:
            vis = VIS()
            gmod = vis.get_gmod(arg)
            locations_ = vis.get_locations(arg)
            success, path = GmodPath.try_parse_full_path(path_str, gmod, locations_)
            return success, path
        if type(gmod) is Gmod and type(arg) is Locations:
            result = GmodPath.parse_full_path_internal(path_str, gmod, arg)
            if isinstance(result, GmodParsePathResult.Ok):
                return True, result.path
            return False, None
        raise ValueError("Invalid arguments")

    @staticmethod
    def parse_full_path(path_str: str, vis_version: VisVersion) -> GmodPath:
        """Parse a full path string into a GmodPath using the provided VisVersion."""
        from vista_sdk.vis import VIS  # noqa: PLC0415

        vis = VIS()
        gmod = vis.get_gmod(vis_version)
        locations = vis.get_locations(vis_version)
        result = GmodPath.parse_full_path_internal(path_str, gmod, locations)

        if isinstance(result, GmodParsePathResult.Ok):
            return result.path
        if isinstance(result, GmodParsePathResult.Err):
            raise ValueError(result.error)
        raise Exception("Unexpected result")

    @staticmethod
    def parse_full_path_internal(  # noqa: C901
        path_str: str,
        gmod,  # noqa: ANN001
        locations: Locations,
    ) -> GmodParsePathResult.Ok | GmodParsePathResult.Err:
        """Parse a full path string into a GmodPath using the provided Gmod and Locations."""  # noqa: E501
        if not path_str.strip():
            return GmodParsePathResult.Err("Item is empty")

        if not path_str.startswith(gmod.root_node.code):
            return GmodParsePathResult.Err(
                f"Path must start with {gmod.root_node.code}"
            )

        nodes: list[GmodNode] = []
        parts = path_str.strip().split("/")

        for part in (p for p in parts if p):
            dash_index = part.find("-")

            try:
                if dash_index == -1:
                    success, node = gmod.try_get_node(part)
                    if not success:
                        return GmodParsePathResult.Err(
                            f"Failed to get GmodNode for {part}"
                        )
                else:
                    code = part[:dash_index]
                    location_str = part[dash_index + 1 :]
                    success, node = gmod.try_get_node(code)
                    if not success:
                        return GmodParsePathResult.Err(
                            f"Failed to get GmodNode for {code}"
                        )

                    location_success, location = locations.try_parse(location_str)
                    if not location_success:
                        return GmodParsePathResult.Err(
                            f"Failed to parse location - {location_str}"
                        )
                    node = node.with_location(location_str)

                nodes.append(node)
            except Exception as e:
                return GmodParsePathResult.Err(f"Error parsing path: {e!s}")
        if not nodes:
            return GmodParsePathResult.Err("Failed to find any nodes")

        end_node = nodes.pop()
        if not GmodPath.is_valid(nodes, end_node):
            return GmodParsePathResult.Err("Sequence of nodes are invalid")

        visitor = LocationSetsVisitor()
        for i, node in enumerate([*nodes, end_node]):
            set_result = visitor.visit(node, i, nodes, end_node)
            if set_result:
                if set_result:
                    start, end, location = set_result
                else:
                    start, end, location = 0, 0, None
                for j in range(start, end + 1):
                    if j < len(nodes):
                        nodes[j] = nodes[j].with_location(
                            location.__str__() if location else None
                        )
                    else:
                        end_node = end_node.with_location(
                            location.__str__() if location else None
                        )

        return GmodParsePathResult.Ok(GmodPath(nodes, end_node, skip_verify=True))


class GmodParsePathResult(ABC):  # noqa: B024
    """Abstract base class for Gmod path parsing results."""

    def __init__(self) -> None:
        """Initialize the GmodParsePathResult."""
        raise NotImplementedError(
            "This is an abstract base class and cannot be instantiated directly."
        )

    class Ok(super):
        """Represents a successful result in Gmod path parsing."""

        def __init__(self, path: GmodPath) -> None:
            """Initialize the Ok result with a GmodPath."""
            self.path = path

    class Err(super):
        """Represents an error result in Gmod path parsing."""

        def __init__(self, error: str) -> None:
            """Initialize the Err result with an error message."""
            self.error = error
