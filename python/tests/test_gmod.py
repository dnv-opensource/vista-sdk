from dataclasses import dataclass
from typing import List
import unittest
from vista_sdk.VIS import VIS

from vista_sdk.GmodNode import GmodNode
from .test_vis import TestVis
from vista_sdk.GmodDto import GmodDto  
from vista_sdk.VisVersions import VisVersion, VisVersionExtension
from vista_sdk.Gmod import TraversalOptions, Gmod, TraversalHandlerResult
from vista_sdk.GmodPath import GmodPath
from vista_sdk.Client import Client


class TestGmod(unittest.TestCase):

    def setUp(self):
        self.vis = TestVis.get_vis()
        Client.get_gmod(VisVersionExtension.to_version_string(VisVersion.v3_4a))
       

    def test_gmod_loads(self):
         for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version) 
                self.assertIsNotNone(gmod)
                self.assertTrue(gmod.try_get_node("400a"))

    def test_gmod_properties(self):
         for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version) 
                self.assertIsNotNone(gmod)

                nodes = list(gmod._node_map)
                min_length = min(gmod, key=lambda x: len(x[1].code))
                max_length = max(gmod, key=lambda x: len(x[1].code))

                self.assertEqual(2, len(min_length[1].code))
                self.assertEqual("VE", min_length[1].code)
                self.assertEqual(10, len(max_length[1].code))
                possible_max = ["C1053.3111", "H346.11113"]
                self.assertIn(max_length[1].code, possible_max)

                expected_counts = [6420, 6557, 6672]
                self.assertIn(len(nodes), expected_counts)

    def test_gmod_lookup(self):
        for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version)
                self.assertIsNotNone(gmod)

                gmodDto : GmodDto = self.vis.get_gmod_dto(version)
                self.assertIsNotNone(gmodDto)

                seen = set()
                for node in gmodDto.items:
                    self.assertIsNotNone(node.code)
                    self.assertTrue(node.code not in seen, f"code: {node.code}")
                    seen.add(node.code)

                    success, foundNode = gmod.try_get_node(node.code)
                    self.assertTrue(success)
                    self.assertIsNotNone(foundNode)
                    if(foundNode is not None):
                        self.assertEqual(node.code, foundNode.code)

                seen.clear()
                counter = 0
                for node in gmod:
                    self.assertIsNotNone(node[1].code)
                    self.assertTrue(node[1].code not in seen, f"code: {node[1].code}")
                    seen.add(node[1].code)

                    success, foundNode = gmod.try_get_node(node[1].code)
                    self.assertTrue(success)
                    self.assertIsNotNone(foundNode)
                    if(foundNode is not None):
                        self.assertEqual(node[1].code, foundNode.code)
                    counter += 1

                self.assertEqual(len(gmodDto.items), counter)

                test_codes = ["ABC", None, "", "SDFASDFSDAFb", "✅", "a✅b", "ac✅bc", "✅bc", "a✅", "ag✅"]
                for code in test_codes:
                    success, _ = gmod.try_get_node(code)
                    self.assertFalse(success)


    def test_gmod_node_equality(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node1 = gmod["400a"]
        node2 = gmod["400a"]

        self.assertEqual(node1, node2, "Nodes should be equal")
        self.assertIs(node1, node2, "Nodes should be the exact same object (identity)")

        if node2 is not None:
            node3 = node2.with_location("1")
        self.assertNotEqual(node1, node3, "Nodes should not be equal after modification")
        self.assertIsNot(node1, node3, "Nodes should not be the same object after modification")

        if node2 is not None:
            node4 = node2.clone()  
        self.assertEqual(node1, node4, "Cloned nodes should be equal")
        self.assertIsNot(node1, node4, "Cloned nodes should not be the same object")

    
    def test_gmod_node_types(self):
            gmod = self.vis.get_gmod(VisVersion.v3_4a)

            unique_types = set()
            for node in gmod:
                category_type = f"{node[1].metadata.category} | {node[1].metadata.type}"
                unique_types.add(category_type)

            self.assertTrue(unique_types, "The set of node types should not be empty")

    def test_gmod_root_node_children(self):
        for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version)

                node = gmod.root_node  
                if node is None:
                    self.fail("Root node should not be None")
                else:
                    self.assertIsNotNone(node, "Root node should not be None")
                    self.assertGreater(len(node.children), 0, "Root node should have children")

    def test_normal_assignments(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node = gmod["411.3"]
        self.assertIsNotNone(node.product_type, "ProductType should not be None for node '411.3'")
        self.assertIsNone(node.product_selection, "ProductSelection should be None for node '411.3'")

        node = gmod["H601"]
        self.assertIsNone(node.product_type, "ProductType should be None for node 'H601'")
                   
    def test_node_with_product_selection(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node = gmod["411.2"]
        self.assertIsNotNone(node.product_selection, "ProductSelection should not be None for node '411.2'")
        self.assertIsNone(node.product_type, "ProductType should be None for node '411.2'")

        node = gmod["H601"]
        self.assertIsNone(node.product_selection, "ProductSelection should be None for node 'H601'")

    def test_product_selection(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node = gmod["CS1"]
        self.assertTrue(node.is_product_selection, "Node 'CS1' should be identified as having a product selection")

    def test_mappability(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        test_cases = [
            ("VE", False),
            ("300a", False),
            ("300", True),
            ("411", True),
            ("410", True),
            ("651.21s", False),
            ("924.2", True),
            ("411.1", False),
            ("C101", True),
            ("CS1", False),
            ("C101.663", True),
            ("C101.4", True),
            ("C101.21s", False),
            ("F201.11", True),
            ("C101.211", False)
        ]

        for code, expected_mappable in test_cases:
            with self.subTest(code=code, mappable=expected_mappable):
                node = gmod[code]
                self.assertEqual(expected_mappable, node.is_mappable, f"Node {code} mappability should be {expected_mappable}")

   
    def occurrences(self, parents: List[GmodNode], node: GmodNode) -> int:
        count = 0
        for parent in parents:
            if parent.code == node.code:
                count += 1
        return count




    def test_full_traversal(self):
        gmod = VIS().get_gmod(VisVersion.v3_4a)

        max_expected = TraversalOptions.DEFAULT_MAX_TRAVERSAL_OCCURRENCE

        class State:
            count:int
            max:int
            paths: List[GmodPath]

            def __init__(self):
                self.count = 0
                self.max = 0 
                self.paths = []

            def increment(self):
                self.count += 1

        def traversal_handler(state: State, parents : List[GmodNode], node : GmodNode):
            self.assertTrue(len(parents) == 0 or parents[0].is_root(), "First parent should be root or no parents")

            state.increment()
            
            def sample_test(parents : List[GmodNode], node : GmodNode) -> bool:
                if(node.code == "HG3"):
                    return True
                for parent in parents:
                    if parent.code == "HG3":
                        return True
                return False
                
            if sample_test(parents, node):
                state.paths.append(GmodPath(parents[:], node)) 

            skip_occurrence_check = Gmod.is_product_selection_assignment(parents[-1] if len(parents)>0 else None, node)
            if skip_occurrence_check:
                return TraversalHandlerResult.CONTINUE
            
            occ = self.occurrences(parents, node)
            if occ > state.max:
                state.max = occ

            return TraversalHandlerResult.CONTINUE
        
        
        state: State = State()
        
        completed = gmod.traverse(args1=state, args2=traversal_handler)
        self.assertEqual(max_expected, state.max, "Maximum occurrence should match expected")
        self.assertTrue(completed, "Traversal should complete successfully")

    @unittest.skip("This test is too slow to run in CI")
    def test_full_traversal_with_options(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        max_expected = 2
        max_occurrence = 0

        def traversal_handler(parents, node):
            skip_occurrence_check = Gmod.is_product_selection_assignment(parents[-1] if parents else None, node)
            if skip_occurrence_check:
                return TraversalHandlerResult.CONTINUE
            
            occ = self.occurrences(parents, node)
            nonlocal max_occurrence
            if occ > max_occurrence:
                max_occurrence = occ

            return TraversalHandlerResult.CONTINUE

        options = TraversalOptions(max_traversal_occurrence=max_expected)
        completed = gmod.traverse(args1=traversal_handler, args2=options)
        
        self.assertEqual(max_expected, max_occurrence, "Maximum occurrence should match the expected limit")
        self.assertTrue(completed, "Traversal should complete successfully")

    def test_partial_traversal(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        state = self.TraversalState(stop_after=5)

        def traversal_handler(state, parents, node):
            self.assertTrue(len(parents) == 0 or parents[0].is_root, "First parent should be root or no parents")
            state.node_count += 1
            if state.node_count == state.stop_after:
                return TraversalHandlerResult.STOP
            return TraversalHandlerResult.CONTINUE

        completed = gmod.traverse(args1=state, args2=traversal_handler)

        self.assertEqual(state.stop_after, state.node_count, "Traversal should stop after the specified number of nodes")
        self.assertFalse(completed, "Traversal should not complete fully (should stop early)")

    def test_full_traversal_from(self):
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        state = self.TraversalState(stop_after=0)

        def traversal_handler(state, parents, node):
            self.assertTrue(len(parents) == 0 or parents[0].code == "400a", "First parent should be root or no parents")
            state.node_count += 1
            return TraversalHandlerResult.CONTINUE

        completed = gmod.traverse(args1=state, args2=gmod["400a"] ,args3=traversal_handler)

        self.assertTrue(completed, "Traversal should complete full")

    @dataclass
    class TraversalState:
        stop_after: int
        node_count: int = 0  

if __name__ == '__main__':
    unittest.main()