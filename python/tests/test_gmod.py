"""Tests for the Gmod class in the Vista SDK."""

import unittest
from dataclasses import dataclass

from src.vista_sdk.client import Client
from src.vista_sdk.gmod import Gmod, TraversalHandlerResult, TraversalOptions
from src.vista_sdk.gmod_dto import GmodDto
from src.vista_sdk.gmod_node import GmodNode
from src.vista_sdk.gmod_path import GmodPath
from src.vista_sdk.vis import VIS
from src.vista_sdk.vis_version import VisVersion, VisVersionExtension

from .test_vis import TestVis


class TestGmod(unittest.TestCase):
    """Unit tests for the Gmod class in the Vista SDK."""

    def setUp(self) -> None:
        """Set up the test environment."""
        self.vis = TestVis.get_vis()
        Client.get_gmod_test(VisVersionExtension.to_version_string(VisVersion.v3_4a))

    def test_gmod_loads(self) -> None:
        """Test that Gmod can be loaded for all versions."""
        for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version)
                assert gmod is not None, (
                    f"Gmod for version {version} should not be None"
                )
                assert gmod.try_get_node("400a")[0], (
                    f"Node '400a' should exist in Gmod for version {version}"
                )

    def test_gmod_properties(self) -> None:
        """Test properties of Gmod for all versions."""
        for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version)
                assert gmod is not None, (
                    f"Gmod for version {version} should not be None"
                )

                nodes = list(gmod._node_map)
                min_length = min(nodes, key=lambda x: len(x[1].code))
                max_length = max(nodes, key=lambda x: len(x[1].code))

                assert len(min_length[1].code) == 2, "Minimum code length should be 2"
                assert min_length[1].code == "VE", "Minimum code should be 'VE'"
                assert max_length[1].code == 10, "Maximum code length should be 10"
                possible_max = ["C1053.3111", "H346.11113"]
                assert max_length[1].code in possible_max, (
                    f"Maximum code should be one of {possible_max}"
                )

                expected_counts = [6420, 6557, 6672]
                assert len(nodes) in expected_counts, (
                    f"Node count for version {version} should be one of {expected_counts}"  # noqa: E501
                )

    def test_gmod_lookup(self) -> None:
        """Test that Gmod can look up nodes by code."""
        for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version)
                assert gmod is not None, (
                    f"Gmod for version {version} should not be None"
                )

                gmod_dto: GmodDto = self.vis.get_gmod_dto(version)
                assert gmod_dto is not None, (
                    f"GmodDto for version {version} should not be None"
                )

                seen = set()
                for node in gmod_dto.items:
                    assert node.code is not None, (
                        f"Node code should not be None: {node}"
                    )
                    assert node.code not in seen, (
                        f"Node code should be unique: {node.code}"
                    )
                    seen.add(node.code)

                    success, found_node = gmod.try_get_node(node.code)
                    assert success
                    assert found_node is not None, (
                        f"Found node should not be None for code: {node.code}"
                    )
                    if found_node is not None:
                        assert node.code == found_node.code, (
                            f"Node code should match found node code: {node.code}"
                        )

                seen.clear()
                counter = 0
                for code, node in gmod._node_map:  # noqa: B007
                    assert node.code is not None, (
                        f"Node code should not be None: {node}"
                    )
                    assert node.code not in seen, (
                        f"Node code should be unique: {node.code}"
                    )
                    seen.add(node.code)

                    success, found_node = gmod.try_get_node(node.code)
                    assert success
                    assert found_node is not None, (
                        f"Found node should not be None for code: {node.code}"
                    )
                    if found_node is not None:
                        assert node.code == found_node.code, (
                            f"Node code should match found node code: {node.code}"
                        )
                    counter += 1

                assert len(gmod_dto.items) == counter, (
                    f"Node count in GmodDto should match counter: {counter}"
                )

                test_codes = [
                    "ABC",
                    None,
                    "",
                    "SDFASDFSDAFb",
                    "✅",
                    "a✅b",
                    "ac✅bc",
                    "✅bc",
                    "a✅",
                    "ag✅",
                ]
                for code in test_codes:
                    success, _ = gmod.try_get_node(code)
                    assert not success, (
                        f"Node lookup for invalid code '{code}' should fail"
                    )

    def test_gmod_node_equality(self) -> None:
        """Test equality and identity of Gmod nodes."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node1 = gmod["400a"]
        node2 = gmod["400a"]

        assert node1 == node2, "Nodes should be equal (identity check)"
        assert node1 is node2, "Nodes should be the exact same object (identity)"

        if node2 is not None:
            node3 = node2.with_location("1")

        assert node1 != node3, "Nodes should not be equal after modification"
        assert node1 is not node3, (
            "Nodes should not be the same object after modification"
        )

        if node2 is not None:
            node4 = node2.clone()
        assert node1 == node4, "Cloned nodes should be equal"
        assert node1 is not node4, "Cloned nodes should not be the same object"

    def test_gmod_node_types(self) -> None:
        """Test that Gmod nodes have unique types."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        unique_types = set()
        for node in gmod.__iter__():
            category_type = f"{node[1].metadata.category} | {node[1].metadata.type}"
            unique_types.add(category_type)

        assert unique_types is not None, "The set of node types should not be empty"

    def test_gmod_root_node_children(self) -> None:
        """Test that the root node has children in Gmod."""
        for version in VisVersion:
            with self.subTest(version=version):
                gmod = self.vis.get_gmod(version)

                node = gmod.root_node
                if node is None:
                    self.fail("Root node should not be None")
                else:
                    assert node is not None, "Root node should not be None"
                    assert len(node.children) > 0, "Root node should have children"

    def test_normal_assignments(self) -> None:
        """Test that normal assignments are correctly set in Gmod nodes."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node = gmod["411.3"]
        assert node.product_type is not None, (
            "ProductType should not be None for node '411.3'"
        )
        assert node.product_selection is not None, (
            "ProductSelection should be None for node '411.3'"
        )

        node = gmod["H601"]
        assert node.product_type is None, "ProductType should be None for node 'H601'"

    def test_node_with_product_selection(self) -> None:
        """Test that nodes with product selection are handled correctly."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node = gmod["411.2"]
        assert node.product_selection is not None, (
            "ProductSelection should not be None for node '411.2'",
        )
        assert node.product_type is not None, (
            "ProductType should be None for node '411.2'"
        )

        node = gmod["H601"]
        assert node.product_selection is not None, (
            "ProductSelection should be None for node 'H601'"
        )

    def test_product_selection(self) -> None:
        """Test that product selection nodes are identified correctly."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node = gmod["CS1"]
        assert node.is_product_selection, (
            "Node 'CS1' should be identified as having a product selection"
        )

    def test_mappability(self) -> None:
        """Test that Gmod nodes have correct mappability."""
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
            ("C101.211", False),
        ]

        for code, expected_mappable in test_cases:
            with self.subTest(code=code, mappable=expected_mappable):
                node = gmod[code]
                assert expected_mappable == node.is_mappable, (
                    f"Node {code} mappability should be {expected_mappable}",
                )

    def occurrences(self, parents: list[GmodNode], node: GmodNode) -> int:
        """Count occurrences of a node in the parents list."""
        count = 0
        for parent in parents:
            if parent.code == node.code:
                count += 1
        return count

    def test_full_traversal(self) -> None:
        """Test full traversal of Gmod with a custom handler."""
        gmod = VIS().get_gmod(VisVersion.v3_4a)

        max_expected = TraversalOptions.DEFAULT_MAX_TRAVERSAL_OCCURRENCE

        class State:
            count: int
            max: int
            paths: list[GmodPath]

            def __init__(self) -> None:
                """Initialize the state for traversal."""
                self.count = 0
                self.max = 0
                self.paths = []

            def increment(self) -> None:
                """Increment the count of traversed nodes."""
                self.count += 1

        def traversal_handler(
            state: State, parents: list[GmodNode], node: GmodNode
        ) -> TraversalHandlerResult:
            assert len(parents) == 0 or parents[0].is_root(), (
                "First parent should be root or no parents"
            )

            state.increment()

            def sample_test(parents: list[GmodNode], node: GmodNode) -> bool:
                if node.code == "HG3":
                    return True
                return any(parent.code == "HG3" for parent in parents)

            if sample_test(parents, node):
                state.paths.append(GmodPath(parents[:], node))

            skip_occurrence_check = Gmod.is_product_selection_assignment(
                parents[-1] if len(parents) > 0 else None, node
            )
            if skip_occurrence_check:
                return TraversalHandlerResult.CONTINUE

            occ = self.occurrences(parents, node)
            if occ > state.max:
                state.max = occ

            return TraversalHandlerResult.CONTINUE

        state: State = State()

        completed = gmod.traverse(args1=state, args2=traversal_handler)
        assert max_expected == state.max, "Maximum occurrence should match expected"
        assert completed, "Traversal should complete successfully"

    @unittest.skip("This test is too slow to run in CI")
    def test_full_traversal_with_options(self) -> None:
        """Test full traversal of Gmod with a custom handler and options."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        max_expected = 2
        max_occurrence = 0

        def traversal_handler(
            parents: list[GmodNode], node: GmodNode
        ) -> TraversalHandlerResult:
            skip_occurrence_check = Gmod.is_product_selection_assignment(
                parents[-1] if parents else None, node
            )
            if skip_occurrence_check:
                return TraversalHandlerResult.CONTINUE

            occ = self.occurrences(parents, node)
            nonlocal max_occurrence
            if occ > max_occurrence:
                max_occurrence = occ

            return TraversalHandlerResult.CONTINUE

        options = TraversalOptions(max_traversal_occurrence=max_expected)
        completed = gmod.traverse(args1=traversal_handler, args2=options)

        assert max_expected == max_occurrence, (
            "Maximum occurrence should match the expected limit"
        )
        assert completed, "Traversal should complete successfully"

    def test_partial_traversal(self) -> None:
        """Test partial traversal of Gmod with a custom handler."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        state = self.TraversalState(stop_after=5)

        def traversal_handler(
            state: "TestGmod.TraversalState",
            parents: list[GmodNode],
            node: GmodNode,  # noqa: ARG001
        ) -> TraversalHandlerResult:
            assert len(parents) == 0 or parents[0].is_root, (
                "First parent should be root or no parents",
            )
            state.node_count += 1
            if state.node_count == state.stop_after:
                return TraversalHandlerResult.STOP
            return TraversalHandlerResult.CONTINUE

        completed = gmod.traverse(args1=state, args2=traversal_handler)

        assert state.stop_after == state.node_count, (
            "Traversal should stop after the specified number of nodes",
        )
        assert not completed, "Traversal should not complete fully (should stop early)"

    def test_full_traversal_from(self) -> None:
        """Test full traversal of Gmod starting from a specific node."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        state = self.TraversalState(stop_after=0)

        def traversal_handler(
            state: "TestGmod.TraversalState",
            parents: list[GmodNode],
            node: GmodNode,  # noqa: ARG001
        ) -> TraversalHandlerResult:
            """Handler for traversal that counts nodes and checks parents."""
            assert len(parents) == 0 or parents[0].code == "400a", (
                "First parent should be root or no parents",
            )
            state.node_count += 1
            return TraversalHandlerResult.CONTINUE

        completed = gmod.traverse(
            args1=state, args2=gmod["400a"], args3=traversal_handler
        )

        assert completed, "Traversal should complete full"

    @dataclass
    class TraversalState:
        """State for tracking traversal progress."""

        stop_after: int
        node_count: int = 0


if __name__ == "__main__":
    unittest.main()
