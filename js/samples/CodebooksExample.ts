/*
    Codebooks and Metadata Tags - Vista SDK JS/TS

    This example demonstrates working with codebooks and metadata tags:
    - Loading and exploring codebooks
    - Creating standard and custom metadata tags
    - Validating tag values
    - Working with different tag types
*/

import { CodebookName, VIS, VisVersion } from "dnv-vista-sdk";

async function main() {
    console.log("=== Codebooks and Metadata Tags Example ===\n");

    const version = VisVersion.v3_6a;
    const vis = VIS.instance;
    const codebooks = await vis.getCodebooks(version);

    // 1. Exploring available codebooks
    console.log("1. Available Codebooks...");

    const availableCodebooks: CodebookName[] = [
        CodebookName.Quantity,
        CodebookName.Content,
        CodebookName.Calculation,
        CodebookName.State,
        CodebookName.Command,
        CodebookName.Type,
        CodebookName.Position,
        CodebookName.Detail,
    ];

    for (const codebookName of availableCodebooks) {
        const codebook = codebooks.getCodebook(codebookName);
        console.log(`   ${CodebookName[codebookName]}:`);
        console.log(
            `     → Standard values: ${codebook.standardValues.values.size}`,
        );
        console.log(`     → Groups: ${codebook.groups.count}`);
        console.log();
    }

    // 2. Creating standard metadata tags
    console.log("2. Creating Metadata Tags...");

    const tagsToCreate: Record<CodebookName, string[]> = {
        [CodebookName.Quantity]: ["temperature", "pressure", "flow"],
        [CodebookName.Position]: ["centre", "inlet", "outlet"],
        [CodebookName.State]: ["opened", "closed", "high", "low"],
        [CodebookName.Content]: ["cooling.water", "exhaust.gas", "fuel.oil"],
    } as any;

    for (const [codebookNameStr, values] of Object.entries(tagsToCreate)) {
        const codebookName = Number(codebookNameStr) as CodebookName;
        console.log(`   ${CodebookName[codebookName]} tags:`);
        const codebook = codebooks.getCodebook(codebookName);

        for (const value of values as string[]) {
            const tag = codebook.tryCreateTag(value);
            if (tag) {
                console.log(
                    `     ✓ ${value}: ${tag} (custom: ${tag.isCustom})`,
                );
            } else {
                console.log(`     ✗ ${value}: Failed to create tag`);
            }
        }
        console.log();
    }

    // 3. Working with custom tags
    console.log("3. Working with Custom Tags...");

    const customExamples: [CodebookName, string][] = [
        [CodebookName.Quantity, "custom_temperature"],
        [CodebookName.Position, "custom_location"],
        [CodebookName.Content, "special.fluid"],
        [CodebookName.State, "partially_open"],
    ];

    for (const [codebookName, customValue] of customExamples) {
        const codebook = codebooks.getCodebook(codebookName);
        const customTag = codebook.tryCreateTag(customValue);
        if (customTag) {
            console.log(
                `   ✓ ${CodebookName[codebookName]}: ${customTag} (custom: ${customTag.isCustom})`,
            );
        } else {
            console.log(
                `   ✗ ${CodebookName[codebookName]}: Failed to create tag for '${customValue}'`,
            );
        }
    }

    // 4. Tag validation
    console.log("\n4. Tag Validation...");

    const validationTests: [CodebookName, string, boolean][] = [
        [CodebookName.Position, "centre", true],
        [CodebookName.Position, "invalid_position", false],
        [CodebookName.Quantity, "temperature", true],
        [CodebookName.Quantity, "nonexistent_quantity", false],
        [CodebookName.State, "opened", true],
        [CodebookName.State, "maybe_opened", false],
    ];

    for (const [codebookName, testValue, expectedValid] of validationTests) {
        const codebook = codebooks.getCodebook(codebookName);
        const isValid = codebook.hasStandardValue(testValue);
        const status = isValid === expectedValid ? "✓" : "✗";
        console.log(
            `   ${status} ${CodebookName[codebookName]}.${testValue}: valid=${isValid} (expected: ${expectedValid})`,
        );
    }

    // 5. Exploring codebook content
    console.log("\n5. Exploring Codebook Content...");

    const stateCodebook = codebooks.getCodebook(CodebookName.State);
    console.log("   State codebook sample values:");

    const stateValues = [...stateCodebook.standardValues.values].slice(0, 10);
    for (const value of stateValues) {
        console.log(`     - ${value}`);
    }

    console.log("   State codebook sample groups:");
    const groups = [...stateCodebook.groups.values].slice(0, 5);
    for (const group of groups) {
        console.log(`     - ${group}`);
    }

    console.log("\n=== Codebooks example completed! ===");
}

main().catch(console.error);
