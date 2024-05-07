from dataclasses import dataclass, field
from typing import Dict, Optional, List, Callable, Optional, TypeVar, Generic,Iterable
from src.GmodPath import GmodPath
from src.GmodDto import GmodDto
from src.internal.ChdDictionary import ChdDictionary
from src.GmodNode import GmodNode
from src.GmodNode import GmodNodeMetadata
from src.VisVersions import VisVersion
from dataclasses import dataclass, field
from enum import Enum
from .GmodNode import GmodNode

TState = TypeVar('TState')

class TraversalHandlerResult(Enum):
    STOP = 0
    SKIP_SUBTREE = 1
    CONTINUE = 2

TraversalHandler = Callable[[List[GmodNode], GmodNode], TraversalHandlerResult]

class TraversalHandlerWithState(Generic[TState]):
    def __init__(self, func: Callable[[TState, List[GmodNode], GmodNode], TraversalHandlerResult]):
        self.func = func

    def __call__(self, state: TState, parents: List[GmodNode], node: GmodNode) -> TraversalHandlerResult:
        return self.func(state, parents, node)

@dataclass
class TraversalOptions:
    DEFAULT_MAX_TRAVERSAL_OCCURRENCE: int = 1  
    max_traversal_occurrence: int = DEFAULT_MAX_TRAVERSAL_OCCURRENCE


@dataclass
class Gmod:
    vis_version: VisVersion
    _node_map: ChdDictionary

    PotentialParentScopeTypes = {"SELECTION", "GROUP", "LEAF"}
    LeafTypes = {"ASSET FUNCTION LEAF", "PRODUCT FUNCTION LEAF"}


    @property
    def root_node(self):
        return self._root_node

    def __iter__(self):
        return iter(self._node_map)
        
    @staticmethod
    def is_potential_parent(type_str : str):
        return type_str in Gmod.PotentialParentScopeTypes

    @staticmethod
    def is_leaf_node(full_type : str):
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
    
    def __init__ (self, vis_version: VisVersion, dto : GmodDto):
        self.vis_version = vis_version

        node_map: Dict[str, GmodNode] = {}

        for node_dto in dto.items:
            node = GmodNode(self.vis_version, node_dto)
            node_map[node_dto.code] = node

        for relation in dto.relations:
            parent_code, child_code = relation
            parent_node = node_map[parent_code]
            child_node = node_map[child_code]
            parent_node.add_child(child_node)
            child_node.add_parent(parent_node)

        self._root_node: Optional[GmodNode] = node_map.get("VE")
        self._node_map = ChdDictionary(node_map.items())

    def __getitem__(self, key: str) -> GmodNode:
        return self._node_map[key]
    
    def try_get_node(self, code):
        node = self._node_map.try_get_value(code)
        return (node is not None, node)

    def parse_path(self, item):
        return GmodPath.parse(item, self.vis_version)

    def try_parse_path(self, item):
        return GmodPath.try_parse_visversion(item, self.vis_version)

    def parse_from_full_path(self, item):
        return GmodPath.parse_full_path(item, self.vis_version)

    def try_parse_from_full_path(self, item):
        return GmodPath.try_parse_full_path_string(item, self.vis_version)


    class Enumerator:
        def __init__(self, inner : ChdDictionary.Enumerator):
            self.inner = inner

        @property
        def current(self):
            return self.inner.current[1] 

        def __iter__(self):
            return self

        def __next__(self):
            if self.inner.__next__():
                return self.current
            else:
                raise StopIteration

        def reset(self):
            self.inner.reset()


    def traverse(self, handler : TraversalHandler, options : Optional[TraversalOptions] = None) -> bool:
        return self.traverse_rootnode(handler, )
    

    def traverse_state(self, state, handler, options : Optional[TraversalOptions] = None) -> bool:
        return self.traverse_rootnode()


    def traverse_rootnode(self, state, root_node, handler, options : Optional[TraversalOptions] = None) -> bool:
        opts = options if options is not None else TraversalOptions()
        context = Gmod.TraversalContext(Gmod.Parents(), handler, state, opts.max_traversal_occurrence)
        return self.traverse_node(context, root_node) == TraversalHandlerResult.CONTINUE
    

    def path_exists_between(self, from_path: Iterable[GmodNode], to_node: GmodNode) -> tuple[bool, Iterable[GmodNode]]:
        last_asset_function = next((node for node in reversed(list(from_path)) if node.is_asset_function_node), None)
        
        state = Gmod.PathExistsContext(to_node)
        
        start_node = last_asset_function if last_asset_function is not None else self._root_node
        
        def handler(state, parents, node):
            if node.code != state.to.code:
                return TraversalHandlerResult.CONTINUE

            actual_parents = []
            current_parents = list(parents) 

            while not current_parents[0].is_root:
                parent = current_parents[0]
                if len(parent.parents) != 1:
                    raise Exception("Invalid state - expected one parent")
                actual_parents.insert(0, parent.parents[0])
                current_parents = [parent.parents[0]] + current_parents
            
            if all(qn.code in (p.code for p in current_parents) for qn in from_path):
                state.remaining_parents = [p for p in current_parents if all(pp.code != p.code for pp in from_path)]
                return TraversalHandlerResult.STOP

            return TraversalHandlerResult.CONTINUE

        reached_end = self.traverse(state, start_node, handler) 

      
        return not reached_end, state.remaining_parents

    @dataclass
    class PathExistsContext:
        to: GmodNode
        remaining_parents: List[GmodNode] = field(default_factory=list)


    def traverse_node(self, context, node):
        if not node.metadata.install_substructure:
            return TraversalHandlerResult.CONTINUE

        result = context.handler(context.state, context.parents.as_list, node)
        if result in (TraversalHandlerResult.STOP, TraversalHandlerResult.SKIP_SUBTREE):
            return result

        skip_occurrence_check = self.is_product_selection_assignment(context.parents.last_or_default(), node) 
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
            self._parents: List[GmodNode] = []

        def push(self, parent: GmodNode) -> None:
            self._parents.append(parent)
            if parent.code in self._occurrences:
                self._occurrences[parent.code] += 1
            else:
                self._occurrences[parent.code] = 1

        def pop(self) -> None:
            if not self._parents:
                return 
            parent = self._parents.pop()
            if self._occurrences[parent.code] == 1:
                del self._occurrences[parent.code]
            else:
                self._occurrences[parent.code] -= 1

        def occurrences(self, node: GmodNode) -> int:
            return self._occurrences.get(node.code, 0)

        def last_or_default(self) -> Optional[GmodNode]:
            return self._parents[-1] if self._parents else None

        def to_list(self) -> List[GmodNode]:
            return self._parents.copy()

        @property
        def as_list(self) -> List[GmodNode]:
            return self._parents
        

    @dataclass(frozen=True)
    class TraversalContext(Generic[TState]):
        parents: 'Gmod.Parents'
        handler: TraversalHandlerWithState[TState]
        state: TState
        max_traversal_occurrence: int
