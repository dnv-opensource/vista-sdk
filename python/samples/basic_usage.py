#!/usr/bin/env python3
"""Basic Usage Example - Vista SDK Python.

This example demonstrates the fundamental operations of the Vista SDK:
- Initializing VIS
- Working with GMOD paths
- Creating Local IDs
- Using codebooks

For more examples, see the other files in this directory.
"""

from vista_sdk.codebook_names import CodebookName
from vista_sdk.gmod import Gmod
from vista_sdk.gmod_path import GmodPath
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_builder import LocalIdBuilder
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


def main() -> None:  # noqa: C901
    """Demonstrate basic Vista SDK usage."""
    print("=== Vista SDK Basic Usage Example ===\n")

    # Initialize VIS instance
    print("1. Initializing VIS...")
    vis = VIS()

    # Get data for VIS version 3.4a
    version = VisVersion.v3_4a
    print(f"   Loading data for VIS version: {version}")

    gmod: Gmod = vis.get_gmod(version)
    codebooks = vis.get_codebooks(version)

    # Count nodes by iterating through the gmod nodes
    node_count = sum(1 for _ in gmod)  # type: ignore
    print(f"   ✓ GMOD loaded with {node_count} nodes")
    print("   ✓ Codebooks loaded")
    print("   ✓ Locations loaded\n")

    # Work with Gmod node
    print("2. Working with Gmod nodes...")

    engine_node = gmod["C101"]
    print(f"   Node code: {engine_node.code}")
    print(f"   Common name: {engine_node.metadata.common_name}")
    print(f"   Category: {engine_node.metadata.category}")
    print(f"   Is leaf: {engine_node.is_leaf_node}")

    children = list(engine_node.children)
    print(f"   Children ({len(children)}):")
    for child in children[:5]:
        print(f"     → {child.code}: {child.metadata.common_name}")

    if len(children) > 5:
        print(f"     ... and {len(children) - 5} more")

    print()

    # Parse GMOD paths
    print("3. Working with GMOD paths...")
    path_strings = ["411.1/C101.31-2", "612.21-1/C701.13/S93", "1021.1i-6P/H123"]

    paths = []
    for path_str in path_strings:
        try:
            path = GmodPath.parse(path_str, arg=version)
            paths.append(path)
            node = path.node
            print(f"   ✓ Parsed: {path_str}")
            print(f"     → Node: {node.code} ({node.metadata.common_name})")
            print(f"     → Depth: {path.length}")
        except Exception as e:
            print(f"   ✗ Failed to parse {path_str}: {e}")

    print()

    # Work with codebooks
    print("4. Using codebooks...")

    # Get specific codebooks
    quantity_book = codebooks[CodebookName.Quantity]
    position_book = codebooks[CodebookName.Position]
    content_book = codebooks[CodebookName.Content]

    # Create metadata tags
    tags = {
        "quantity": quantity_book.create_tag("temperature"),
        "position": position_book.create_tag("centre"),
        "content": content_book.create_tag("cooling.water"),
    }

    for tag_type, tag in tags.items():
        print(f"   ✓ Created {tag_type} tag: {tag.value} (custom: {tag.is_custom})")

    print()

    # Build Local IDs
    print("5. Creating Local IDs...")

    for i, path in enumerate(paths[:2]):  # Use first two paths
        try:
            local_id = (
                LocalIdBuilder.create(version)
                .with_primary_item(path)
                .with_metadata_tag(tags["quantity"])
                .with_metadata_tag(tags["content"])
                .with_metadata_tag(tags["position"])
                .build()
            )

            print(f"   ✓ Local ID {i + 1}: {local_id}")
            print(f"     → Primary item: {local_id.primary_item}")

        except Exception as e:
            print(f"   ✗ Failed to create Local ID for {path}: {e}")

    print()

    # Demonstrate parsing existing Local IDs
    print("6. Parsing existing Local IDs...")

    sample_local_ids = [
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
    ]

    for local_id_str in sample_local_ids:
        try:
            local_id = LocalId.parse(local_id_str)
            print(f"   ✓ Parsed: {local_id_str}")
            print(f"     → Primary: {local_id.primary_item}")
            print(f"     → Metadata tags: {len(local_id.metadata_tags)}")

            for tag in local_id.metadata_tags:
                print(f"       - {tag.name.name}: {tag.value}")

        except Exception as e:
            print(f"   ✗ Failed to parse {local_id_str}: {e}")

    print("\n=== Example completed successfully! ===")


if __name__ == "__main__":
    main()
