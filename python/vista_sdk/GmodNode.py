from __future__ import annotations
from dataclasses import dataclass, field ,replace
from typing import List, Optional, Tuple
from .Locations import Location
from .VisVersions import VisVersion
from .GmodDto import GmodNodeDto
from .ParsingErrors import ParsingErrors

class GmodNodeMetadata:
    def __init__(self, category, type, name, common_name, definition, common_definition, install_substructure, normal_assignment_names):
        self.category = category
        self.type = type
        self.name = name
        self.common_name = common_name
        self.definition = definition
        self.common_definition = common_definition
        self.install_substructure = install_substructure
        self.normal_assignment_names = normal_assignment_names
    
    @property
    def full_type(self) -> str:
        return f"{self.category} {self.type}"

@dataclass
class GmodNode:
    vis_version: VisVersion
    code: str
    metadata: GmodNodeMetadata
    location: Optional[Location] = None
    children: List[GmodNode] = field(default_factory=list)
    parents: List[GmodNode] = field(default_factory=list)

    @staticmethod
    def create_from_dto(vis_version: VisVersion, dto: GmodNodeDto) -> GmodNode:
        metadata = GmodNodeMetadata(
            dto.category,
            dto.type,
            dto.name,
            dto.common_name,
            dto.definition,
            dto.common_definition,
            dto.install_substructure,
            dto.normal_assignment_names if dto.normal_assignment_names is not None else {}
        )
        return GmodNode(
            vis_version=vis_version,
            code=dto.code,
            metadata=metadata
        )
    
    def __eq__(self, other: GmodNode) -> bool:
        if(self.code != other.code): return False
        if(self.location is not None and other.location is not None and self.location.value != other.location.value): return False
        if(self.location is None and other.location is not None): return False
        if(self.location is not None and other.location is None): return False
        return True
    

    def without_location(self) -> GmodNode:
        return GmodNode(
            vis_version=self.vis_version,
            code=self.code,
            metadata=self.metadata,
            children=self.children,
            parents=self.parents
        )

    def with_location(self, location_str: str) -> GmodNode:
        node = self.try_with_location(location_str)
        if node:
            return node
        raise ValueError(f"Invalid location: {location_str}")

    def try_with_location(self, location_str: Optional[str]) -> GmodNode:
        from .VIS import VIS
        if location_str is None:
            return self
        locations = VIS().get_locations(self.vis_version)
        try:
            new_location = locations.try_parse(location_str)[1]
            return GmodNode(
                vis_version=self.vis_version,
                code=self.code,
                metadata=self.metadata,
                location=new_location,
                children=self.children,
                parents=self.parents
            )
        except ValueError:
            return self

    def try_with_location_errors(self, location_str: Optional[str], errors: List[ParsingErrors]) -> Tuple[GmodNode, List[ParsingErrors]]:
        from .VIS import VIS
        if location_str is None:
            return self, errors
        locations = VIS().get_locations(self.vis_version)
        if locations.try_parse(location_str):
            new_location = locations.parse(location_str)
            return GmodNode(
                vis_version=self.vis_version,
                code=self.code,
                metadata=self.metadata,
                location=new_location,
                children=self.children,
                parents=self.parents
            ), errors
        return self, errors


    def is_individualizable(self, is_target_node : bool =False, is_in_set : bool =False) -> bool:
        if self.metadata.type in ["GROUP", "SELECTION"]:
            return False
        if self.is_product_type:
            return False
        if self.metadata.category == "ASSET" and self.metadata.type == "TYPE":
            return False
        if self.is_function_composition:
            return self.code[-1] == 'i' or is_in_set or is_target_node
        return True

    @property
    def is_function_composition(self) -> bool:
        return (self.metadata.category in ["ASSET FUNCTION", "PRODUCT FUNCTION"] and
                self.metadata.type == "COMPOSITION")

    @property
    def is_mappable(self) -> bool:
        if self.product_type or self.product_selection or self.is_product_selection or self.is_asset:
            return False
        last_char = self.code[-1]
        return last_char not in ['a', 's']

    @property
    def is_product_selection(self) -> bool:
        return self.metadata.category == "PRODUCT" and self.metadata.type == "SELECTION"

    @property
    def is_product_type(self) -> bool:
        return self.metadata.category == "PRODUCT" and self.metadata.type == "TYPE"

    @property
    def is_asset(self) -> bool:
        return self.metadata.category == "ASSET" and self.metadata.type in ["TYPE", "SELECTION"]

    @property
    def product_type(self) -> Optional[GmodNode]:
        if len(self.children) == 1:
            child = self.children[0]
            if "FUNCTION" in self.metadata.category and child.metadata.category == "PRODUCT" and child.metadata.type == "TYPE":
                return child
        return None

    @property
    def product_selection(self) ->  Optional[GmodNode]:
        if len(self.children) == 1:
            child = self.children[0]
            if "FUNCTION" in self.metadata.category and "PRODUCT" in child.metadata.category and child.metadata.type == "SELECTION":
                return child
        return None

    def clone(self, **changes) -> GmodNode:
        return replace(self, **changes)


    def add_child(self, child: GmodNode) -> None:
        if child not in self.children:
            self.children.append(child)

    def add_parent(self, parent: GmodNode) -> None:
        if parent not in self.parents:
            self.parents.append(parent)

    def is_child(self, node_or_code) -> bool:
        code = node_or_code if isinstance(node_or_code, str) else node_or_code.code
        return any(child.code == code for child in self.children)

    def is_leaf_node(self) -> bool:
        full_type : str = self.metadata.full_type
        return full_type in ["ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF"]

    def is_function_node(self) -> bool:
        return self.metadata.category != "PRODUCT" and self.metadata.type != "ASSET"

    def is_asset_function_node(self) -> bool:
        return self.metadata.category == "ASSET FUNCTION" and self.metadata.type == "FUNCTION"

    def is_root(self) -> bool:
        return self.code == "VE"
    
    def __str__(self):
        return f"{self.code}-{self.location}" if self.location is not None else self.code
