#!/usr/bin/env python3
"""GMOD Operations - Vista SDK Python.

This example demonstrates working with GMOD (Generic Product Model):
- Parsing and traversing GMOD paths
- Exploring node hierarchies
- Working with node metadata
- Version conversion operations
"""

from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.traversal_handler_result import TraversalHandlerResult
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


def main() -> None:  # noqa : C901
    """Demonstrate GMOD operations."""
    print("=== GMOD Operations Example ===\n")

    # Initialize VIS
    vis = VIS()
    version = VisVersion.v3_4a
    gmod = vis.get_gmod(version)

    # 1. Basic GMOD path operations
    print("1. Basic GMOD Path Operations...")

    sample_paths = [
        "411.1",
        "411.1/C101.31-2",
        "612.21-1/C701.13/S93",
        "1021.1i-6P/H123",
    ]

    for path_str in sample_paths:
        try:
            path = gmod.parse_path(path_str)
            # or
            path = GmodPath.parse(path_str, arg=version)
            node = path.node
            print(f"   Path: {path_str}")
            print(f"     → Code: {node.code}")
            print(f"     → Name: {node.metadata.name}")
            print(f"     → Common Name: {node.metadata.common_name}")
            print(f"     → Depth: {path.length}")
            print()
        except Exception as e:
            print(f"   ✗ Failed to parse {path_str}: {e}")

    # 2. Exploring node hierarchies
    print("2. Exploring Node Hierarchies...")

    root_path = gmod.parse_path("411.1")
    root_node = root_path.node

    print(f"   Root node: {root_node.code} - {root_node.metadata.common_name}")
    print("   Child nodes:")

    # List immediate children
    children = list(root_node.children)[:5]  # Show first 5 children
    for child in children:
        print(f"     - {child.code}: {child.metadata.common_name}")

    # 3. Path traversal and analysis
    print("\n3. Path Traversal and Analysis...")

    deep_path = gmod.parse_path("411.1/C101.31-2")

    print(f"   Analyzing path: {deep_path}")
    print("   Full path traversal:")
    common_names = {c[0]: c[1] for c in deep_path.get_common_names()}
    for depth, node in deep_path.get_full_path():
        if depth == 0:
            continue  # Skip root
        # Common names must be looked up as they often are a combination between parent and child  # noqa: E501
        common_name = common_names.get(depth)
        if common_name is None:
            common_name = node.metadata.common_name
        indent = "     " * (depth - 1)
        print(f"   {indent}Depth {depth}: {node.code} - {common_name}")

    # 4. Node properties and metadata
    print("\n4. Node Properties and Metadata...")

    sample_nodes = ["411.1", "C101.31", "S206"]

    for node_code in sample_nodes:
        # Access through try_get pattern...
        success, lookup_node = gmod.try_get_node(node_code)
        try:
            # Or through direct lookup
            lookup_node = gmod[node_code]
            if lookup_node is None:
                raise ValueError(f"Node {node_code} not found")
            print(f"   Node: {node_code}")
            print(f"     → Name: {lookup_node.metadata.name}")
            print(f"     → Common Name: {lookup_node.metadata.common_name}")
            print(f"     → Is Mappable: {lookup_node.is_mappable}")
            print(f"     → Has Children: {len(lookup_node.children) > 0}")
            print()
        except Exception as e:
            print(f"   ✗ Node {node_code} not found: {e}")

    # 5. Path validation and error handling
    print("\n5. Path Validation and Error Handling...")

    invalid_paths = ["invalid.node", "411.1/invalid-child", "999.999/does-not-exist"]

    for invalid_path in invalid_paths:
        try:
            success, result = gmod.try_parse_path(invalid_path)
            if not success or result is None:
                print(f"   ✗ Path '{invalid_path}' is invalid (as expected)")
            else:
                print(f"   ⚠ Path '{invalid_path}' unexpectedly parsed as: {result}")
        except Exception as e:
            print(f"   ✗ Exception parsing '{invalid_path}': {e}")

    # 6. GMOD version conversion (if available)
    print("\n6. GMOD Version Conversion...")

    try:
        conversion_tests = [
            ("111.3/H402", VisVersion.v3_7a, VisVersion.v3_8a),
            ("846/G203", VisVersion.v3_7a, VisVersion.v3_8a),
        ]

        for old_path_str, source_version, target_version in conversion_tests:
            try:
                # Parse the old path
                old_path = GmodPath.parse(old_path_str, arg=source_version)

                # Convert using VIS convert_path method
                new_path = vis.convert_path(source_version, old_path, target_version)
                if new_path:
                    # No changes are very common
                    print(f"   ✓ Converted: {old_path_str} → {new_path}")
                else:
                    print(f"   ✗ Conversion returned None for {old_path_str}")
            except Exception as e:
                print(f"   ✗ Conversion failed for {old_path_str}: {e}")

    except Exception as e:
        print(f"   ⚠ Version conversion not available: {e}")

    print("\n7. GMOD Traversal...")
    node_count = 0

    def simple_handler(_: list[GmodNode], __: GmodNode) -> TraversalHandlerResult:
        nonlocal node_count
        node_count += 1
        return TraversalHandlerResult.CONTINUE

    completed = gmod.traverse(simple_handler)
    print(f"   Traversal completed: {completed}, Nodes visited: {node_count}")
    first_leaf: GmodNode | None = None

    def early_stop_handler(_: list[GmodNode], node: GmodNode) -> TraversalHandlerResult:
        nonlocal first_leaf
        if node.is_leaf_node:
            first_leaf = node
            return TraversalHandlerResult.STOP
        return TraversalHandlerResult.CONTINUE

    completed = gmod.traverse(early_stop_handler)
    print(f"   Traversal completed: {completed}, First leaf node: {first_leaf}")

    start_node = gmod["411"]
    child_cound = 0

    def from_node_handler(
        _: list[GmodNode],
        __: GmodNode,
    ) -> TraversalHandlerResult:
        nonlocal child_cound
        child_cound += 1
        return TraversalHandlerResult.CONTINUE

    completed = gmod.traverse(start_node, from_node_handler)

    print(f"   Traversal completed: {completed}, Nodes under '411': {child_cound}")

    print("\n=== GMOD operations completed! ===")


if __name__ == "__main__":
    main()
