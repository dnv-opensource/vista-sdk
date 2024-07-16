from typing import List, Optional, Tuple
from vista_sdk.GmodNode import GmodNode
from vista_sdk.Locations import Location


class LocationSetsVisitor:
        current_parent_start: int = -1
        
        def visit(self, node: GmodNode, i: int, parents: List[GmodNode], target: GmodNode) -> Optional[Tuple[int, int, Optional[Location]]]:
            from vista_sdk.Gmod import Gmod
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