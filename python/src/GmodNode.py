from dataclasses import dataclass, field
from typing import List, Optional

# Assuming these are imported from other modules
from .Locations import Locations, Location
from .VisVersions import VisVersion
from .GmodDto import GmodNodeDto
from .ParsingErrors import ParsingErrors
from .VIS import VIS

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
    def full_type(self):
        return f"{self.category} {self.type}"

@dataclass
class GmodNode:
    vis_version: VisVersion
    dto : GmodNodeDto
    location: Optional[Location] = None
    children: List['GmodNode'] = field(default_factory=list)
    parents: List['GmodNode'] = field(default_factory=list)

    def __post_init__(self):
        self._children = list(self.children)
        self._parents = list(self.parents)
        self.metadata = GmodNodeMetadata(**self.dto.model_dump())
        self.code = self.dto.code

    def without_location(self):
        return GmodNode(vis_version=self.vis_version, dto=self.dto, location=None, children=self.children, parents=self.parents)

    def with_location(self, location_str: str):
        locations = VIS.instance().get_locations(self.vis_version)
        new_location = locations.parse_location(location_str)
        return GmodNode(vis_version=self.vis_version, dto=self.dto, location=new_location, children=self.children, parents=self.parents)

    def try_with_location(self, location_str: Optional[str]):
        if location_str is None:
            return self
        locations = VIS.instance().get_locations(self.vis_version)
        try:
            new_location = locations.try_parse(location_str)[1]
            return GmodNode(vis_version=self.vis_version, dto=self.dto, location=new_location, children=self.children, parents=self.parents)
        except ValueError:
            return self

    def try_with_location_errors(self, location_str: Optional[str], errors: List[ParsingErrors]):
        if location_str is None:
            return self, errors
        locations = VIS.instance().get_locations(self.vis_version)
        if locations.try_parse(location_str):
            new_location = locations.parse_location(location_str)
            return GmodNode(vis_version=self.vis_version, dto=self.dto, location=new_location, children=self.children, parents=self.parents), errors
        return self, errors


    def is_individualizable(self, is_target_node=False, is_in_set=False):
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
    def is_mappable(self):
        if self.product_type or self.product_selection or self.is_product_selection or self.is_asset:
            return False
        last_char = self.code[-1]
        return last_char not in ['a', 's']

    @property
    def is_product_selection(self):
        return any(child.is_product_selection for child in self.children)

    @property
    def is_product_type(self):
        return any(child.is_product_type for child in self.children)

    @property
    def is_asset(self):
        return self.metadata.category == "ASSET" and self.metadata.type in ["TYPE", "SELECTION"]

    @property
    def product_type(self):
        if len(self.children) == 1:
            child = self.children[0]
            if "FUNCTION" in self.metadata.category and child.metadata.category == "PRODUCT" and child.metadata.type == "TYPE":
                return child
        return None

    @property
    def product_selection(self):
        if len(self.children) == 1:
            child = self.children[0]
            if "FUNCTION" in self.metadata.category and "PRODUCT" in child.metadata.category and child.metadata.type == "SELECTION":
                return child
        return None



    def add_child(self, child: 'GmodNode'):
        if child not in self._children:
            self._children.append(child)

    def add_parent(self, parent: 'GmodNode'):
        if parent not in self._parents:
            self._parents.append(parent)

    def is_child(self, node_or_code):
        code = node_or_code if isinstance(node_or_code, str) else node_or_code.code
        return any(child.code == code for child in self._children)

    def is_leaf_node(self):
        return len(self._children) == 0

    def is_function_node(self):
        return self.metadata.category in ["FUNCTION", "SYSTEM FUNCTION"]

    def is_asset_function_node(self):
        return self.metadata.category == "ASSET FUNCTION" and self.metadata.type == "FUNCTION"

    def is_root(self):
        return self.code == "VE"
