from __future__ import annotations

from dataclasses import dataclass
from turtle import st
from typing import Optional, Dict, List, Deque
from collections import deque
from typing import Optional, Tuple


from src.GmodNode import GmodNode
from src.Locations import Locations, Location
from src.VIS import VIS, VisVersion
from src.Gmod import Gmod


class GmodIndividualizableSet:
    def __init__(self, nodes : List[int], path : 'GmodPath'):
        if not nodes:
            raise Exception("GmodIndividualizableSet can't be empty")
        if any(not path[i].is_individualizable(i == path.length - 1, len(nodes) > 1) for i in nodes):
            raise Exception("GmodIndividualizableSet nodes must be individualizable")
        if len(set(path[i].location for i in nodes)) != 1:
            raise Exception("GmodIndividualizableSet nodes have different locations")
        if not any(path[i] == path.node or path[i].is_leaf_node for i in nodes):
            raise Exception("GmodIndividualizableSet has no nodes that are part of short path")
        

        self._nodes = nodes
        self._path = path 
        self._path.parents = list(path.parents) 
        self._path.node = path.node 

    @property
    def nodes(self):
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        return [self._path[i] for i in self._nodes]

    @property
    def node_indices(self):
        return self._nodes

    @property
    def location(self):
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        return self._path[self._nodes[0]].location

    @location.setter
    def location(self, value):
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        for i in self._nodes:
            node = self._path[i]
            if value is None:
                self._path[i] = node.without_location()
            else:
                self._path[i] = node.with_location(value)

    def build(self):
        if self._path is None:
            raise Exception("Tried to build individualizable set twice")
        path = self._path
        self._path = None
        return path

    def __str__(self):
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        return "/".join(
            str(self._path[i]) for i, _ in enumerate(self._nodes) 
            if self._path[i].is_leaf_node or i == len(self._nodes) - 1
        )
    
class GmodPath:
    def __init__(self, parents : List[GmodNode], node : GmodNode, skip_verify=True):
        if not skip_verify:
            if not parents:
                raise ValueError(f"Invalid gmod path - no parents, and {node.code} is not the root of gmod")
            if parents and not parents[0].is_root:
                raise ValueError(f"Invalid gmod path - first parent should be root of gmod (VE), but was {parents[0].code}")

            child_codes = {parents[0].code}
            for i in range(len(parents) - 1):
                parent = parents[i]
                child = parents[i + 1]
                if not parent.is_child(child):
                    raise ValueError(f"Invalid gmod path - {child.code} not child of {parent.code}")

                if child.code in child_codes:
                    raise ValueError(f"Recursion in gmod path argument for code: {child.code}")
                child_codes.add(child.code)

            if not parents[-1].is_child(node):
                raise ValueError(f"Invalid gmod path - {node.code} not child of {parents[-1].code}")

        self._parents = parents
        self.node = node

    @property
    def parents(self):
        return self._parents
    
    @parents.setter
    def parents(self, value):
        self._parents = value

    @property
    def length(self):
        return len(self._parents) + 1

    @property
    def is_mappable(self):
        return self.node.is_mappable

    def __getitem__(self, depth) -> GmodNode:
        if depth < 0 or depth > len(self._parents):
            raise IndexError("Index out of range for GmodPath indexer")
        return self._parents[depth] if depth < len(self._parents) else self.node

    def __setitem__(self, depth, value):
        if depth < 0 or depth >= len(self._parents):
            raise IndexError("Index out of range for GmodPath indexer")
        if depth == len(self._parents):
            self.node = value
        else:
            self._parents[depth] = value

    @property
    def individualizable_sets(self):
        visitor = self.LocationSetsVisitor()
        result = []
        for i in range(self.length):
            node = self[i]
            set_info = visitor.visit(node, i, self._parents, self.node)
            if set_info:
                start, end, _ = set_info
                nodes = list(range(start, end + 1))
                result.append(GmodIndividualizableSet(nodes, self))
        return result

    @property
    def is_individualizable(self):
        visitor = self.LocationSetsVisitor()
        for i in range(self.length):
            node = self[i]
            if visitor.visit(node, i, self._parents, self.node):
                return True
        return False

    
    @staticmethod
    def is_valid(parents, node):
        if not parents:
            return False
        if not parents[0].is_root:
            return False

        for i, parent in enumerate(parents[:-1]):
            if not parent.is_child(parents[i + 1]):
                return False
        if not parents[-1].is_child(node):
            return False
        return True

    def without_locations(self):
        new_parents = [parent.without_location() for parent in self._parents]
        new_node = self.node.without_location()
        return GmodPath(new_parents, new_node)

    def __str__(self):
        return "/".join([parent.__str__() for parent in self._parents if parent.is_leaf_node()] + [self.node.__str__()])

    def to_full_path_string(self):
        return "/".join([node.__str__() for node in self.get_full_path()])

    def to_string_dump(self):
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
        return ''.join(parts)
    
    def __eq__(self, other : 'GmodPath'):
        if other is None:
            return False
        if len(self._parents) != len(other._parents):
            return False
        if any(parent != other_parent for parent, other_parent in zip(self._parents, other._parents)):
            return False
        return self.node == other.node

    def __hash__(self):
        return hash(tuple(self._parents + [self.node]))
    
  

    class Enumerator:
        def __init__(self, path : 'GmodPath', from_depth=None):
            self._path = path
            self._current_index = -1 
            self._current_node = None
            self._from_depth = from_depth

            if from_depth is not None:
                if from_depth < 0 or from_depth > len(self._path.parents):
                    raise IndexError("from_depth out of range")
                self._current_index = from_depth - 1

        def __iter__(self):
            return self

        def __next__(self):
            if self._current_index < len(self._path.parents):
                self._current_index += 1
                if self._current_index == len(self._path.parents):
                    self._current_node = self._path.node
                else:
                    self._current_node = self._path.parents[self._current_index]
                return self._current_index, self._current_node
            else:
                raise StopIteration

        def reset(self):
            self._current_index = self._from_depth - 1 if self._from_depth is not None else -1

    def get_full_path(self):
        return self.Enumerator(self)

    def get_full_path_from(self, from_depth):
        if from_depth < 0 or from_depth > len(self._parents):
            raise IndexError("fromDepth is out of allowed range")
        return self.Enumerator(self, from_depth)
    
    def get_normal_assignment_name(self, node_depth):
        node = self[node_depth]
        normal_assignment_names = node.metadata.normal_assignment_names
        
        if not normal_assignment_names:
            return None

        for i in range(len(self._parents) - 1, -1, -1):
            child = self[i]
            name = normal_assignment_names.get(child.code)
            if name:
                return name

        return None

    def get_common_names(self):
        for depth, node in enumerate(self.get_full_path()):
            is_target = depth == len(self._parents)
            if not (node[1].is_leaf_node() or is_target) or not node[1].is_function_node():
                continue

            name = node[1].metadata.common_name or node[1].metadata.name
            normal_assignment_names = node[1].metadata.normal_assignment_names

            if normal_assignment_names:
                assignment = normal_assignment_names.get(self.node.code)
                if assignment:
                    name = assignment

                for i in range(len(self._parents) - 1, depth - 1, -1):
                    assignment = normal_assignment_names.get(self._parents[i].code)
                    if assignment:
                        name = assignment

            yield (depth, name)

    @dataclass(frozen=True)
    class PathNode:
        code: str
        location: Optional['Location'] = None 

    @dataclass
    class ParseContext:
        parts: Deque[GmodPath.PathNode]
        to_find: GmodPath.PathNode
        locations: Optional[Dict[str, 'Location']] = None
        path: Optional['GmodPath'] = None

    @staticmethod
    def parse(item: str, vis_version: 'VisVersion') -> 'GmodPath':
        path = GmodPath.try_parse_visversion(item, vis_version)
        if not path:
            raise ValueError("Couldn't parse path")
        return path

    @staticmethod
    def try_parse_visversion(item: Optional[str], vis_version: 'VisVersion'):
        gmod = VIS.instance().get_gmod(vis_version) # TODO: check singelton 
        locations = VIS.instance().get_locations(vis_version)
        return GmodPath.try_parse_gmod(item, gmod, locations)

    @staticmethod
    def parse_with_gmod_and_locations(item: str, gmod: 'Gmod', locations: 'Locations') -> 'GmodPath':
        result = GmodPath.parse_internal(item, gmod, locations)
        if isinstance(result, Ok):
            return result.path
        elif isinstance(result, Err):
            raise ValueError(result.error)
        else:
            raise Exception("Unexpected result")
        
    class LocationSetsVisitor:
        current_parent_start: int = -1

        def visit(self, node: 'GmodNode', i: int, parents: List['GmodNode'], target: 'GmodNode') -> Optional[Tuple[int, int, Optional['Location']]]:
            is_parent = Gmod.is_potential_parent(node.metadata.type)
            is_target_node = i == len(parents)
            
            if self.current_parent_start == -1:
                if is_parent:
                    self.current_parent_start = i
                if node.is_individualizable(is_target_node):
                    return (i, i, node.location)
            else:
                if is_parent or is_target_node:
                    nodes = None
                    if self.current_parent_start + 1 == i:
                        if node.is_individualizable(is_target_node):
                            nodes = (i, i, node.location)
                    else:
                        skipped_one = -1
                        has_composition = False
                        for j in range(self.current_parent_start + 1, i + 1):
                            set_node = parents[j] if j < len(parents) else target
                            if not set_node.is_individualizable(j == len(parents), is_in_set=True):
                                if nodes is not None:
                                    skipped_one = j
                                continue
                            
                            if (nodes and nodes[2] is not None and set_node.location is not None and nodes[2] != set_node.location):
                                raise Exception(f"Mapping error: different locations in the same nodeset: {nodes[2]}, {set_node.location}")

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

                    self.current_parent_start = i
                    if nodes:
                        has_leaf_node = False
                        for j in range(nodes[0], nodes[1] + 1):
                            set_node = parents[j] if j < len(parents) else target
                            if set_node.is_leaf_node() or j == len(parents):
                                has_leaf_node = True
                                break
                        if has_leaf_node:
                            return nodes

                if is_target_node and node.is_individualizable(is_target_node):
                    return (i, i, node.location)

            return None
    
    @staticmethod
    def try_parse_gmod(item: Optional[str], gmod: 'Gmod', locations: 'Locations') -> 'GmodPath':
        result = GmodPath.parse_internal(item, gmod, locations)
        if isinstance(result, Ok):
            return result.path
        elif isinstance(result, Err):
            raise ValueError(result.error)
        else:
            raise Exception("Unexpected result during path parsing")
    

    @staticmethod
    def parse_internal(item : Optional[str], gmod : Gmod, locations : Locations) -> GmodParsePathResult:
        if gmod.vis_version != locations.vis_version:
            return Err("Got different VIS versions for Gmod and Locations arguments")

        if not item or item.isspace():
            return Err("Item is empty")

        item = item.strip().lstrip('/')
        parts = deque()

        for part_str in item.split('/'):
            if '-' in part_str:
                code, loc_str = part_str.split('-')
                node = gmod.try_get_node(code)
                location = locations.try_parse(loc_str)[1]
                if node is None:
                    return Err(f"Failed to get GmodNode for {part_str}")
                if location is None:
                    return Err(f"Failed to parse location {loc_str}")
                parts.append(GmodPath.PathNode(code, location))
            else:
                node = gmod.try_get_node(part_str)
                if node is None:
                    return Err(f"Failed to get GmodNode for {part_str}")
                parts.append(GmodPath.PathNode(part_str))

        if not parts:
            return Err("Failed to find any parts")

        to_find = parts.popleft()
        base_node = gmod.try_get_node(to_find.code)
        if base_node is None:
            return Err("Failed to find base node")

        context = GmodPath.ParseContext(parts=parts, to_find=to_find)

        #TODO: Implement the traverse method after gmod is implemented
        gmod.traverse(context, base_node, handle_node_visit)

        if context.path:
            return Ok(context.path)
        else:
            return Err("Failed to find path after traversal")
    
    @staticmethod
    def parse_full_path(path_str, vis_version):
        vis = VIS.instance()
        gmod = vis.get_gmod(vis_version)
        locations = vis.get_locations(vis_version)
        result = GmodPath.parse_full_path_internal(path_str, gmod, locations)

        if isinstance(result, Ok):
            return result.path
        elif isinstance(result, Err):
            raise ValueError(result.error)
        else:
            raise Exception("Unexpected result")

    @staticmethod
    def try_parse_full_path_string(path_str, vis_version):
        vis = VIS.instance()
        gmod = vis.get_gmod(vis_version)
        locations = vis.get_locations(vis_version)
        success, path = GmodPath.try_parse_full_path_location(path_str, gmod, locations)
        return success, path

    @staticmethod
    def try_parse_full_path_location(path_str, gmod, locations):
        result = GmodPath.parse_full_path_internal(path_str, gmod, locations)
        if isinstance(result, Ok):
            return True, result.path
        return False, None


    @staticmethod
    def parse_full_path_internal(path_str, gmod, locations):
        if not path_str.strip():
            return Err("Item is empty")

        if not path_str.startswith(gmod.root_node.code):
            return Err(f"Path must start with {gmod.root_node.code}")

        nodes = []
        parts = path_str.strip().split('/')

        for part in parts:
            dash_index = part.find('-')

            if dash_index == -1:
                node = gmod.try_get_node(part)
                if node is None:
                    return Err(f"Failed to get GmodNode for {part}")
                nodes.append(node)
            else:
                code = part[:dash_index]
                location_str = part[dash_index + 1:]
                node = gmod.try_get_node(code)
                if node is None:
                    return Err(f"Failed to get GmodNode for {code}")
                location = locations.try_parse(location_str)
                if location is None:
                    return Err(f"Failed to parse location - {location_str}")

                node = node.with_location(location)
                nodes.append(node)

        if not nodes:
            return Err("Failed to find any nodes")

        end_node = nodes.pop() 
        if not GmodPath.is_valid(nodes, end_node):
            return Err("Sequence of nodes are invalid")

        visitor = GmodPath.LocationSetsVisitor()
        for i, node in enumerate(nodes + [end_node]):
            set_result = visitor.visit(node, i, nodes, end_node)
            if set_result:
                start, end, location = set_result
                for j in range(start, end + 1):
                    if j < len(nodes):
                        nodes[j] = nodes[j].with_location(location)
                    else:
                        end_node = end_node.with_location(location)

        return Ok(GmodPath(nodes,  end_node, skip_verify=True))



@dataclass
class GmodParsePathResult:
    def __init__(self):
        raise NotImplementedError("This is an abstract base class and cannot be instantiated directly.")

@dataclass
class Ok(GmodParsePathResult):
    path: GmodPath 

@dataclass
class Err(GmodParsePathResult):
    error: str