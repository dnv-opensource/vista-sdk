/*
    Basic Usage Example - Vista SDK JS/TS

    This example demonstrates the fundamental operations of the Vista SDK:
    - Initializing VIS
    - Working with GMOD paths
    - Creating Local IDs
    - Using codebooks
*/

import {
    CodebookName,
    GmodPath,
    LocalId,
    LocalIdBuilder,
    MetadataTag,
    VIS,
    VisVersion,
} from "dnv-vista-sdk";

async function main() {
    console.log("=== Vista SDK Basic Usage Example ===\n");

    // 1. Initialize VIS instance
    console.log("1. Initializing VIS...");
    const version = VisVersion.v3_4a;
    console.log(`   Loading data for VIS version: ${version}`);

    const vis = VIS.instance;
    const { gmod, codebooks, locations } = await vis.getVIS(version);

    console.log("   ✓ GMOD loaded");
    console.log("   ✓ Codebooks loaded\n");

    // 2. Work with Gmod node
    console.log("2. Working with Gmod nodes...");

    const engineNode = gmod.getNode("C101");
    console.log(`   Node code: ${engineNode.code}`);
    console.log(`   Common name: ${engineNode.metadata.commonName ?? ""}`);
    console.log(`   Category: ${engineNode.metadata.category}`);
    console.log(`   Is leaf: ${engineNode.isLeafNode}`);

    console.log(`   Children (${engineNode.children.length}):`);
    for (const child of engineNode.children.slice(0, 5)) {
        console.log(`     → ${child.code}: ${child.metadata.commonName}`);
    }
    if (engineNode.children.length > 5) {
        console.log(`     ... and ${engineNode.children.length - 5} more`);
    }
    console.log();

    // 3. Parse GMOD paths
    console.log("3. Working with GMOD paths...");
    const pathStrings = [
        "411.1/C101.31-2",
        "612.21-1/C701.13/S93",
        "1021.1i-6P/H123",
    ];

    const paths: GmodPath[] = [];
    for (const pathStr of pathStrings) {
        const path = gmod.tryParsePath(pathStr, locations);
        if (path) {
            paths.push(path);
            const node = path.node;
            console.log(`   ✓ Parsed: ${pathStr}`);
            console.log(
                `     → Node: ${node} (${node.metadata.commonName ?? ""})`,
            );
            console.log(`     → Depth: ${path.length}`);
        } else {
            console.log(`   ✗ Failed to parse ${pathStr}`);
        }
    }
    console.log();

    // 4. Work with codebooks
    console.log("4. Using codebooks...");

    const quantityBook = codebooks.getCodebook(CodebookName.Quantity);
    const positionBook = codebooks.getCodebook(CodebookName.Position);
    const contentBook = codebooks.getCodebook(CodebookName.Content);

    const tags: Record<string, MetadataTag> = {
        quantity: quantityBook.createTag("temperature"),
        position: positionBook.createTag("centre"),
        content: contentBook.createTag("cooling.water"),
    };

    for (const [tagType, tag] of Object.entries(tags)) {
        console.log(
            `   ✓ Created ${tagType} tag: ${tag.value} (custom: ${tag.isCustom})`,
        );
    }
    console.log();

    // 5. Build Local IDs
    console.log("5. Creating Local IDs...");

    for (let i = 0; i < Math.min(2, paths.length); i++) {
        const path = paths[i];
        try {
            const localId = LocalIdBuilder.create(version)
                .withPrimaryItem(path)
                .withMetadataTag(tags["quantity"])
                .withMetadataTag(tags["content"])
                .withMetadataTag(tags["position"])
                .build();

            console.log(`   ✓ Local ID ${i + 1}: ${localId}`);
            console.log(`     → Primary item: ${localId.primaryItem}`);
        } catch (e: any) {
            console.log(
                `   ✗ Failed to create Local ID for ${path}: ${e.message}`,
            );
        }
    }
    console.log();

    // 6. Demonstrate parsing existing Local IDs
    console.log("6. Parsing existing Local IDs...");

    const sampleLocalIds = [
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
    ];

    for (const localIdStr of sampleLocalIds) {
        try {
            const localId = LocalId.parse(
                localIdStr,
                gmod,
                codebooks,
                locations,
            );
            console.log(`   ✓ Parsed: ${localIdStr}`);
            console.log(`     → Primary: ${localId.primaryItem}`);
            console.log(`     → Metadata tags: ${localId.metadataTags.length}`);
            for (const tag of localId.metadataTags) {
                console.log(`       - ${CodebookName[tag.name]}: ${tag.value}`);
            }
        } catch {
            console.log(`   ✗ Failed to parse ${localIdStr}`);
        }
    }

    console.log("\n=== Example completed successfully! ===");
}

main().catch(console.error);
