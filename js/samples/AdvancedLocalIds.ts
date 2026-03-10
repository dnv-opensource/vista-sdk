/*
    Advanced Local ID Operations - Vista SDK JS/TS

    This example demonstrates advanced Local ID operations:
    - Building complex Local IDs with multiple components
    - Error handling during parsing
    - Working with custom tags
    - Local ID validation and inspection
*/

import {
    CodebookName,
    LocalId,
    LocalIdBuilder,
    LocalIdParsingErrorBuilder,
    ParsingState,
    VIS,
    VisVersion,
    VisVersionExtension,
} from "dnv-vista-sdk";

async function main() {
    console.log("=== Advanced Local ID Operations ===\n");

    const version = VisVersion.v3_4a;
    const vis = VIS.instance;
    const { gmod, codebooks, locations } = await vis.getVIS(version);

    // 1. Building complex Local IDs
    console.log("1. Building Complex Local IDs...");

    const primaryPath = gmod.parsePath("411.1/C101.31-2", locations);
    const secondaryPath = gmod.parsePath("411.1/C101.63/S206", locations);

    const complexLocalId = LocalIdBuilder.create(version)
        .withPrimaryItem(primaryPath)
        .withSecondaryItem(secondaryPath)
        .withMetadataTag(
            codebooks.createTag(CodebookName.Quantity, "temperature"),
        )
        .withMetadataTag(
            codebooks.createTag(CodebookName.Content, "exhaust.gas"),
        )
        .withMetadataTag(codebooks.createTag(CodebookName.State, "high"))
        .withMetadataTag(codebooks.createTag(CodebookName.Position, "inlet"))
        .build();

    console.log(`   Complex Local ID: ${complexLocalId}`);
    console.log(
        `   Has secondary item: ${complexLocalId.secondaryItem !== undefined}`,
    );
    console.log(
        `   Number of metadata tags: ${complexLocalId.metadataTags.length}`,
    );

    // 2. Working with custom tags
    console.log("\n2. Working with Custom Tags...");

    const customQuantity = codebooks.tryCreateTag(
        CodebookName.Quantity,
        "custom_temperature",
    );
    const customPosition = codebooks.tryCreateTag(
        CodebookName.Position,
        "custom_location",
    );

    if (customQuantity && customPosition) {
        const customLocalId = LocalIdBuilder.create(version)
            .withPrimaryItem(primaryPath)
            .withMetadataTag(customQuantity)
            .withMetadataTag(customPosition)
            .build();

        console.log(`   Custom Local ID: ${customLocalId}`);
        console.log(
            `   Quantity tag is custom: ${customLocalId.builder.quantity?.isCustom}`,
        );
        console.log(
            `   Position tag is custom: ${customLocalId.builder.position?.isCustom}`,
        );
    }

    // 3. Error handling and validation
    console.log("\n3. Error Handling and Validation...");

    const invalidLocalIds = [
        "/dnv-v2/vis-3-4a/invalid-path/meta/qty-temperature",
        "/invalid-naming-rule/vis-3-4a/411.1/meta/qty-temperature",
        "/dnv-v2/vis-3-4a/411.1/meta/qty-invalid_quantity",
    ];

    for (const invalidId of invalidLocalIds) {
        console.log(`\n   Testing invalid Local ID: ${invalidId}`);
        const errorBuilder = new LocalIdParsingErrorBuilder();
        const result = LocalIdBuilder.tryParse(
            invalidId,
            gmod,
            codebooks,
            locations,
            errorBuilder,
        );

        if (!result) {
            console.log("     ✗ Parsing failed as expected");
            if (errorBuilder.hasError) {
                console.log("     Errors found:");
                for (const e of errorBuilder.errors) {
                    console.log(
                        `       - ${ParsingState[e.type]}: ${e.message}`,
                    );
                }
            }
        } else {
            console.log(`     ⚠ Unexpectedly parsed successfully: ${result}`);
        }
    }

    // 4. Local ID inspection and analysis
    console.log("\n4. Local ID Inspection and Analysis...");

    const validLocalIds = [
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
    ];

    for (const localIdStr of validLocalIds) {
        console.log(`\n   Analyzing: ${localIdStr}`);

        try {
            const localId = await LocalId.parseAsync(localIdStr);
            console.log("     ✓ Successfully parsed");
            console.log(
                `     ✓ VIS Version: ${VisVersionExtension.toVersionString(localId.visVersion)}`,
            );
            console.log(`     ✓ Has custom tags: ${localId.hasCustomTag}`);
            console.log(`     ✓ Primary item: ${localId.primaryItem}`);

            if (localId.secondaryItem) {
                console.log(`     ✓ Secondary item: ${localId.secondaryItem}`);
            }

            console.log("     ✓ Metadata tags:");
            for (const tag of localId.metadataTags) {
                console.log(
                    `       - ${CodebookName[tag.name]}: ${tag.value} (custom: ${tag.isCustom})`,
                );
            }
        } catch {
            console.log("     ✗ Failed to parse");
        }
    }

    // 5. Builder pattern variations
    console.log("\n5. Builder Pattern Variations...");

    let builder = LocalIdBuilder.create(version);

    // Try adding components safely
    const pathForBuilder = gmod.tryParsePath("411.1/C101.31", locations);
    if (pathForBuilder) {
        builder = builder.withPrimaryItem(pathForBuilder);
    }

    // Try with valid tags
    const tempTag = codebooks.tryCreateTag(
        CodebookName.Quantity,
        "temperature",
    );
    if (tempTag) {
        builder = builder.withMetadataTag(tempTag);
    }

    const exhaustTag = codebooks.tryCreateTag(
        CodebookName.Content,
        "exhaust.gas",
    );
    if (exhaustTag) {
        builder = builder.withMetadataTag(exhaustTag);
    }

    try {
        const safeLocalId = builder.build();
        console.log(`   Safe building result: ${safeLocalId}`);
    } catch (e: any) {
        console.log(`   Safe building failed: ${e.message}`);
    }

    // 6. Verbose mode demonstration
    console.log("\n6. Verbose Mode...");

    const verboseLocalId = LocalIdBuilder.create(version)
        .withVerboseMode(true)
        .withPrimaryItem(primaryPath)
        .withMetadataTag(
            codebooks.createTag(CodebookName.Quantity, "temperature"),
        )
        .build();

    const regularLocalId = LocalIdBuilder.create(version)
        .withVerboseMode(false)
        .withPrimaryItem(primaryPath)
        .withMetadataTag(
            codebooks.createTag(CodebookName.Quantity, "temperature"),
        )
        .build();

    console.log(`   Verbose mode: ${verboseLocalId}`);
    console.log(`   Regular mode: ${regularLocalId}`);

    console.log("\n=== Advanced operations completed! ===");
}

main().catch(console.error);
