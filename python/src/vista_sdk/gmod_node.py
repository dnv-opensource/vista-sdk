"""This module defines the GmodNode class and its metadata.

It includes methods for creating nodes from DTOs,
managing their relationships, and handling locations.
"""

from __future__ import annotations

from dataclasses import dataclass, field, replace
from typing import ClassVar

from vista_sdk.gmod_dto import GmodNodeDto
from vista_sdk.locations import Location
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.vis_version import VisVersion


@dataclass(frozen=True)
class GmodNodeMetadata:
    """Metadata for a node in the Generic Product Model (GMOD) tree."""

    category: str
    type_val: str
    name: str
    common_name: str | None
    definition: str | None
    common_definition: str | None
    install_substructure: bool | None
    normal_assignment_names: dict[str, str]

    @property
    def type(self) -> str:
        """Get the type value."""
        return self.type_val

    @property
    def full_type(self) -> str:
        """Return the full type of the node."""
        return f"{self.category} {self.type}"


@dataclass
class GmodNode:
    """Represents a node in the Generic Product Model (GMOD) tree."""

    PotentialParentScopeTypes: ClassVar[set[str]] = {"SELECTION", "GROUP", "LEAF"}
    LeafTypes: ClassVar[set[str]] = {"ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF"}

    vis_version: VisVersion
    code: str
    metadata: GmodNodeMetadata
    location: Location | None = None
    children: list[GmodNode] = field(default_factory=list)
    parents: list[GmodNode] = field(default_factory=list)

    @staticmethod
    def is_potential_parent_scope_type(type_str: str) -> bool:
        """Check if the given type string is a potential parent scope type."""
        return type_str in GmodNode.PotentialParentScopeTypes

    @staticmethod
    def is_leaf_node_type(full_type: str) -> bool:
        """Check if the given full type is a leaf node type."""
        return full_type in GmodNode.LeafTypes

    @staticmethod
    def is_leaf_node_from_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a leaf node."""
        return GmodNode.is_leaf_node_type(metadata.full_type)

    @staticmethod
    def is_function_node_category(category: str) -> bool:
        """Check if the given category represents a function node."""
        return category != "PRODUCT" and category != "ASSET"

    @staticmethod
    def is_function_node_from_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a function node."""
        return GmodNode.is_function_node_category(metadata.category)

    @staticmethod
    def is_product_selection_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a product selection."""
        return metadata.category == "PRODUCT" and metadata.type == "SELECTION"

    @staticmethod
    def is_product_type_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents a product type."""
        return metadata.category == "PRODUCT" and metadata.type == "TYPE"

    @staticmethod
    def is_asset_metadata(metadata: GmodNodeMetadata) -> bool:
        """Check if the given metadata represents an asset."""
        return metadata.category == "ASSET"

    @staticmethod
    def is_asset_function_node_metadata(metadata: GmodNodeMetadata) -> bool:
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

    @staticmethod
    def create_from_dto(vis_version: VisVersion, dto: GmodNodeDto) -> GmodNode:
        """Create a GmodNode from a GmodNodeDto."""
        metadata = GmodNodeMetadata(
            dto.category,
            dto.type,
            dto.name,
            dto.common_name,
            dto.definition,
            dto.common_definition,
            dto.install_substructure,
            dto.normal_assignment_names
            if dto.normal_assignment_names is not None
            else {},
        )
        return GmodNode(vis_version=vis_version, code=dto.code, metadata=metadata)

    def __eq__(self, other: object) -> bool:
        """Check equality of two GmodNode instances."""
        if not isinstance(other, GmodNode):
            return NotImplemented
        return (
            self.code == other.code
            and self.metadata.category == other.metadata.category
            and self.metadata.type == other.metadata.type
            and (
                (self.location is None and other.location is None)
                or (
                    self.location is not None
                    and other.location is not None
                    and self.location.value == other.location.value
                )
            )
        )

    def __hash__(self) -> int:
        """Return a hash of the GmodNode based on immutable fields."""
        # Use only the immutable parts for hashing
        return hash((self.code, self.metadata.category, self.metadata.type))

    def without_location(self) -> GmodNode:
        """Return a new node without the location."""
        return GmodNode(
            vis_version=self.vis_version,
            code=self.code,
            metadata=self.metadata,
            children=self.children,
            parents=self.parents,
            location=None,
        )

    def with_location(self, location_str: str | None) -> GmodNode:
        """Set the location of the node, returning a new node if successful."""
        node = self.try_with_location(location_str)
        if node:
            return node
        raise ValueError(f"Invalid location: {location_str}")

    def try_with_location(self, location_str: str | None) -> GmodNode:
        """Try to set the location of the node, returning a new node if successful."""

        def get_locations(vis_version):  # noqa: ANN001, ANN202
            """Get the locations for the current VIS version."""
            from vista_sdk.vis import VIS

            return VIS().get_locations(vis_version)

        if location_str is None:
            return self
        locations = get_locations(self.vis_version)
        try:
            success, new_location = locations.try_parse(location_str)
            if success is False:
                return self
            return GmodNode(
                vis_version=self.vis_version,
                code=self.code,
                metadata=self.metadata,
                location=new_location,
                children=self.children,
                parents=self.parents,
            )
        except ValueError:
            return self

    def try_with_location_errors(
        self, location_str: str | None
    ) -> tuple[GmodNode, ParsingErrors]:
        """Try to set the location of the node, returning errors if parsing fails."""

        def get_locations(vis_version):  # noqa: ANN001, ANN202
            """Get the locations for the current VIS version."""
            from vista_sdk.vis import VIS

            return VIS().get_locations(vis_version)

        locations = get_locations(self.vis_version)
        success, new_location, errors_from_parse = locations.try_parse_with_errors(
            location_str
        )
        if not success:
            return self, errors_from_parse
        if locations.try_parse(location_str):
            new_location = locations.parse(location_str)
            return GmodNode(
                vis_version=self.vis_version,
                code=self.code,
                metadata=self.metadata,
                location=new_location,
                children=self.children,
                parents=self.parents,
            ), errors_from_parse
        return self, errors_from_parse

    def is_individualizable(
        self, is_target_node: bool = False, is_in_set: bool = False
    ) -> bool:
        """Check if the node can be individualized."""
        if self.metadata.type in ["GROUP", "SELECTION"]:
            return False
        if self.is_product_type:
            return False
        if self.metadata.category == "ASSET" and self.metadata.type == "TYPE":
            return False
        if self.is_function_composition:
            return self.code[-1] == "i" or is_in_set or is_target_node
        return True

    @property
    def is_function_composition(self) -> bool:
        """Check if the node is a function composition."""
        return (
            self.metadata.category in ["ASSET FUNCTION", "PRODUCT FUNCTION"]
            and self.metadata.type == "COMPOSITION"
        )

    @property
    def is_mappable(self) -> bool:
        """Check if the node is mappable."""
        if (
            self.product_type
            or self.product_selection
            or self.is_product_selection
            or self.is_asset
        ):
            return False
        last_char = self.code[-1]
        return last_char not in ["a", "s"]

    @property
    def is_product_selection(self) -> bool:
        """Check if the node is a product selection."""
        return GmodNode.is_product_selection_metadata(self.metadata)

    @property
    def is_product_type(self) -> bool:
        """Check if the node is a product type."""
        return GmodNode.is_product_type_metadata(self.metadata)

    @property
    def is_asset(self) -> bool:
        """Check if the node is an asset."""
        return GmodNode.is_asset_metadata(self.metadata)

    @property
    def product_type(self) -> GmodNode | None:
        """Get the product type child node if it exists."""
        if len(self.children) != 1:
            return None

        if "FUNCTION" not in self.metadata.category:
            return None
        child = self.children[0]
        if child.metadata.category != "PRODUCT":
            return None
        if child.metadata.type != "TYPE":
            return None
        return child

    @property
    def product_selection(self) -> GmodNode | None:
        """Get the product selection child node if it exists."""
        if len(self.children) == 1:
            child = self.children[0]
            if (
                "FUNCTION" in self.metadata.category
                and "PRODUCT" in child.metadata.category
                and child.metadata.type == "SELECTION"
            ):
                return child
        return None

    def clone(self, **changes) -> GmodNode:  # noqa: ANN003
        """Create a clone of the GmodNode with specified changes."""
        return replace(self, **changes)

    def add_child(self, child: GmodNode) -> None:
        """Add a child to this GmodNode."""
        if child not in self.children:
            self.children.append(child)

    def add_parent(self, parent: GmodNode) -> None:
        """Add a parent to this GmodNode."""
        if parent not in self.parents:
            self.parents.append(parent)

    def is_child(self, node_or_code: str | GmodNode) -> bool:
        """Check if the node is a child of this GmodNode."""
        code = node_or_code if isinstance(node_or_code, str) else node_or_code.code
        return any(child.code == code for child in self.children)

    @property
    def is_leaf_node(self) -> bool:
        """Check if the node is a leaf node."""
        return GmodNode.is_leaf_node_type(self.metadata.full_type)

    @property
    def is_function_node(self) -> bool:
        """Check if the node is a function node."""
        return GmodNode.is_function_node_category(self.metadata.category)

    def is_asset_function_node(self) -> bool:
        """Check if the node is an asset function node."""
        return GmodNode.is_asset_function_node_metadata(self.metadata)

    def is_root(self) -> bool:
        """Check if the node is a root node."""
        return self.code == "VE"

    def __repr__(self) -> str:
        """Return a repr that avoids recursive expansion of children/parents."""
        return f"GmodNode(code={self.code!r}, location={self.location})"

    def __str__(self) -> str:
        """Return a string representation of the GmodNode."""
        return (
            f"{self.code}-{self.location}" if self.location is not None else self.code
        )
