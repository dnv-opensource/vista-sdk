from __future__ import annotations
from collections import deque
from dataclasses import dataclass, field
import inspect
from math import e
from types import NoneType
from typing import Dict, Optional, List, Callable, Optional, Tuple, TypeVar, Generic,Iterable, cast, overload
from vista_sdk.GmodDto import GmodDto
from vista_sdk.internal.ChdDictionary import ChdDictionary
from vista_sdk.GmodNode import GmodNode, GmodNodeMetadata
from vista_sdk.VisVersions import VisVersion
from dataclasses import dataclass, field
from .TraversalHandlerResult import TraversalHandlerResult

TState = TypeVar('TState')

TraversalHandler = Callable[[List[GmodNode], GmodNode], TraversalHandlerResult]
TraversalHandlerWithState = Callable[[TState, List[GmodNode], GmodNode], TraversalHandlerResult]

@dataclass
class TraversalOptions:
    DEFAULT_MAX_TRAVERSAL_OCCURRENCE: int = 1  
    max_traversal_occurrence: int = DEFAULT_MAX_TRAVERSAL_OCCURRENCE



class Gmod:

    PotentialParentScopeTypes = {"SELECTION", "GROUP", "LEAF"}
    LeafTypes = {"ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF"}

    def __init__ (self, vis_version: VisVersion, dto : GmodDto):
        self.vis_version = vis_version

        node_map: Dict[str, GmodNode] = {}

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
        else:
            root_node = node_map.get("VE")
            if root_node is None:
                raise Exception("Invalid state - root node is None despite 'VE' being in node_map")
            self._root_node: GmodNode = root_node
        self._node_map = ChdDictionary([(key, value) for key, value in node_map.items()])

    @property
    def root_node(self) -> GmodNode:
        return self._root_node

    def __iter__(self):
        return iter(self._node_map)
        
    @staticmethod
    def is_potential_parent(type_str : str) -> bool:
        return type_str in Gmod.PotentialParentScopeTypes

    @staticmethod
    def is_leaf_node(full_type : str) -> bool:
        return full_type in Gmod.LeafTypes
    
    @staticmethod
    def is_leaf_node_metadata(metadata: GmodNodeMetadata) -> bool:
        return Gmod.is_leaf_node(metadata.full_type)

    @staticmethod
    def is_function_node(category: str) -> bool:
        return category != "PRODUCT" and category != "ASSET"

    @staticmethod
    def is_function_node_metadata(metadata: GmodNodeMetadata) -> bool:
        return Gmod.is_function_node(metadata.category)

    @staticmethod
    def is_product_selection(metadata: GmodNodeMetadata) -> bool:
        return metadata.category == "PRODUCT" and metadata.type == "SELECTION"

    @staticmethod
    def is_product_type(metadata: GmodNodeMetadata) -> bool:
        return metadata.category == "PRODUCT" and metadata.type == "TYPE"
    
    @staticmethod
    def is_asset(metadata: GmodNodeMetadata) -> bool:
        return metadata.category == "ASSET"

    @staticmethod
    def is_asset_function_node(metadata: GmodNodeMetadata) -> bool:
        return metadata.category == "ASSET FUNCTION"
    
    @staticmethod
    def is_product_type_assignment(parent : Optional[GmodNode], child : Optional[GmodNode]) -> bool:
       
        if parent is None or child is None:
            return False
        if "FUNCTION" not in parent.metadata.category:
            return False
        if child.metadata.category != "PRODUCT" or child.metadata.type != "TYPE":
            return False
        return True

    @staticmethod
    def is_product_selection_assignment(parent : Optional[GmodNode], child : Optional[GmodNode]) -> bool:
       
        if parent is None or child is None:
            return False
        if "FUNCTION" not in parent.metadata.category:
            return False
        if "PRODUCT" not in child.metadata.category or child.metadata.type != "SELECTION":
            return False
        return True
    
   

    def __getitem__(self, key: str) -> GmodNode:
        node = self._node_map[key]
        if node is not None:
            return node
        else:
            raise KeyError(f"Key not found: {key}")
    
    def try_get_node(self, code : str) -> tuple[bool, Optional[GmodNode]]:
        node = self._node_map.try_get_value(code)
        return (node[1], node[0])

    def parse_path(self, item : str):
        from vista_sdk.GmodPath import GmodPath
        return GmodPath.parse(item, arg = self.vis_version)

    def try_parse_path(self, item : str):
        from vista_sdk.GmodPath import GmodPath
        return GmodPath.try_parse(item, arg = self.vis_version)

    def parse_from_full_path(self, item : str):
        from vista_sdk.GmodPath import GmodPath
        return GmodPath.parse_full_path(item, self.vis_version)

    def try_parse_from_full_path(self, item : str):
        from vista_sdk.GmodPath import GmodPath
        return GmodPath.try_parse_full_path(item, arg = self.vis_version)
    
    def check_signature(self, handler : TraversalHandler | TraversalHandlerWithState, param_count : int):
        sig = inspect.signature(handler)
        return len(sig.parameters) == param_count


    @overload
    def traverse(self, args1: TraversalHandler, args2: Optional[TraversalOptions] = None) -> bool:
        ...
    
    @overload
    def traverse(self, args1: GmodNode, args2 : TraversalHandler, args3: Optional[TraversalOptions] = None) -> bool:
        ...

    @overload
    def traverse(self, args1: TState, args2: TraversalHandlerWithState[TState], args3: Optional[TraversalOptions] = None) -> bool:
        ...
    
    @overload
    def traverse(self, args1: TState, args2: GmodNode, args3: TraversalHandlerWithState[TState], args4: Optional[TraversalOptions] = None) -> bool:
        ...

    def traverse(self, args1 = None, args2 = None, args3 = None, args4 = None) -> bool:
        state: Optional[TState] = None # type: ignore
        root_node: Optional[GmodNode] = None
        handler : Optional[TraversalHandlerWithState | TraversalHandler] = None
        options : Optional[TraversalOptions] = None

        arg1Type = type(args1) 
        arg2Type = type(args2)
        arg3Type = type(args3)
        arg4Type = type(args4)
        
        if(callable(args1) and self.check_signature(args1, 2)  and arg3Type is NoneType and arg4Type is NoneType):
            handler = lambda handler, parents, node: handler(parents, node)
            options = args2
            state = args1
            root_node = self.root_node
        elif(callable(args1) and self.check_signature(args1, 2)  and arg2Type is GmodNode and arg4Type is NoneType):
            handler = lambda handler, parents, node: handler(parents, node)
            root_node = self.root_node
            options = args3
        elif(arg1Type is not None and callable(args2) and self.check_signature(args2, 3) and arg3Type is NoneType and arg4Type is NoneType):
            state = args1
            handler = args2
            options = args3
            root_node = self.root_node
        elif(arg1Type is not None and arg2Type is GmodNode and callable(args3) and self.check_signature(args3, 3)):
            state = args1
            root_node = args2
            handler = args3
            options = args4
        else:
            raise ValueError("Invalid arguments")
            

        return self._traverse_internal(state, cast(GmodNode, root_node), cast(TraversalHandlerWithState, handler), options)
    
    def _traverse_internal(self, state: TState, node: GmodNode, handler: TraversalHandlerWithState[TState],
                            options: Optional[TraversalOptions]) -> bool:
        opts = options if options is not None else TraversalOptions()
        parents = Gmod.Parents()
        context  = Gmod.TraversalContext(parents, handler, state, opts.max_traversal_occurrence)
        return self.traverse_node(context=context, node=node) == TraversalHandlerResult.CONTINUE

    def _path_exists_between(self, from_path: Iterable[GmodNode], to_node: GmodNode) -> tuple[bool, Iterable[GmodNode]]:
        last_asset_function = next((node for node in reversed(list(from_path)) if node.is_asset_function_node), None)        
        start_node = last_asset_function if last_asset_function is not None else self._root_node

        state = self.PathExistsContext(to=to_node)

        def handler(state, parents : List[GmodNode] , node : GmodNode):
            if node.code != state.to.code:
                return TraversalHandlerResult.CONTINUE

            actual_parents = []
            current_parents = list(parents)

            while current_parents and not current_parents[0].is_root:
                parent = current_parents[0]
                if len(parent.parents) != 1:
                    raise Exception("Invalid state - expected one parent")
                actual_parents.insert(0, parent.parents[0])
                current_parents = [parent.parents[0]] + current_parents

            if all(qn.code in (p.code for p in current_parents) for qn in from_path):
                state.remaining_parents = [p for p in current_parents if all(pp.code != p.code for pp in from_path)]
                return TraversalHandlerResult.STOP

            return TraversalHandlerResult.CONTINUE

        reached_end = self.traverse(args1=state, args2=start_node, args3 = handler)
        return not reached_end, state.remaining_parents

    @dataclass
    class PathExistsContext:
        to: GmodNode
        remaining_parents: List[GmodNode] = field(default_factory=list)


    def traverse_node(self, context : Gmod.TraversalContext[TState], node : GmodNode) -> TraversalHandlerResult:
        if node.metadata.install_substructure == False:
            return TraversalHandlerResult.CONTINUE

        result = context.handler(context.state, context.parents.nodes, node)
        if result in (TraversalHandlerResult.STOP, TraversalHandlerResult.SKIP_SUBTREE):
            return result

        skip_occurrence_check = Gmod.is_product_selection_assignment(context.parents.last_or_default(), node) 
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
            elif result == TraversalHandlerResult.SKIP_SUBTREE:
                continue

        context.parents.pop()
        return TraversalHandlerResult.CONTINUE


    class Parents:
        def __init__(self):
            self._occurrences: Dict[str, int] = {}
            self.nodes: List[GmodNode] = []
        
        def contains(self, node: GmodNode) -> bool:
            return node.code in self._occurrences

        def push(self, parent: GmodNode) -> None:
            self.nodes.append(parent)
            if parent.code in self._occurrences:
                self._occurrences[parent.code] += 1
            else:
                self._occurrences[parent.code] = 1

        def pop(self) -> None:
            if not self.nodes:
                return 
            parent = self.nodes.pop()
            if self._occurrences[parent.code] == 1:
                del self._occurrences[parent.code]
            else:
                self._occurrences[parent.code] -= 1

        def occurrences(self, node: GmodNode) -> int:
            return self._occurrences.get(node.code, 0)

        def last_or_default(self) -> Optional[GmodNode]:
            return self.nodes[-1] if len(self.nodes)>0 else None

        def to_list(self) -> List[GmodNode]:
            return self.nodes.copy()
        

    @dataclass
    class TraversalContext(Generic[TState]):
        parents: Gmod.Parents
        handler: TraversalHandlerWithState[TState]
        state: TState
        max_traversal_occurrence: int = 1

    