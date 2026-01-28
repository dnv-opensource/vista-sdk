#!/usr/bin/env python3
"""Local ID Query Operations - Vista SDK Python.

This example demonstrates how to use Local ID queries to filter and match
Local IDs based on various criteria:
- GmodPathQuery for matching paths with or without locations
- MetadataTagsQuery for matching metadata tags
- LocalIdQuery for combining path and tag queries
"""

from vista_sdk.codebook_names import CodebookName
from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_path_query import GmodPathQueryBuilder
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_query_builder import (
    LocalIdQueryBuilder,
    NodesConfig,
    PathConfig,
)
from vista_sdk.metadata_tag_query import MetadataTagsQueryBuilder
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


def main() -> None:  # noqa: C901
    """Demonstrate Local ID query operations."""
    print("=== Local ID Query Operations ===\n")

    # Initialize VIS
    vis = VIS()
    version = VisVersion.v3_4a
    gmod = vis.get_gmod(version)

    # Sample Local IDs to query against
    local_ids = [
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1/C101.31-5/meta/qty-temperature/cnt-exhaust.gas/pos-outlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-pressure/cnt-fuel.oil",
        "/dnv-v2/vis-3-4a/411.1-2/C101.63/S206/meta/qty-pressure/cnt-lubricating.oil",
        "/dnv-v2/vis-3-9a/411.1/C101/sec/412.3/meta/qty-power",
        "/dnv-v2/vis-3-4a/511.11/C101.63/S206/meta/qty-pressure/cnt-lubricating.oil",
        "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo",
    ]

    parsed_local_ids = [LocalId.parse(lid) for lid in local_ids]

    # =========================================================================
    # 1. GmodPathQuery - Matching paths
    # =========================================================================
    print("1. GmodPathQuery - Matching Paths")
    print("-" * 50)

    # 1a. Exact path match (with locations)
    print("\n1a. Exact path match (including locations):")
    exact_path = gmod.parse_path("411.1/C101.31-2")
    exact_query = GmodPathQueryBuilder.from_path(exact_path).build()

    for lid in parsed_local_ids:
        if exact_query.match(lid.primary_item):
            print(f"   ✓ Matched: {lid}")

    # 1b. Path match ignoring locations
    print("\n1b. Path match ignoring locations (matches any cylinder):")
    path_no_loc = gmod.parse_path("411.1/C101.31-2")
    query_no_loc = (
        GmodPathQueryBuilder.from_path(path_no_loc).without_locations().build()
    )

    for lid in parsed_local_ids:
        if query_no_loc.match(lid.primary_item):
            print(f"   ✓ Matched: {lid}")

    # 1c. Match specific node with any location
    print("\n1c. Match node 411.1 with any location using Nodes builder:")
    node_411 = gmod["411.1"]
    nodes_query = (
        GmodPathQueryBuilder.empty()
        .with_node(node_411, match_all_locations=True)
        .build()
    )

    for lid in parsed_local_ids:
        if nodes_query.match(lid.primary_item):
            print(f"   ✓ Matched: {lid}")

    # 1d. Match accross VIS versions
    print(
        f"\n1d. Match path across VIS versions (3-7a and {VIS.latest_vis_version!s}):"
    )
    l1 = LocalId.parse(
        "/dnv-v2/vis-3-7a/411.1/C111.1/C103.17/meta/qty-temperature/cnt-exhaust.gas/pos-outlet"
    )
    l2 = vis.convert_local_id(l1, VIS.latest_vis_version)
    if l2 is not None:
        query = (
            GmodPathQueryBuilder.from_path(l1.primary_item).without_locations().build()
        )
        for lid in [l1, l2]:
            if query.match(lid.primary_item):
                print(f"   ✓ Matched: {lid}")

    # =========================================================================
    # 2. MetadataTagsQuery - Matching tags
    # =========================================================================
    print("\n\n2. MetadataTagsQuery - Matching Tags")
    print("-" * 50)

    # 2a. Match by single tag
    print("\n2a. Match Local IDs with temperature quantity:")
    temp_tag_query = (
        MetadataTagsQueryBuilder.empty()
        .with_tag(CodebookName.Quantity, "temperature")
        .build()
    )

    for lid in parsed_local_ids:
        if temp_tag_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 2b. Match by multiple tags
    print("\n2b. Match Local IDs with temperature AND outlet:")
    multi_tag_query = (
        MetadataTagsQueryBuilder.empty()
        .with_tag(CodebookName.Quantity, "temperature")
        .with_tag(CodebookName.Position, "outlet")
        .build()
    )

    for lid in parsed_local_ids:
        if multi_tag_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 2c. Exact tag match (no other tags allowed)
    print("\n2c. Match Local IDs with ONLY pressure and fuel.oil (exact match):")
    exact_tags_query = (
        MetadataTagsQueryBuilder.empty()
        .with_tag(CodebookName.Quantity, "pressure")
        .with_tag(CodebookName.Content, "fuel.oil")
        .with_allow_other_tags(False)
        .build()
    )

    for lid in parsed_local_ids:
        if exact_tags_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 2d. Create query from existing LocalId
    print("\n2d. Create tag query from an existing LocalId:")
    source_lid = parsed_local_ids[0]
    from_lid_query = MetadataTagsQueryBuilder.from_local_id(source_lid).build()
    print(f"   Source: {source_lid}")
    print("   Matching Local IDs:")
    for lid in parsed_local_ids:
        if from_lid_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # =========================================================================
    # 3. LocalIdQuery - Combined queries
    # =========================================================================
    print("\n\n3. LocalIdQuery - Combined Path and Tag Queries")
    print("-" * 50)

    # 3a. Match by primary item path
    print("\n3a. Match by primary item path (411.1/C101.63/S206):")
    primary_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            GmodPathQueryBuilder.from_path(
                GmodPath.parse("411.1/C101.63/S206", arg=version)
            )
            .without_locations()
            .build()
        )
        .build()
    )

    for lid in parsed_local_ids:
        if primary_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 3b. Match by tags using lambda
    print("\n3b. Match by tags using lambda configuration:")
    tags_query = (
        LocalIdQueryBuilder.empty()
        .with_tags(
            lambda tags: tags.with_tag(CodebookName.Content, "sea.water").build()
        )
        .build()
    )

    for lid in parsed_local_ids:
        if tags_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 3c. Combined path and tag query
    print("\n3c. Combined: C101.31 cylinder (any) with temperature:")
    combined_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            GmodPathQueryBuilder.from_path(
                GmodPath.parse("411.1/C101.31-2", arg=version)
            )
            .without_locations()
            .build()
        )
        .with_tags(
            lambda tags: tags.with_tag(CodebookName.Quantity, "temperature").build()
        )
        .build()
    )

    for lid in parsed_local_ids:
        if combined_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 3d. Create query from existing LocalId string
    print("\n3d. Create query from LocalId string:")
    string_query = LocalIdQueryBuilder.from_string(
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"
    ).build()

    for lid in parsed_local_ids:
        if string_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # =========================================================================
    # 4. Advanced Query Patterns
    # =========================================================================
    print("\n\n4. Advanced Query Patterns")
    print("-" * 50)

    # 4a. Using NodesConfig for flexible node matching
    print("\n4a. Using NodesConfig to match specific nodes (must have 411.1 and S206):")
    node_config_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            NodesConfig(
                lambda nodes: nodes.with_node(gmod["411.1"], match_all_locations=True)
                .with_node(gmod["S206"], match_all_locations=True)
                .build()
            )
        )
        .build()
    )

    for lid in parsed_local_ids:
        if node_config_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 4b. Using PathConfig to modify path matching
    print("\n4b. Using PathConfig to customize path matching:")
    base_path = GmodPath.parse("411.1/C101.31-2", arg=version)
    path_config_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            base_path,
            PathConfig(lambda path: path.without_locations().build()),
        )
        .build()
    )

    for lid in parsed_local_ids:
        if path_config_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 4c. Using with_any_node_before to match only the leaf node
    print(
        "\n4c. Using with_any_node_before to ignore parent nodes (Any path before S206):"  # noqa: E501
    )
    sensor_path = GmodPath.parse("411.1/C101.63/S206", arg=version)
    any_before_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            sensor_path,
            PathConfig(
                lambda path: path.without_locations()
                .with_any_node_before(lambda nodes: nodes["S206"])
                .build()
            ),
        )
        .build()
    )

    for lid in parsed_local_ids:
        if any_before_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 4d. Using with_any_node_after for prefix-style matching (411.1/*)
    print("\n4d. Using with_any_node_after for prefix matching (411.1/*):")
    print("   Match any path starting with 411.1, regardless of children:")
    prefix_path = GmodPath.parse("411.1/C101.31", arg=version)
    any_after_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            prefix_path,
            PathConfig(
                lambda path: path.without_locations()
                .with_any_node_after(lambda nodes: nodes["411.1"])
                .build()
            ),
        )
        .build()
    )

    for lid in parsed_local_ids:
        if any_after_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 4e. Query requiring NO secondary item
    print("\n4e. Match Local IDs that includes 411.1 and is without secondary item:")
    no_secondary_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            NodesConfig(
                lambda nodes: nodes.with_node(
                    gmod["411.1"], match_all_locations=True
                ).build()
            )
        )
        .without_secondary_item()
        .build()
    )

    for lid in parsed_local_ids:
        if no_secondary_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # 4f. Query with any or no secondary item
    print("\n4f. Match Local IDs that includes 411.1 with any secondary item:")
    any_secondary_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            NodesConfig(
                lambda nodes: nodes.with_node(
                    gmod["411.1"], match_all_locations=True
                ).build()
            )
        )
        .with_any_secondary_item()
        .build()
    )

    for lid in parsed_local_ids:
        if any_secondary_query.match(lid):
            print(f"   ✓ Matched: {lid}")

    # =========================================================================
    # 5. Practical Use Case: Filtering Sensor Data
    # =========================================================================
    print("\n\n5. Practical Use Case: Filtering Sensor Data")
    print("-" * 50)

    print("\n   Scenario: Find all temperature sensors on the main engine")

    # Define query for main engine temperature sensors
    me_path = GmodPath.parse("411.1/C101", arg=version)
    engine_temp_query = (
        LocalIdQueryBuilder.empty()
        .with_primary_item(
            me_path,
            PathConfig(
                lambda path: path.with_any_node_after(lambda nodes: nodes["C101"])
                .without_locations()
                .build()
            ),
        )
        .with_tags(
            lambda tags: tags.with_tag(CodebookName.Quantity, "temperature").build()
        )
        .build()
    )

    print("   Query: Primary item contains 411.1/C101 (any location) + qty=temperature")
    print("   Results:")
    for lid in parsed_local_ids:
        if engine_temp_query.match(lid):
            print(f"   ✓ {lid}")

    print("\n   Scenario: Find all pressure sensors with fuel or lubricating oil")
    print("   Note: OR conditions require multiple queries")

    # Query for pressure + fuel.oil
    fuel_oil_query = (
        LocalIdQueryBuilder.empty()
        .with_tags(
            lambda tags: tags.with_tag(CodebookName.Quantity, "pressure")
            .with_tag(CodebookName.Content, "fuel.oil")
            .with_allow_other_tags(True)
            .build()
        )
        .build()
    )

    # Query for pressure + lubricating.oil
    lube_oil_query = (
        LocalIdQueryBuilder.empty()
        .with_tags(
            lambda tags: tags.with_tag(CodebookName.Quantity, "pressure")
            .with_tag(CodebookName.Content, "lubricating.oil")
            .with_allow_other_tags(True)
            .build()
        )
        .build()
    )

    print("   Query: qty=pressure AND (cnt=fuel.oil OR cnt=lubricating.oil)")
    print("   Results:")
    for lid in parsed_local_ids:
        if fuel_oil_query.match(lid) or lube_oil_query.match(lid):
            print(f"   ✓ {lid}")

    print("\n=== Query Operations Complete ===")


if __name__ == "__main__":
    main()
