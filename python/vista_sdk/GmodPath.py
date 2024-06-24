from __future__ import annotations
from abc import ABC
from collections import deque
from dataclasses import dataclass
from io import StringIO
from optparse import Option
from typing import Generator, Optional, Dict, List, Deque, overload
from typing import Optional, Tuple
from vista_sdk.Gmod import Gmod
from vista_sdk.TraversalHandlerResult import TraversalHandlerResult
from vista_sdk.VIS import VIS
from vista_sdk.VisVersions import VisVersion
from vista_sdk.GmodNode import GmodNode
from vista_sdk.Locations import Location, Locations
from vista_sdk.LocationSetsVisitor import LocationSetsVisitor

class GmodIndividualizableSet:
    def __init__(self, nodes : List[int], path : GmodPath):
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
        self._path._parents = list(path.parents) 
        self._path.node = path.node 

    @property
    def nodes(self) -> List[GmodNode]:
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        return [self._path[i] for i in self._nodes]

    @property
    def node_indices(self) -> List[int]:
        return self._nodes

    @property
    def location(self) -> Optional[Location]:
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        return self._path[self._nodes[0]].location

    @location.setter
    def location(self, value : str) -> None:
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        for i in self._nodes:
            node = self._path[i]
            if value is None:
                self._path[i] = node.without_location()
            else:
                self._path[i] = node.with_location(value)

    def build(self) -> GmodPath:
        if self._path is None:
            raise Exception("Tried to build individualizable set twice")
        path = self._path
        self._path = None
        return path

    def __str__(self) -> str:
        if self._path is None:
            raise ValueError("Attempting to access nodes on a non-initialized or cleared path")
        return "/".join(
            str(self._path[i]) for i, _ in enumerate(self._nodes) 
            if self._path[i].is_leaf_node or i == len(self._nodes) - 1
        )
    
class GmodPath:
    def __init__(self, parents : List[GmodNode], node : GmodNode, skip_verify=True):
        self._parents : List[GmodNode] = []
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
    def parents(self) -> List[GmodNode]:
        return self._parents
    
    @parents.setter
    def set_parents(self, value) -> None:
        self._parents = value

    @property
    def length(self) -> int:
        return len(self.parents) + 1

    @property
    def is_mappable(self) -> bool:
        return self.node.is_mappable

    def __getitem__(self, depth : int) -> GmodNode:
        if depth < 0 or depth > len(self.parents):
            raise IndexError("Index out of range for GmodPath indexer")
        return self.parents[depth] if depth < len(self.parents) else self.node

    def __setitem__(self, depth : int, value : GmodNode) -> None:
        if depth < 0 or depth >= len(self.parents):
            raise IndexError("Index out of range for GmodPath indexer")
        if depth == len(self.parents):
            self.node = value
        else:
            self.parents[depth] = value

    @property
    def individualizable_sets(self) -> List[GmodIndividualizableSet]:
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
        visitor = LocationSetsVisitor()
        for i in range(self.length):
            node = self[i]
            if visitor.visit(node, i, self.parents, self.node):
                return True
        return False

    @staticmethod
    def is_valid(parents: List[GmodNode], node: GmodNode) -> Tuple[bool, int]:
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
        new_parents = [parent.without_location() for parent in self.parents]
        new_node = self.node.without_location()
        return GmodPath(new_parents, new_node)

    def __str__(self) -> str:
        return "/".join([parent.__str__() for parent in self.parents if parent.is_leaf_node()] + [self.node.__str__()])

  
    def to_full_path_string(self) -> str:
        return "/".join([node[1].__str__() for node in self.get_full_path()])

    def to_string_dump(self) -> str:
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
    
    def __eq__(self, other : GmodPath) -> bool:
        if other is None:
            return False
        if len(self.parents) != len(other.parents):
            return False
        if any(parent != other_parent for parent, other_parent in zip(self.parents, other.parents)):
            return False
        return self.node == other.node

    def __hash__(self) -> int:
        return hash(tuple(self.parents + [self.node]))
    
  

    class Enumerator:
        def __init__(self, path : GmodPath, from_depth=None):
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

        def __next__(self) -> Tuple[int, GmodNode]:
            if self._current_index < len(self._path.parents):
                self._current_index += 1
                if self._current_index == len(self._path.parents):
                    self._current_node = self._path.node
                else:
                    self._current_node = self._path.parents[self._current_index]
                return self._current_index, self._current_node
            else:
                raise StopIteration

        def reset(self) -> None:
            self._current_index = self._from_depth - 1 if self._from_depth is not None else -1

    def get_full_path(self) -> Generator[Tuple[int, GmodNode], None, None]:
        for i in range(len(self.parents)):
            yield (i, self.parents[i])
        yield (len(self.parents), self.node)

    def get_full_path_from(self, from_depth : int) -> GmodPath.Enumerator:
        if from_depth < 0 or from_depth > len(self.parents):
            raise IndexError("fromDepth is out of allowed range")
        return self.Enumerator(self, from_depth)
    
    def get_normal_assignment_name(self, node_depth: int) -> Optional[str]:
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

    def get_common_names(self):
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

    def to_verbose_string(self, space_delimiter: str = " ", end_delimiter : str = "/") -> str:
        builder = []
        for depth, common_name in self.get_common_names():
            location = str(self[depth].location)
            prev = None
            for ch in common_name:
                if ch == '/':
                    continue
                if prev == ' ' and ch == ' ':
                    continue

                current = ch
                if ch == ' ':
                    current = space_delimiter
                else:
                    match = VIS.is_iso_string(ch)
                    if not match:
                        current = space_delimiter
                    else:
                        current = ch

                if current == '.' and prev == '.':
                    continue
                builder.append(current)
                prev = current

            if location and location != "None":
                builder.append(space_delimiter)
                builder.append(location)
            builder.append(end_delimiter)
        builder.pop(-1)
        return ''.join(builder)
        

    @dataclass(frozen=True)
    class PathNode:
        code: str
        location: Optional[Location] = None 

    @dataclass
    class ParseContext:
        parts: Deque[GmodPath.PathNode]
        to_find: GmodPath.PathNode
        locations: Optional[Dict[str, Location]] = None
        path: Optional[GmodPath] = None

    @staticmethod
    @overload
    def parse(item: str, arg : VisVersion) -> GmodPath:
        ...
    
    @staticmethod
    @overload
    def parse(item: str, arg : Gmod, arg2 : Locations) -> GmodPath:
        ...

    @staticmethod
    def parse(item: str, arg = None, arg2 = None):
        if type(arg) is VisVersion and arg2 is None:
            path = GmodPath.try_parse(item, arg = arg)[1]
            if not path:
                raise ValueError("Couldn't parse path")
            return path
        elif type(arg) is Gmod and type(arg2) is Locations:
            result = GmodPath.parse_internal(item, arg, arg2)
            if isinstance(result, GmodParsePathResult.Ok):
                return result.path
            elif isinstance(result, GmodParsePathResult.Err):
                raise ValueError(result.error)
            else:
                raise Exception("Unexpected result")
        

    @staticmethod
    @overload
    def try_parse(item: Optional[str], arg: VisVersion) -> Tuple[bool ,Optional[GmodPath]]:
        ...
    
    @staticmethod
    @overload
    def try_parse(item: Optional[str], arg: Locations, gmod: Gmod) -> Tuple[bool ,Optional[GmodPath]]:
        ...

    @staticmethod
    def try_parse(item: Optional[str], arg : VisVersion | Locations | None = None,  gmod : Gmod | None = None) -> Tuple[bool , Optional[GmodPath]]:  
        if type(arg) is VisVersion and gmod is None:
            gmod = VIS().get_gmod(arg) 
            locations = VIS().get_locations(arg)
            return GmodPath.try_parse(item, arg = locations, gmod = gmod)
        elif type(gmod) is Gmod and type(arg) is Locations:
            result = GmodPath.parse_internal(item, gmod, arg)
            if isinstance(result, GmodParsePathResult.Ok):
                return True, result.path
            elif isinstance(result, GmodParsePathResult.Err):
                return False, None
            else:
                raise Exception("Unexpected result during path parsing")
        else:
            raise ValueError("Invalid arguments")
                
    

    @staticmethod
    def parse_internal(item : Optional[str], gmod : Gmod, locations : Locations) -> GmodParsePathResult.Ok | GmodParsePathResult.Err:
        if gmod.vis_version != locations.vis_version:
            return GmodParsePathResult.Err("Got different VIS versions for Gmod and Locations arguments")

        if not item or item.isspace():
            return GmodParsePathResult.Err("Item is empty")

        item = item.strip().lstrip('/')
        parts = deque()

        for part_str in item.split('/'):
            if '-' in part_str:
                code, loc_str = part_str.split('-')
                node = gmod.try_get_node(code)
                location = locations.try_parse(loc_str)[1]
                if node is None:
                    return GmodParsePathResult.Err(f"Failed to get GmodNode for {part_str}")
                if location is None:
                    return GmodParsePathResult.Err(f"Failed to parse location {loc_str}")
                parts.append(GmodPath.PathNode(code, location))
            else:
                node = gmod.try_get_node(part_str)
                if node is None:
                    return GmodParsePathResult.Err(f"Failed to get GmodNode for {part_str}")
                parts.append(GmodPath.PathNode(part_str))

        if not parts:
            return GmodParsePathResult.Err("Failed to find any parts")

        to_find : GmodPath.PathNode = parts.popleft()
        (result, base_node) = gmod.try_get_node(to_find.code)
        if not result or not base_node:
            return GmodParsePathResult.Err("Failed to find base node")

        context = GmodPath.ParseContext(parts=parts, to_find=to_find)

        def traverse_handler(context: GmodPath.ParseContext, parents: List[GmodNode], current: GmodNode):
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

            path_parents: List[GmodNode] = []
            for parent in parents:
                if context.locations and parent.code in context.locations:
                    path_parents.append(parent.with_location(context.locations[parent.code].value))
                else:
                    path_parents.append(parent)
            end_node = current.with_location(to_find.location.value) if to_find.location else current

            start_node: GmodNode | None = None
            
            if(len(path_parents) > 0 and len(path_parents[0].parents) == 1):
                start_node = path_parents[0].parents[0]
            else:
                if(len(end_node.parents) == 1):
                    start_node = end_node.parents[0]
                else:
                    start_node = None

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
                n :GmodNode = path_parents[i] if i < len(path_parents) else end_node
                set_result = visitor.visit(n, i, path_parents, end_node)
                if set_result is None:
                    if n.location is not None:
                        return TraversalHandlerResult.STOP
                    continue
                start, end, location = set_result
                if start == end:
                    continue
                for j in range(start, end + 1):
                    if j < len(path_parents):
                        path_parents[j] = path_parents[j].clone(location=location)
                    else:
                        end_node = end_node.clone(location=location)
          

            context.path = GmodPath(path_parents, end_node)
            return TraversalHandlerResult.STOP


        gmod.traverse(args1 = context, args2 = base_node, args3 = traverse_handler)

        if context.path:
            return GmodParsePathResult.Ok(context.path)
        else:
            return GmodParsePathResult.Err("Failed to find path after traversal")
    
    @staticmethod
    @overload
    def try_parse_full_path(path_str: str, *, arg : VisVersion) -> Tuple[bool, GmodPath]:
        ...
    
    @staticmethod
    @overload
    def try_parse_full_path(path_str: str, gmod : Gmod, arg : Locations) -> Tuple[bool, GmodPath]:
        ...

    @staticmethod
    def try_parse_full_path(path_str: str, gmod = None, arg = None) -> Tuple[bool, Optional[GmodPath]]:
        if type(arg) is VisVersion and gmod is None:
            vis = VIS()
            gmod = vis.get_gmod(arg)
            locations_ = vis.get_locations(arg)
            success, path = GmodPath.try_parse_full_path(path_str, gmod, locations_)
            return success, path
        elif type(gmod) is Gmod and type(arg) is Locations:
            result = GmodPath.parse_full_path_internal(path_str, gmod, arg)
            if isinstance(result, GmodParsePathResult.Ok):
                return True, result.path
            return False, None
        else:
            raise ValueError("Invalid arguments")
        
    @staticmethod
    def parse_full_path(path_str : str, vis_version : VisVersion) -> GmodPath:
        vis = VIS()
        gmod = vis.get_gmod(vis_version)
        locations = vis.get_locations(vis_version)
        result = GmodPath.parse_full_path_internal(path_str, gmod, locations)

        if isinstance(result, GmodParsePathResult.Ok):
            return result.path
        elif isinstance(result, GmodParsePathResult.Err):
            raise ValueError(result.error)
        else:
            raise Exception("Unexpected result")
        

    @staticmethod
    def parse_full_path_internal(path_str : str, gmod : Gmod, locations : Locations) -> GmodParsePathResult.Ok | GmodParsePathResult.Err:
        if not path_str.strip():
            return GmodParsePathResult.Err("Item is empty")

        if not path_str.startswith(gmod.root_node.code):
            return GmodParsePathResult.Err(f"Path must start with {gmod.root_node.code}")

        nodes = []
        parts = path_str.strip().split('/')

        for part in parts:
            dash_index = part.find('-')
            
            if dash_index == -1:
                node = gmod.try_get_node(part)[1]
                if gmod.try_get_node(part)[0] is False:
                    return GmodParsePathResult.Err(f"Failed to get GmodNode for {part}")
            else:
                code = part[:dash_index]
                location_str = part[dash_index + 1:]
                node = gmod.try_get_node(code)[1]
                if node is None:
                    return GmodParsePathResult.Err(f"Failed to get GmodNode for {code}")
                location = locations.try_parse(location_str)
                if not location[0]:
                    return GmodParsePathResult.Err(f"Failed to parse location - {location_str}")
                else:
                    node = node.with_location(location_str=location_str)
            nodes.append(node)

        if not nodes:
            return GmodParsePathResult.Err("Failed to find any nodes")

        end_node = nodes.pop() 
        if not GmodPath.is_valid(nodes, end_node):
            return GmodParsePathResult.Err("Sequence of nodes are invalid")

        visitor = LocationSetsVisitor()
        for i, node in enumerate(nodes + [end_node]):
            set_result = visitor.visit(node, i, nodes, end_node)
            if set_result:
                start, end, location = set_result
                for j in range(start, end + 1):
                    if j < len(nodes):
                        nodes[j] = nodes[j].with_location(location)
                    else:
                        end_node = end_node.with_location(location)

        return GmodParsePathResult.Ok(GmodPath(nodes,  end_node, skip_verify=True))
        


class GmodParsePathResult(ABC):
    def __init__(self):
        raise NotImplementedError("This is an abstract base class and cannot be instantiated directly.")

    class Ok(super):

        def __init__(self, path: GmodPath):
            self.path = path
    class Err(super):
        def __init__(self, error: str):
            self.error = error
        
            
