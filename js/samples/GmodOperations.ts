/*
    GMOD Operations - Vista SDK JS/TS

    This example demonstrates working with GMOD (Generic Product Model):
    - Parsing and traversing GMOD paths
    - Exploring node hierarchies
    - Working with node metadata
    - Version conversion operations
*/

import {
    GmodNode,
    TraversalHandlerResult,
    VIS,
    VisVersion,
} from "dnv-vista-sdk";

async function main() {
    console.log("=== GMOD Operations Example ===\n");

    const version = VisVersion.v3_4a;
    const vis = VIS.instance;
    const { gmod, locations } = await vis.getVIS(version);

    // 1. Basic GMOD path operations
    console.log("1. Basic GMOD Path Operations...");

    const samplePaths = [
        "411.1",
        "411.1/C101.31-2",
        "612.21-1/C701.13/S93",
        "1021.1i-6P/H123",
    ];

    for (const pathStr of samplePaths) {
        const path = gmod.tryParsePath(pathStr, locations);
        if (path) {
            const node = path.node;
            console.log(`   Path: ${pathStr}`);
            console.log(`     → Code: ${node.code}`);
            console.log(`     → Name: ${node.metadata.name}`);
            console.log(
                `     → Common Name: ${node.metadata.commonName ?? ""}`,
            );
            console.log(`     → Depth: ${path.length}`);
            console.log();
        } else {
            console.log(`   ✗ Failed to parse ${pathStr}`);
        }
    }

    // 2. Exploring node hierarchies
    console.log("2. Exploring Node Hierarchies...");

    const rootPath = gmod.parsePath("411.1", locations);
    const rootNode = rootPath.node;

    console.log(
        `   Root node: ${rootNode.code} - ${rootNode.metadata.commonName ?? ""}`,
    );
    console.log("   Child nodes:");

    const children = rootNode.children.slice(0, 5);
    for (const child of children) {
        console.log(`     - ${child.code}: ${child.metadata.commonName ?? ""}`);
    }

    // 3. Path traversal and analysis
    console.log("\n3. Path Traversal and Analysis...");

    const deepPath = gmod.parsePath("411.1/C101.31-2", locations);

    console.log(`   Analyzing path: ${deepPath}`);
    console.log("   Full path traversal:");
    const commonNames = new Map(
        deepPath.getCommonNames().map((c) => [c.depth, c.name]),
    );
    const fullPath = deepPath.getFullPath();
    for (let depth = 0; depth < fullPath.length; depth++) {
        if (depth === 0) continue; // Skip root
        const node = fullPath[depth];
        const indent = " ".repeat((depth - 1) * 5);
        const commonName =
            commonNames.get(depth) ?? node.metadata.commonName ?? "";
        console.log(`   ${indent}Depth ${depth}: ${node.code} (${commonName})`);
    }

    // 4. Node properties and metadata
    console.log("\n4. Node Properties and Metadata...");

    const sampleNodes = ["411.1", "C101.31", "S206"];

    for (const nodeCode of sampleNodes) {
        const node = gmod.tryGetNode(nodeCode);
        if (node) {
            console.log(`   Node: ${nodeCode}`);
            console.log(`     → Name: ${node.metadata.name}`);
            console.log(
                `     → Common Name: ${node.metadata.commonName ?? ""}`,
            );
            console.log(`     → Is Mappable: ${node.isMappable}`);
            console.log(`     → Has Children: ${node.children.length > 0}`);
            console.log();
        } else {
            console.log(`   ✗ Node ${nodeCode} not found`);
        }
    }

    // 5. Path validation and error handling
    console.log("5. Path Validation and Error Handling...");

    const invalidPaths = [
        "invalid.node",
        "411.1/invalid-child",
        "999.999/does-not-exist",
    ];

    for (const invalidPath of invalidPaths) {
        const result = gmod.tryParsePath(invalidPath, locations);
        if (!result) {
            console.log(`   ✗ Path '${invalidPath}' is invalid (as expected)`);
        } else {
            console.log(
                `   ⚠ Path '${invalidPath}' unexpectedly parsed successfully`,
            );
        }
    }

    // 6. GMOD version conversion
    console.log("\n6. GMOD Version Conversion...");

    const conversionTests: [string, VisVersion, VisVersion][] = [
        ["111.3/H402", VisVersion.v3_7a, VisVersion.v3_8a],
        ["846/G203", VisVersion.v3_7a, VisVersion.v3_8a],
    ];

    for (const [oldPathStr, sourceVersion, targetVersion] of conversionTests) {
        const sourceGmod = await vis.getGmod(sourceVersion);
        const sourceLocations = await vis.getLocations(sourceVersion);

        const oldPath = sourceGmod.tryParsePath(oldPathStr, sourceLocations);
        if (!oldPath) {
            console.log(`   ✗ Failed to parse ${oldPathStr}`);
            continue;
        }

        const newPath = await vis.convertPath(
            sourceVersion,
            oldPath,
            targetVersion,
        );
        if (newPath) {
            console.log(
                `   ✓ Converted: ${oldPathStr} (${sourceVersion}) → ${newPath} (${targetVersion})`,
            );
        } else {
            console.log(`   ✗ Conversion returned undefined for ${oldPathStr}`);
        }
    }

    // 7. GMOD Traversal
    console.log("\n7. GMOD Traversal...");

    // Simple traversal - count all nodes
    let nodeCount = 0;
    const completed = gmod.traverse((parents: GmodNode[], node: GmodNode) => {
        nodeCount++;
        return TraversalHandlerResult.Continue;
    });
    console.log(
        `   Traversal completed: ${completed}, Nodes visited: ${nodeCount}`,
    );

    // Traversal with early stop - find first leaf node
    let firstLeaf: GmodNode | undefined;
    const completed2 = gmod.traverse((parents: GmodNode[], node: GmodNode) => {
        if (node.isLeafNode) {
            firstLeaf = node;
            return TraversalHandlerResult.Stop;
        }
        return TraversalHandlerResult.Continue;
    });
    console.log(
        `   Traversal completed: ${completed2}, First leaf node: ${firstLeaf}`,
    );

    // Traversal from specific node
    const startNode = gmod.getNode("411");
    let childCount = 0;
    const completed3 = gmod.traverse(
        (parents: GmodNode[], node: GmodNode) => {
            childCount++;
            return TraversalHandlerResult.Continue;
        },
        { rootNode: startNode },
    );

    console.log(
        `   Traversal completed: ${completed3}, Child nodes visited from '411': ${childCount}`,
    );

    console.log("\n=== GMOD operations completed! ===");
}

main().catch(console.error);
