"""This module defines the GmodNode class and its metadata.

It includes methods for creating nodes from DTOs,
managing their relationships, and handling locations.
"""

from __future__ import annotations

from dataclasses import dataclass, field, replace

from vista_sdk.gmod_dto import GmodNodeDto
from vista_sdk.locations import Location
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.vis_version import VisVersion


@dataclass(frozen=True)
class GmodNodeMetadata:
    """Metadata for a node in the General Maritime Object Data (GMOD) tree."""

    category: str
    type_val: str
    name: str
    common_name: str | None
    definition: str | None
    common_definition: str | None
    install_substructure: bool | None
    normal_assignment_names: dict[str, str] | None = None

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
    """Represents a node in the General Maritime Object Data (GMOD) tree."""

    vis_version: VisVersion
    code: str
    metadata: GmodNodeMetadata
    location: Location | None = None
    children: list[GmodNode] = field(default_factory=list)
    parents: list[GmodNode] = field(default_factory=list)

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
                or (self.location is not None and other.location is not None
                    and self.location.value == other.location.value)
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
            new_location = locations.try_parse(location_str)[1]
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
        self, location_str: str | None, errors: list[ParsingErrors]
    ) -> tuple[GmodNode, list[ParsingErrors]]:
        """Try to set the location of the node, returning errors if parsing fails."""

        def get_locations(vis_version):  # noqa: ANN001, ANN202
            """Get the locations for the current VIS version."""
            from vista_sdk.vis import VIS

            return VIS().get_locations(vis_version)

        if location_str is None:
            return self, errors
        locations = get_locations(self.vis_version)
        if locations.try_parse(location_str):
            new_location = locations.parse(location_str)
            return GmodNode(
                vis_version=self.vis_version,
                code=self.code,
                metadata=self.metadata,
                location=new_location,
                children=self.children,
                parents=self.parents,
            ), errors
        return self, errors

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
        return self.metadata.category == "PRODUCT" and self.metadata.type == "SELECTION"

    @property
    def is_product_type(self) -> bool:
        """Check if the node is a product type."""
        return self.metadata.category == "PRODUCT" and self.metadata.type == "TYPE"

    @property
    def is_asset(self) -> bool:
        """Check if the node is an asset."""
        return self.metadata.category == "ASSET" and self.metadata.type in [
            "TYPE",
            "SELECTION",
        ]

    @property
    def product_type(self) -> GmodNode | None:
        """Get the product type child node if it exists."""
        if len(self.children) == 1:
            child = self.children[0]
            if (
                "FUNCTION" in self.metadata.category
                and child.metadata.category == "PRODUCT"
                and child.metadata.type == "TYPE"
            ):
                return child
        return None

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

    def is_leaf_node(self) -> bool:
        """Check if the node is a leaf node."""
        full_type: str = self.metadata.full_type
        return full_type in ["ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF"]

    def is_function_node(self) -> bool:
        """Check if the node is a function node."""
        return self.metadata.category != "PRODUCT" and self.metadata.type != "ASSET"

    def is_asset_function_node(self) -> bool:
        """Check if the node is an asset function node."""
        return self.metadata.category == "ASSET FUNCTION"

    def is_root(self) -> bool:
        """Check if the node is a root node."""
        return self.code == "VE"

    def __str__(self) -> str:
        """Return a string representation of the GmodNode."""
        return (
            f"{self.code}-{self.location}" if self.location is not None else self.code
        )
