/*
    Asset Model Example: Building a Digital Twin Structure from GmodPaths.

    This example demonstrates one way to build an asset model (digital twin structure)
    from a list of GmodPaths. This is the foundation for:
    - Defining what equipment exists on your vessel
    - Creating hierarchical visualizations
    - Generating LocalIds for sensor data

    The key insight is that a GmodPath like "411.1-1/C101/C101.31-1" implicitly
    contains the entire hierarchy. GMOD validates that the path is structurally
    valid, and we can build a tree by combining multiple paths.
*/

import { GmodNode, GmodPath, LocalId, VIS, VisVersion } from "dnv-vista-sdk";

// =========================================================================
// Asset Model Classes
// =========================================================================

class AssetNode {
    public readonly node: GmodNode;
    public readonly path: GmodPath | undefined;
    public readonly depth: number;
    public readonly children: AssetNode[] = [];
    public parent: AssetNode | undefined;
    private readonly _displayName: string | undefined;

    constructor(node: GmodNode, path: GmodPath | undefined, depth: number) {
        this.node = node;
        this.path = path;
        this.depth = depth;
        if (path) {
            const names = path.getCommonNames();
            this._displayName =
                names.length > 0 ? names[names.length - 1].name : undefined;
        }
    }

    addChild(child: AssetNode): void {
        child.parent = this;
        this.children.push(child);
    }

    get displayName(): string {
        return (
            this._displayName ??
            this.node.metadata.commonName ??
            this.node.metadata.name
        );
    }

    toDict(): Record<string, unknown> {
        return {
            path: this.path?.toString(),
            code: this.node.code,
            name: this.node.metadata.name,
            commonName: this.node.metadata.commonName,
            displayName: this.displayName,
            category: this.node.metadata.category,
            type: this.node.metadata.type,
            location: this.node.location?.toString(),
            children: this.children
                .filter((c) => c.path !== undefined)
                .sort((a, b) =>
                    (a.path?.toString() ?? "").localeCompare(
                        b.path?.toString() ?? "",
                    ),
                )
                .map((c) => c.toDict()),
        };
    }

    printTree(indent: number = 0): void {
        const prefix = " ".repeat(indent * 2);
        const connector = indent > 0 ? "├─" : "";
        const location = this.node.location ? ` [${this.node.location}]` : "";
        console.log(
            `${prefix}${connector} ${this.node.code}${location}: ${this.displayName}`,
        );
        const sortedChildren = this.children
            .filter((c) => c.path !== undefined)
            .sort((a, b) =>
                (a.path?.toString() ?? "").localeCompare(
                    b.path?.toString() ?? "",
                ),
            );
        for (const child of sortedChildren) {
            child.printTree(indent + 1);
        }
    }
}

class AssetModel {
    public readonly visVersion: VisVersion;
    public root: AssetNode | undefined;
    private readonly _nodeMap = new Map<string, AssetNode>();
    private readonly _nodesByCode = new Map<string, AssetNode[]>();

    private constructor(visVersion: VisVersion) {
        this.visVersion = visVersion;
    }

    static fromPaths(visVersion: VisVersion, paths: GmodPath[]): AssetModel {
        const model = new AssetModel(visVersion);
        for (const path of paths) {
            model.addPath(path);
        }
        return model;
    }

    static async fromPathStrings(
        visVersion: VisVersion,
        pathStrings: string[],
    ): Promise<AssetModel> {
        const vis = VIS.instance;
        const { gmod, locations } = await vis.getVIS(visVersion);

        const paths: GmodPath[] = [];
        for (const pathStr of pathStrings) {
            const path = gmod.tryParsePath(pathStr, locations);
            if (path) {
                paths.push(path);
            } else {
                console.log(`  Warning: Could not parse path '${pathStr}'`);
            }
        }
        return AssetModel.fromPaths(visVersion, paths);
    }

    private addPath(path: GmodPath): void {
        const fullPath = path.getFullPath();

        for (let i = 0; i < fullPath.length; i++) {
            const node = fullPath[i];
            const depth = i;

            // Build the full path ID up to this point
            const nodeId = fullPath
                .slice(0, i + 1)
                .map((n) => n.toString())
                .join("/");

            // Skip if already added
            if (this._nodeMap.has(nodeId)) continue;

            // Build the GmodPath for this node
            const parents = fullPath.slice(0, i);
            const nodePath =
                depth === 0 ? undefined : new GmodPath(parents, node, true);

            // Create the asset node
            const assetNode = new AssetNode(node, nodePath, depth);

            // Add to map
            this._nodeMap.set(nodeId, assetNode);

            // Add to nodes_by_code index
            const code = node.code;
            if (!this._nodesByCode.has(code)) {
                this._nodesByCode.set(code, []);
            }
            this._nodesByCode.get(code)!.push(assetNode);

            // Handle root node
            if (depth === 0) {
                if (!this.root) this.root = assetNode;
                continue;
            }

            // Find parent and link
            const parentId = fullPath
                .slice(0, i)
                .map((n) => n.toString())
                .join("/");
            const parentNode = this._nodeMap.get(parentId);
            if (parentNode) {
                parentNode.addChild(assetNode);
            }
        }
    }

    getNode(nodeId: string): AssetNode | undefined {
        return this._nodeMap.get(nodeId);
    }

    get nodesByCode(): Map<string, AssetNode[]> {
        return this._nodesByCode;
    }

    getNodesByCode(code: string): AssetNode[] {
        return this._nodesByCode.get(code) ?? [];
    }

    get nodeCount(): number {
        return this._nodeMap.size;
    }

    get maxDepth(): number {
        let max = 0;
        for (const node of this._nodeMap.values()) {
            if (node.depth > max) max = node.depth;
        }
        return max;
    }

    toDict(): Record<string, unknown> {
        return this.root?.toDict() ?? {};
    }

    toJson(): string {
        return JSON.stringify(this.toDict(), null, 2);
    }

    printTree(): void {
        this.root?.printTree();
    }
}

// =========================================================================
// Example 1: Dual-engine Vessel Asset Model
// =========================================================================
async function exampleDualEngineVessel(): Promise<void> {
    console.log("\n" + "=".repeat(70));
    console.log("Example 1: Dual-engine Vessel Asset Model");
    console.log("=".repeat(70));

    const visVersion = VisVersion.v3_4a;

    // Define the equipment that exists on this vessel
    const assetPaths = [
        // === Port Main Engine (411.1-P) with 6 cylinders ===
        "411.1-P/C101/C101.31-1", // Port engine, cylinder 1
        "411.1-P/C101/C101.31-2", // Port engine, cylinder 2
        "411.1-P/C101/C101.31-3", // Port engine, cylinder 3
        "411.1-P/C101/C101.31-4", // Port engine, cylinder 4
        "411.1-P/C101/C101.31-5", // Port engine, cylinder 5
        "411.1-P/C101/C101.31-6", // Port engine, cylinder 6
        "411.1-P/C101/C101.63/S206", // Port engine, cooling system
        // === Starboard Main Engine (411.1-S) with 6 cylinders ===
        "411.1-S/C101/C101.31-1", // Starboard engine, cylinder 1
        "411.1-S/C101/C101.31-2", // Starboard engine, cylinder 2
        "411.1-S/C101/C101.31-3", // Starboard engine, cylinder 3
        "411.1-S/C101/C101.31-4", // Starboard engine, cylinder 4
        "411.1-S/C101/C101.31-5", // Starboard engine, cylinder 5
        "411.1-S/C101/C101.31-6", // Starboard engine, cylinder 6
        "411.1-S/C101/C101.63/S206", // Starboard engine, cooling system
        // === Generator Sets ===
        "511.11-1/C101", // Generator 1, diesel engine
        "511.11-2/C101", // Generator 2, diesel engine
        // === Fuel System ===
        "621.21/S90", // Fuel oil transfer system, piping
    ];

    console.log(`\n  Defining ${assetPaths.length} equipment paths...`);
    console.log("  (Each path implicitly includes all parent nodes)");

    // Build the asset model
    const model = await AssetModel.fromPathStrings(visVersion, assetPaths);

    console.log(`\n  Built model with ${model.nodeCount} total nodes`);
    console.log(`  Maximum depth: ${model.maxDepth}`);

    // Demonstrate nodes_by_code lookup - "show me all engines"
    console.log("\n  === Lookup: All Engines (C101) ===");
    const engines = model.getNodesByCode("C101");
    console.log(
        `  Found ${engines.length} instances of C101 (Internal combustion engine):`,
    );
    for (const engine of engines) {
        console.log(`    - Path: ${engine.path}`);
    }

    // Demonstrate looking up all cylinders
    console.log("\n  === Lookup: All Cylinders (C101.31) ===");
    const cylinders = model.getNodesByCode("C101.31");
    console.log(`  Found ${cylinders.length} instances of C101.31 (Cylinder):`);
    for (const cyl of cylinders.slice(0, 6)) {
        console.log(`    - Path: ${cyl.path}`);
    }
    if (cylinders.length > 6) {
        console.log(`    ... and ${cylinders.length - 6} more`);
    }

    // Show available codes in the model
    console.log("\n  === All available codes in model ===");
    const codes = [...model.nodesByCode.keys()].sort();
    console.log(`  ${codes.length} unique codes: [${codes.join(", ")}]`);

    console.log("\n  Asset Model Tree:");
    console.log();
    model.printTree();
}

// =========================================================================
// Example 2: Build Model from LocalIds (Runtime Flow)
// =========================================================================
async function exampleFromLocalIds(): Promise<void> {
    console.log("\n" + "=".repeat(70));
    console.log("Example 2: Build Model from LocalIds (Runtime Flow)");
    console.log("=".repeat(70));

    const visVersion = VisVersion.v3_4a;

    // Simulate LocalIds coming from a DataChannelList or data source
    const localIdStrings = [
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-1/C101.31-3/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-2/C101.31-1/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1-2/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/511.11-1/C101/meta/qty-revolution",
    ];

    console.log(
        `\n  Received ${localIdStrings.length} LocalIds from data source`,
    );

    // Parse LocalIds and extract GmodPaths
    const paths: GmodPath[] = [];
    for (const lidStr of localIdStrings) {
        const localId = await LocalId.parseAsync(lidStr);
        if (localId.primaryItem) {
            paths.push(localId.primaryItem);
        }
        if (localId.secondaryItem) {
            paths.push(localId.secondaryItem);
        }
    }

    console.log(`  Extracted ${paths.length} GmodPaths from LocalIds`);

    // Build model from extracted paths
    const model = AssetModel.fromPaths(visVersion, paths);

    console.log(`\n  Built model with ${model.nodeCount} nodes`);
    console.log(`  Maximum depth: ${model.maxDepth}`);

    console.log("\n  Asset Model Tree (derived from LocalIds):");
    console.log();
    model.printTree();
}

// =========================================================================
// Example 3: JSON Export for Visualization
// =========================================================================
async function exampleJsonExport(): Promise<void> {
    console.log("\n" + "=".repeat(70));
    console.log("Example 3: JSON Export for Visualization");
    console.log("=".repeat(70));

    const visVersion = VisVersion.v3_4a;

    // Small example for readable JSON
    const assetPaths = ["411.1/C101/C101.31-1", "411.1/C101/C101.31-2"];

    const model = await AssetModel.fromPathStrings(visVersion, assetPaths);

    console.log("\n  JSON output (for D3.js, etc.):");
    console.log();
    console.log(model.toJson());
}

// =========================================================================
// Main
// =========================================================================
async function main() {
    console.log("=".repeat(70));
    console.log(
        "Asset Model Examples: Building Digital Twin Structures from GmodPaths",
    );
    console.log("=".repeat(70));
    console.log();
    console.log(
        "These examples demonstrate how to define what equipment exists",
    );
    console.log("on your vessel using GmodPaths, then build a tree structure");
    console.log(
        "for visualization and further processing (like LocalId generation).",
    );

    await exampleDualEngineVessel();
    await exampleFromLocalIds();
    await exampleJsonExport();

    console.log("\n" + "=".repeat(70));
    console.log("All examples completed!");
    console.log("=".repeat(70));
}

main().catch(console.error);
