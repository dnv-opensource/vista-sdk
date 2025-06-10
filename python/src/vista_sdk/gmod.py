"""GMOD (Generalized Model of Data) module for the VISTA SDK.

This module provides the core GMOD functionality,
including traversal and path operations.
"""

from __future__ import annotations

import inspect
from collections.abc import Callable, Iterable
from dataclasses import dataclass, field
from types import NoneType
from typing import ClassVar, Generic, TypeVar, cast, overload

from vista_sdk.gmod_dto import GmodDto
from vista_sdk.gmod_node import GmodNode, GmodNodeMetadata
from vista_sdk.gmod_path import GmodPath
from vista_sdk.internal.chd_dictionary import ChdDictionary
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis_version import VisVersion

TState = TypeVar("TState")

TraversalHandler = Callable[[list[GmodNode], GmodNode], TraversalHandlerResult]
TraversalHandlerWithState = Callable[
    [TState, list[GmodNode], GmodNode], TraversalHandlerResult
]


@dataclass
class TraversalOptions:
    """Options for traversing the GMOD structure."""

    DEFAULT_MAX_TRAVERSAL_OCCURRENCE: int = 1
    max_traversal_occurrence: int = DEFAULT_MAX_TRAVERSAL_OCCURRENCE


class Gmod:
    """Class representing the GMOD (Generalized Model of Data) structure."""

    PotentialParentScopeTypes: ClassVar[set[str]] = {"SELECTION", "GROUP", "LEAF"}
    LeafTypes: ClassVar[set[str]] = {"ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF"}

    def __init__(self, vis_version: VisVersion, dto: GmodDto) -> None:
        """Initialize the Gmod instance with a version and a DTO."""
        self.vis_version = vis_version

        node_map: dict[str, GmodNode] = {}

        for node_dto in dto.items:
            node = GmodNode.create_from_dto(self.vis_version, node_dto)
            node_map[node_dto.code] = node

        for relation in dto.relations:
            parent_code, child_code = relation
            parent_node = node_map[parent_code]
            child_node = node_map[child_code]
            parent_node.add_child(child_node)
            child_node.add_parent(parent_node)

        if "VE" not in node_map:
            raise Exception("Invalid state - root node not found")
        root_node = node_map.get("VE")
        if root_node is None:
            raise Exception(
                "Invalid state - root node is None despite 'VE' being in node_map"
            )
        self._root_node: GmodNode = root_node
        self._node_map = ChdDictionary(list(node_map.items()))

    @property
    def root_node(self) -> GmodNode:
        """Return the root node of the GMOD."""
        return self._root_node

    def __iter__(self) -> Iterable[tuple[str, GmodNode]]:
        """Return an iterator over the nodes in the GMOD."""
        return iter(self._node_map)

    @staticmethod
    def is_potential_parent(type_str: str) -> bool:
        """Check if the given type string is a potential parent scope type."""
        return type_str in Gmod.PotentialParentScopeTypes

    @staticmethod
    def is_leaf_node(full_type: str) -> bool:
        """Check if the given full type is a leaf node type."""
        return full_type in Gmod.LeafTypes

    @staticmethod
    def is_leaf_node_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a leaf node."""
        return Gmod.is_leaf_node(metadata.full_type)

    @staticmethod
    def is_function_node(category: str) -> bool:
        """Check if the given category represents a function node."""
        return category != "PRODUCT" and category != "ASSET"

    @staticmethod
    def is_function_node_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a function node."""
        return Gmod.is_function_node(metadata.category)

    @staticmethod
    def is_product_selection(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a product selection."""
        return metadata.category == "PRODUCT" and metadata.type == "SELECTION"

    @staticmethod
    def is_product_type(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a product type."""
        return metadata.category == "PRODUCT" and metadata.type == "TYPE"

    @staticmethod
    def is_asset(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents an asset."""
        return metadata.category == "ASSET"

    @staticmethod
    def is_asset_function_node(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents an asset function node."""
        return metadata.category == "ASSET FUNCTION"

    @staticmethod
    def is_product_type_assignment(
        parent: GmodNode | None, child: GmodNode | None
    ) -> bool:
        """Check if the parent-child relationship is a product type assignment."""
        if parent is None or child is None:
            return False
        if "FUNCTION" not in parent.metadata.category:
            return False
        return not (
            child.metadata.category != "PRODUCT" or child.metadata.type != "TYPE"
        )

    @staticmethod
    def is_product_selection_assignment(
        parent: GmodNode | None, child: GmodNode | None
    ) -> bool:
        """Check if the parent-child relationship is a product selection assignment."""
        if parent is None or child is None:
            return False
        if "FUNCTION" not in parent.metadata.category:
            return False
        return not (
            "PRODUCT" not in child.metadata.category
            or child.metadata.type != "SELECTION"
        )

    def __getitem__(self, key: str) -> GmodNode:
        """Get a node by its key (code)."""
        node = self._node_map[key]
        if node is not None:
            return node
        raise KeyError(f"Key not found: {key}")

    def try_get_node(self, code: str) -> tuple[bool, GmodNode | None]:
        """Try to get a node by its code."""
        node = self._node_map.try_get_value(code)
        return (node[1], node[0])

    def parse_path(self, item: str) -> GmodPath:
        """Parse a string into a GmodPath object."""
        return GmodPath.parse(item, arg=self.vis_version)

    def try_parse_path(self, item: str) -> tuple[bool, GmodPath | None] | None:
        """Try to parse a string into a GmodPath object."""
        return GmodPath.try_parse(item, arg=self.vis_version)

    def parse_from_full_path(self, item: str) -> GmodPath:
        """Parse a full path string into a GmodPath object."""
        return GmodPath.parse_full_path(item, self.vis_version)

    def try_parse_from_full_path(self, item: str) -> tuple[bool, GmodPath | None]:
        """Try to parse a full path string into a GmodPath object."""
        return GmodPath.try_parse_full_path(item, arg=self.vis_version)

    def check_signature(
        self, handler: TraversalHandler | TraversalHandlerWithState, param_count: int
    ) -> bool:
        """Check if the handler function has the expected number of parameters."""
        sig = inspect.signature(handler)
        return len(sig.parameters) == param_count

    @overload
    def traverse(
        self, args1: TraversalHandler, args2: TraversalOptions | None = None
    ) -> bool: ...

    @overload
    def traverse(
        self,
        args1: GmodNode,
        args2: TraversalHandler,
        args3: TraversalOptions | None = None,
    ) -> bool: ...

    @overload
    def traverse(
        self,
        args1: TState,
        args2: TraversalHandlerWithState[TState],
        args3: TraversalOptions | None = None,
    ) -> bool: ...

    @overload
    def traverse(
        self,
        args1: TState,
        args2: GmodNode,
        args3: TraversalHandlerWithState[TState],
        args4: TraversalOptions | None = None,
    ) -> bool: ...

    def traverse(self, args1=None, args2=None, args3=None, args4=None) -> bool:
        """Traverse the GMOD structure based on the provided arguments."""
        state: TState | None  # type: ignore
        root_node: GmodNode | None = None
        handler: TraversalHandlerWithState | TraversalHandler | None = None
        options: TraversalOptions | None = None

        args1_type: type = type(args1)
        arg2_type = type(args2)
        arg3_type = type(args3)
        arg4_type = type(args4)

        if (
            callable(args1)
            and self.check_signature(args1, 2)  # type: ignore
            and arg3_type is NoneType
            and arg4_type is NoneType
        ):
            handler = lambda handler, parents, node: handler(parents, node)  # noqa: E731
            options = args2
            state = args1
            root_node = self.root_node
        elif (
            callable(args1)
            and self.check_signature(args1, 2)  # type: ignore
            and arg2_type is GmodNode
            and arg4_type is NoneType
        ):
            handler = lambda handler, parents, node: handler(parents, node)  # noqa: E731
            root_node = self.root_node
            options = args3
        elif (
            args1_type is not None
            and callable(args2)
            and self.check_signature(args2, 3)  # type: ignore
            and arg3_type is NoneType
            and arg4_type is NoneType
        ):
            state = args1
            handler = args2  # type: ignore
            options = args3
            root_node = self.root_node
        elif (
            args1_type is not None
            and arg2_type is GmodNode
            and callable(args3)
            and self.check_signature(args3, 3)  # type: ignore
        ):
            state = args1
            root_node = args2  # type: ignore
            handler = args3  # type: ignore
            options = args4
        else:
            raise ValueError("Invalid arguments")

        return self._traverse_internal(
            state,
            cast(GmodNode, root_node),
            cast(TraversalHandlerWithState, handler),
            options,
        )

    def _traverse_internal(
        self,
        state: TState,
        node: GmodNode,
        handler: TraversalHandlerWithState[TState],
        options: TraversalOptions | None = None,
    ) -> bool:
        opts = options if options is not None else TraversalOptions()
        parents = Gmod.Parents()
        context = Gmod.TraversalContext(
            parents, handler, state, opts.max_traversal_occurrence
        )
        return (
            self.traverse_node(context=context, node=node)
            == TraversalHandlerResult.CONTINUE
        )

    def _path_exists_between(
        self, from_path: Iterable[GmodNode], to_node: GmodNode
    ) -> tuple[bool, Iterable[GmodNode]]:
        last_asset_function = next(
            (
                node
                for node in reversed(list(from_path))
                if node.is_asset_function_node()
            ),
            None,
        )
        start_node = (
            last_asset_function if last_asset_function is not None else self._root_node
        )

        state = self.PathExistsContext(to=to_node)

        def handler(
            state: Gmod.PathExistsContext, parents: list[GmodNode], node: GmodNode
        ) -> TraversalHandlerResult:
            """Handler for path existence check."""
            if node.code != state.to.code:
                return TraversalHandlerResult.CONTINUE

            actual_parents: list[GmodNode] = []
            current_parents = list(parents)

            while current_parents and not current_parents[0].is_root():
                parent = current_parents[0]
                if len(parent.parents) != 1:
                    raise Exception("Invalid state - expected one parent")
                actual_parents.insert(0, parent.parents[0])
                current_parents = [parent.parents[0], *current_parents]

            if all(qn.code in (p.code for p in current_parents) for qn in from_path):
                state.remaining_parents = [
                    p
                    for p in current_parents
                    if all(pp.code != p.code for pp in from_path)
                ]
                return TraversalHandlerResult.STOP

            return TraversalHandlerResult.CONTINUE

        reached_end = self.traverse(args1=state, args2=start_node, args3=handler)
        return not reached_end, state.remaining_parents

    @dataclass
    class PathExistsContext:
        """Context for checking if a path exists between nodes."""

        to: GmodNode
        remaining_parents: list[GmodNode] = field(default_factory=list)

    def traverse_node(
        self, context: Gmod.TraversalContext[TState], node: GmodNode
    ) -> TraversalHandlerResult:
        """Traverse a single node in the GMOD structure."""
        if node.metadata.install_substructure is False:
            return TraversalHandlerResult.CONTINUE

        result = context.handler(context.state, context.parents.nodes, node)
        if result in (TraversalHandlerResult.STOP, TraversalHandlerResult.SKIP_SUBTREE):
            return result

        skip_occurrence_check = Gmod.is_product_selection_assignment(
            context.parents.last_or_default(), node
        )
        if not skip_occurrence_check:
            occ = context.parents.occurrences(node)
            if occ == context.max_traversal_occurrence:
                return TraversalHandlerResult.SKIP_SUBTREE
            if occ > context.max_traversal_occurrence:
                raise Exception("Invalid state - node occurred more than expected")

        context.parents.push(node)
        for child in node.children:
            result = self.traverse_node(context, child)
            if result == TraversalHandlerResult.STOP:
                return result
            if result == TraversalHandlerResult.SKIP_SUBTREE:
                continue

        context.parents.pop()
        return TraversalHandlerResult.CONTINUE

    class Parents:
        """Class to manage the parent nodes during traversal."""

        def __init__(self) -> None:
            """Initialize the Parents instance."""
            self._occurrences: dict[str, int] = {}
            self.nodes: list[GmodNode] = []

        def contains(self, node: GmodNode) -> bool:
            """Check if the node is already in the occurrences."""
            return node.code in self._occurrences

        def push(self, parent: GmodNode) -> None:
            """Push a parent node onto the stack and update occurrences."""
            self.nodes.append(parent)
            if parent.code in self._occurrences:
                self._occurrences[parent.code] += 1
            else:
                self._occurrences[parent.code] = 1

        def pop(self) -> None:
            """Pop the last parent node from the stack and update occurrences."""
            if not self.nodes:
                return
            parent = self.nodes.pop()
            if self._occurrences[parent.code] == 1:
                del self._occurrences[parent.code]
            else:
                self._occurrences[parent.code] -= 1

        def occurrences(self, node: GmodNode) -> int:
            """Get the number of occurrences of a node."""
            return self._occurrences.get(node.code, 0)

        def last_or_default(self) -> GmodNode | None:
            """Get the last node in the stack or None if empty."""
            return self.nodes[-1] if len(self.nodes) > 0 else None

        def to_list(self) -> list[GmodNode]:
            """Return a copy of the current nodes in the stack."""
            return self.nodes.copy()

    @dataclass
    class TraversalContext(Generic[TState]):
        """Context for traversing the GMOD structure."""

        parents: Gmod.Parents
        handler: TraversalHandlerWithState[TState]
        state: TState
        max_traversal_occurrence: int = 1
