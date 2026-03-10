/*
    Local ID Query Operations - Vista SDK JS/TS

    This example demonstrates how to use Local ID queries to filter and match
    Local IDs based on various criteria:
    - GmodPathQuery for matching paths with or without locations
    - MetadataTagsQuery for matching metadata tags
    - LocalIdQuery for combining path and tag queries
*/

import {
    CodebookName,
    GmodPath,
    GmodPathQueryBuilder,
    LocalId,
    LocalIdQueryBuilder,
    MetadataTagsQueryBuilder,
    VIS,
    VisVersion,
} from "dnv-vista-sdk";

async function main() {
    console.log("=== Local ID Query Operations ===\n");

    const version = VisVersion.v3_4a;
    const vis = VIS.instance;
    const { gmod, codebooks, locations } = await vis.getVIS(version);

    // Sample Local IDs to query against
    const localIdStrings = [
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1/C101.31-5/meta/qty-temperature/cnt-exhaust.gas/pos-outlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-pressure/cnt-fuel.oil",
        "/dnv-v2/vis-3-4a/411.1-2/C101.63/S206/meta/qty-pressure/cnt-lubricating.oil",
        "/dnv-v2/vis-3-4a/511.11/C101.63/S206/meta/qty-pressure/cnt-lubricating.oil",
        "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo",
    ];

    const parsedLocalIds: LocalId[] = [];
    for (const str of localIdStrings) {
        parsedLocalIds.push(LocalId.parse(str, gmod, codebooks, locations));
    }

    // =========================================================================
    // 1. GmodPathQuery - Matching paths
    // =========================================================================
    console.log("1. GmodPathQuery - Matching Paths");
    console.log("-".repeat(50));

    // 1a. Exact path match (with locations)
    console.log("\n1a. Exact path match (including locations):");
    const exactPath = gmod.parsePath("411.1/C101.31-2", locations);
    const exactQuery = GmodPathQueryBuilder.from(exactPath).build();

    for (const lid of parsedLocalIds) {
        if (await exactQuery.match(lid.primaryItem)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 1b. Path match ignoring locations
    console.log("\n1b. Path match ignoring locations (matches any cylinder):");
    const pathNoLoc = gmod.parsePath("411.1/C101.31-2", locations);
    const queryNoLoc = GmodPathQueryBuilder.from(pathNoLoc)
        .withoutLocations()
        .build();

    for (const lid of parsedLocalIds) {
        if (await queryNoLoc.match(lid.primaryItem)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 1c. Match specific node with any location
    console.log(
        "\n1c. Match node 411.1 with any location using Nodes builder:",
    );
    const node411 = gmod.getNode("411.1");
    const nodesQuery = GmodPathQueryBuilder.empty()
        .withNode(node411, true)
        .build();

    for (const lid of parsedLocalIds) {
        if (await nodesQuery.match(lid.primaryItem)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // =========================================================================
    // 2. MetadataTagsQuery - Matching tags
    // =========================================================================
    console.log("\n\n2. MetadataTagsQuery - Matching Tags");
    console.log("-".repeat(50));

    // 2a. Match by single tag
    console.log("\n2a. Match Local IDs with temperature quantity:");
    const tempTagQuery = MetadataTagsQueryBuilder.empty()
        .withTag(CodebookName.Quantity, "temperature")
        .build();

    for (const lid of parsedLocalIds) {
        if (tempTagQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 2b. Match by multiple tags
    console.log("\n2b. Match Local IDs with temperature AND outlet:");
    const multiTagQuery = MetadataTagsQueryBuilder.empty()
        .withTag(CodebookName.Quantity, "temperature")
        .withTag(CodebookName.Position, "outlet")
        .build();

    for (const lid of parsedLocalIds) {
        if (multiTagQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 2c. Exact tag match (no other tags allowed)
    console.log(
        "\n2c. Match Local IDs with ONLY pressure and fuel.oil (exact match):",
    );
    const exactTagsQuery = MetadataTagsQueryBuilder.empty()
        .withTag(CodebookName.Quantity, "pressure")
        .withTag(CodebookName.Content, "fuel.oil")
        .withAllowOtherTags(false)
        .build();

    for (const lid of parsedLocalIds) {
        if (exactTagsQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 2d. Create query from existing LocalId
    console.log("\n2d. Create tag query from an existing LocalId:");
    const sourceLid = parsedLocalIds[0];
    const fromLidQuery = MetadataTagsQueryBuilder.from(sourceLid).build();
    console.log(`   Source: ${sourceLid}`);
    console.log("   Matching Local IDs:");
    for (const lid of parsedLocalIds) {
        if (fromLidQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // =========================================================================
    // 3. LocalIdQuery - Combined queries
    // =========================================================================
    console.log("\n\n3. LocalIdQuery - Combined Path and Tag Queries");
    console.log("-".repeat(50));

    // 3a. Match by primary item path
    console.log("\n3a. Match by primary item path (411.1/C101.63/S206):");
    const primaryQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromPath(
            gmod.parsePath("411.1/C101.63/S206", locations),
            (path) => path.withoutLocations().build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await primaryQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 3b. Match by tags
    console.log("\n3b. Match by tags:");
    const tagsQuery = LocalIdQueryBuilder.empty()
        .withTags((tags) =>
            tags.withTag(CodebookName.Content, "sea.water").build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await tagsQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 3c. Combined path and tag query
    console.log("\n3c. Combined: C101.31 cylinder (any) with temperature:");
    const combinedQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromPath(
            gmod.parsePath("411.1/C101.31-2", locations),
            (path) => path.withoutLocations().build(),
        )
        .withTags((tags) =>
            tags.withTag(CodebookName.Quantity, "temperature").build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await combinedQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 3d. Create query from existing LocalId string
    console.log("\n3d. Create query from LocalId string:");
    const stringQuery = (
        await LocalIdQueryBuilder.fromStr(
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        )
    ).build();

    for (const lid of parsedLocalIds) {
        if (await stringQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // =========================================================================
    // 4. Advanced Query Patterns
    // =========================================================================
    console.log("\n\n4. Advanced Query Patterns");
    console.log("-".repeat(50));

    // 4a. Using NodesConfig for flexible node matching
    console.log(
        "\n4a. Using nodes builder to match specific nodes (must have 411.1 and S206):",
    );
    const nodeConfigQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromNodes((nodes) =>
            nodes
                .withNode(gmod.getNode("411.1"), true)
                .withNode(gmod.getNode("S206"), true)
                .build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await nodeConfigQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 4b. Using PathConfig to modify path matching
    console.log("\n4b. Using path configuration to customize path matching:");
    const basePath = GmodPath.parse("411.1/C101.31-2", locations, gmod);
    const pathConfigQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromPath(basePath, (path) =>
            path.withoutLocations().build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await pathConfigQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 4c. Using WithAnyNodeBefore to match only the leaf node
    console.log(
        "\n4c. Using WithAnyNodeBefore to ignore parent nodes (any path before S206):",
    );
    const sensorPath = gmod.parsePath("411.1/C101.63/S206", locations);
    const anyBeforeQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromPath(sensorPath, (path) =>
            path
                .withoutLocations()
                .withAnyNodeBefore((nodes) => nodes.get("S206")!)
                .build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await anyBeforeQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 4d. Using WithAnyNodeAfter for prefix-style matching
    console.log("\n4d. Using WithAnyNodeAfter for prefix matching (411.1/*):");
    console.log("   Match any path with 411.1, regardless of children:");
    const prefixPath = gmod.parsePath("411.1/C101.31", locations);
    const anyAfterQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromPath(prefixPath, (path) =>
            path
                .withoutLocations()
                .withAnyNodeAfter((nodes) => nodes.get("411.1")!)
                .build(),
        )
        .build();

    for (const lid of parsedLocalIds) {
        if (await anyAfterQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 4e. Query requiring NO secondary item
    console.log(
        "\n4e. Match Local IDs that include 411.1 and have no secondary item:",
    );
    const noSecondaryQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromNodes((nodes) =>
            nodes.withNode(gmod.getNode("411.1"), true).build(),
        )
        .withoutSecondaryItem()
        .build();

    for (const lid of parsedLocalIds) {
        if (await noSecondaryQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // 4f. Query with any or no secondary item
    console.log(
        "\n4f. Match Local IDs that include 411.1 and have any or no secondary item:",
    );
    const anyOrNoSecondaryQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromNodes((nodes) =>
            nodes.withNode(gmod.getNode("411.1"), true).build(),
        )
        .withAnySecondaryItem()
        .build();

    for (const lid of parsedLocalIds) {
        if (await anyOrNoSecondaryQuery.match(lid)) {
            console.log(`   ✓ Matched: ${lid}`);
        }
    }

    // =========================================================================
    // 5. Practical Use Case: Filtering Sensor Data
    // =========================================================================
    console.log("\n\n5. Practical Use Case: Filtering Sensor Data");
    console.log("-".repeat(50));

    console.log(
        "\n   Scenario: Find all temperature sensors on the main engine",
    );

    const mePath = GmodPath.parse("411.1/C101", locations, gmod);
    const engineTempQuery = LocalIdQueryBuilder.empty()
        .withPrimaryItemFromPath(mePath, (path) =>
            path
                .withAnyNodeAfter((nodes) => nodes.get("C101")!)
                .withoutLocations()
                .build(),
        )
        .withTags((tags) =>
            tags.withTag(CodebookName.Quantity, "temperature").build(),
        )
        .build();

    console.log(
        "   Query: Primary item contains 411.1/C101 (any location) + qty=temperature",
    );
    console.log("   Results:");
    for (const lid of parsedLocalIds) {
        if (await engineTempQuery.match(lid)) {
            console.log(`   ✓ ${lid}`);
        }
    }

    console.log(
        "\n   Scenario: Find all pressure sensors with fuel or lubricating oil",
    );
    console.log("   Note: OR conditions require multiple queries");

    const fuelOilQuery = LocalIdQueryBuilder.empty()
        .withTags((tags) =>
            tags
                .withTag(CodebookName.Quantity, "pressure")
                .withTag(CodebookName.Content, "fuel.oil")
                .build(),
        )
        .build();

    const lubeOilQuery = LocalIdQueryBuilder.empty()
        .withTags((tags) =>
            tags
                .withTag(CodebookName.Quantity, "pressure")
                .withTag(CodebookName.Content, "lubricating.oil")
                .build(),
        )
        .build();

    console.log(
        "   Query: qty=pressure AND (cnt=fuel.oil OR cnt=lubricating.oil)",
    );
    console.log("   Results:");
    for (const lid of parsedLocalIds) {
        if (
            (await fuelOilQuery.match(lid)) ||
            (await lubeOilQuery.match(lid))
        ) {
            console.log(`   ✓ ${lid}`);
        }
    }

    console.log("\n=== Query Operations Complete ===");
}

main().catch(console.error);
