#!/usr/bin/env python3
"""GMOD Operations - Vista SDK Python.

This example demonstrates working with GMOD (General Model of Data):
- Parsing and traversing GMOD paths
- Exploring node hierarchies
- Working with node metadata
- Version conversion operations
"""

from vista_sdk.gmod_path import GmodPath
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

    root_path = GmodPath.parse("411.1", arg=version)
    root_node = root_path.node

    print(f"   Root node: {root_node.code} - {root_node.metadata.common_name}")
    print("   Child nodes:")

    # List immediate children
    children = list(root_node.children)[:5]  # Show first 5 children
    for child in children:
        print(f"     - {child.code}: {child.metadata.common_name}")

    # 3. Path traversal and analysis
    print("\n3. Path Traversal and Analysis...")

    deep_path = GmodPath.parse("411.1/C101.31-2", arg=version)

    print(f"   Analyzing path: {deep_path}")
    print("   Full path traversal:")

    for depth, node in deep_path.get_full_path():
        if depth == 0:
            continue  # Skip root
        indent = "     " * (depth - 1)
        print(f"   {indent}Depth {depth}: {node.code} - {node.metadata.common_name}")

    # 4. Node properties and metadata
    print("\n4. Node Properties and Metadata...")

    sample_nodes = ["411.1", "C101.31", "S206"]

    for node_code in sample_nodes:
        try:
            node = gmod[node_code]
            print(f"   Node: {node_code}")
            print(f"     → Name: {node.metadata.name}")
            print(f"     → Common Name: {node.metadata.common_name}")
            print(f"     → Is Mappable: {node.is_mappable}")
            print(f"     → Has Children: {len(list(node.children)) > 0}")
            print()
        except Exception as e:
            print(f"   ✗ Node {node_code} not found: {e}")

    # 5. Working with different VIS versions
    print("5. Working with Different VIS Versions...")

    versions_to_test = [VisVersion.v3_4a, VisVersion.v3_5a, VisVersion.v3_6a]
    test_path = "411.1/C101.31"

    for test_version in versions_to_test:
        try:
            version_gmod = vis.get_gmod(test_version)
            parsed_path = GmodPath.parse(test_path, arg=test_version)
            node = parsed_path.node

            print(f"   VIS {test_version}:")
            print(f"     → Path exists: {parsed_path is not None}")
            print(f"     → Node name: {node.metadata.common_name}")
            print(f"     → Total nodes in GMOD: {sum(1 for _ in version_gmod)}")  # type: ignore

        except Exception as e:
            print(f"   ✗ Error with version {test_version}: {e}")

    # 6. Path validation and error handling
    print("\n6. Path Validation and Error Handling...")

    invalid_paths = ["invalid.node", "411.1/invalid-child", "999.999/does-not-exist"]

    for invalid_path in invalid_paths:
        try:
            success, result = GmodPath.try_parse(invalid_path, arg=version)
            if not success or result is None:
                print(f"   ✗ Path '{invalid_path}' is invalid (as expected)")
            else:
                print(f"   ⚠ Path '{invalid_path}' unexpectedly parsed as: {result}")
        except Exception as e:
            print(f"   ✗ Exception parsing '{invalid_path}': {e}")

    # 7. GMOD version conversion (if available)
    print("\n7. GMOD Version Conversion...")

    try:
        conversion_tests = [
            ("411.1/C101.72/I101", VisVersion.v3_4a, VisVersion.v3_5a),
            ("612.21/C701.13/S93", VisVersion.v3_5a, VisVersion.v3_6a),
        ]

        for old_path_str, source_version, target_version in conversion_tests:
            try:
                # Parse the old path
                old_path = GmodPath.parse(old_path_str, arg=source_version)

                # Convert using VIS convert_path method
                new_path = vis.convert_path(source_version, old_path, target_version)
                if new_path:
                    print(f"   ✓ Converted: {old_path_str} → {new_path}")
                else:
                    print(f"   ✗ Conversion returned None for {old_path_str}")
            except Exception as e:
                print(f"   ✗ Conversion failed for {old_path_str}: {e}")

    except Exception as e:
        print(f"   ⚠ Version conversion not available: {e}")

    print("\n=== GMOD operations completed! ===")


if __name__ == "__main__":
    main()
