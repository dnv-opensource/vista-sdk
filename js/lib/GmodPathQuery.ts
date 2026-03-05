import { GmodNode } from "./GmodNode";
import { GmodPath } from "./GmodPath";
import { Location } from "./Location";
import { VIS } from "./VIS";
import { VisVersionExtension } from "./VisVersion";

// ---------------------------------------------------------------------------
// NodeItem – internal filter descriptor
// ---------------------------------------------------------------------------

class NodeItem {
    public node: GmodNode;
    public locations: Set<string>;
    public matchAllLocations: boolean;
    public ignoreInMatching: boolean;

    constructor(node: GmodNode, locations: Set<string>) {
        this.node = node;
        this.locations = locations;
        this.matchAllLocations = false;
        this.ignoreInMatching = false;
    }
}

// ---------------------------------------------------------------------------
// GmodPathQuery – immutable query object
// ---------------------------------------------------------------------------

export class GmodPathQuery {
    /** @internal */
    public readonly builder: GmodPathQueryBuilder;

    /** @internal */
    constructor(builder: GmodPathQueryBuilder) {
        this.builder = builder;
    }

    /**
     * Test whether the given GmodPath matches this query.
     * Async because version‐conversion requires fetching GMOD data.
     */
    public match(other: GmodPath | undefined): Promise<boolean> {
        return this.builder.match(other);
    }
}

// ---------------------------------------------------------------------------
// GmodPathQueryBuilder – abstract base with Path / Nodes sub‐classes
// ---------------------------------------------------------------------------

export abstract class GmodPathQueryBuilder {
    /** @internal */
    protected readonly _filter: Map<string, NodeItem>;

    protected constructor() {
        this._filter = new Map();
    }

    // -- Factory methods ---------------------------------------------------

    public static empty(): NodesBuilder {
        return new NodesBuilder();
    }

    public static from(path: GmodPath): PathBuilder {
        return new PathBuilder(path);
    }

    public build(): GmodPathQuery {
        return new GmodPathQuery(this);
    }

    // -- Version helpers (async) -------------------------------------------

    private static async ensurePathVersion(path: GmodPath): Promise<GmodPath> {
        const pathVersion = path.node.visVersion;
        if (VisVersionExtension.lessThan(pathVersion, VIS.latestVisVersion)) {
            const converted = await VIS.instance.convertPath(
                pathVersion,
                path,
                VIS.latestVisVersion,
            );
            if (!converted)
                throw new Error("Failed to convert path to latest version");
            return converted;
        }
        return path;
    }

    private static async ensureNodeVersion(node: GmodNode): Promise<GmodNode> {
        if (
            VisVersionExtension.lessThan(node.visVersion, VIS.latestVisVersion)
        ) {
            const converted = await VIS.instance.convertNode(
                node.visVersion,
                node,
                VIS.latestVisVersion,
            );
            if (!converted)
                throw new Error("Failed to convert node to latest version");
            return converted;
        }
        return node;
    }

    // -- Match logic -------------------------------------------------------

    /** @internal */
    public async match(other: GmodPath | undefined): Promise<boolean> {
        if (!other) return false;

        const target = await GmodPathQueryBuilder.ensurePathVersion(other);

        // Build a map of code -> locations[] from the target path
        const targetNodes = new Map<string, string[]>();
        const nodes = target.getFullPath();
        for (const node of nodes) {
            let locations = targetNodes.get(node.code);
            if (!locations) {
                locations = [];
                targetNodes.set(node.code, locations);
            }
            if (node.location) {
                locations.push(node.location.value);
            }
        }

        // Check each filter item against the target
        for (const [, item] of this._filter) {
            const node = await GmodPathQueryBuilder.ensureNodeVersion(
                item.node,
            );

            // Skip nodes marked as ignorable
            if (item.ignoreInMatching) continue;

            const potentialLocations = targetNodes.get(node.code);
            if (!potentialLocations) return false;
            if (item.matchAllLocations) continue;
            if (item.locations.size > 0) {
                if (potentialLocations.length === 0) return false;
                if (!potentialLocations.some((l) => item.locations.has(l)))
                    return false;
            } else {
                if (potentialLocations.length > 0) return false;
            }
        }

        return true;
    }
}

// ---------------------------------------------------------------------------
// PathBuilder
// ---------------------------------------------------------------------------

export class PathBuilder extends GmodPathQueryBuilder {
    public readonly gmodPath: GmodPath;
    private readonly _setNodes: Map<string, GmodNode>;
    private readonly _nodes: Map<string, GmodNode>;

    /** @internal */
    constructor(path: GmodPath) {
        super();
        this.gmodPath = path;

        this._setNodes = new Map();
        for (const set of path.individualizableSets) {
            const setNodes = set.nodes;
            const setNode = setNodes[setNodes.length - 1];
            this._setNodes.set(setNode.code, setNode);

            const locations = new Set<string>();
            if (set.location) {
                locations.add(set.location.value);
            }
            this._filter.set(setNode.code, new NodeItem(setNode, locations));
        }

        this._nodes = new Map();
        for (const n of path.getFullPath()) {
            this._nodes.set(n.code, n);
        }
    }

    // -- withNode (matchAllLocations) --------------------------------------

    public withNode(
        select: (nodes: Map<string, GmodNode>) => GmodNode,
        matchAllLocations: boolean,
    ): PathBuilder;
    public withNode(
        select: (nodes: Map<string, GmodNode>) => GmodNode,
        ...locations: Location[]
    ): PathBuilder;
    public withNode(
        select: (nodes: Map<string, GmodNode>) => GmodNode,
        ...args: unknown[]
    ): PathBuilder {
        const node = select(this._setNodes);
        const item = this._filter.get(node.code);
        if (!item)
            throw new Error(
                "Expected to find a filter on the node in the path",
            );

        if (args.length === 1 && typeof args[0] === "boolean") {
            // matchAllLocations overload
            item.locations = new Set();
            item.matchAllLocations = args[0] as boolean;
        } else {
            // locations overload (may be empty array)
            const locs = args as Location[];
            item.locations = new Set(locs.map((l) => l.value));
        }
        return this;
    }

    // -- withAnyNodeBefore -------------------------------------------------

    public withAnyNodeBefore(
        select: (nodes: Map<string, GmodNode>) => GmodNode,
    ): PathBuilder {
        const node = select(this._nodes);
        const fullPath = this.gmodPath.getFullPath();
        if (!fullPath.some((n) => n.code === node.code))
            throw new Error(`Node ${node.code} is not in the path`);

        for (const pathNode of fullPath) {
            if (pathNode.code === node.code) break;
            const item = this._filter.get(pathNode.code);
            if (!item) continue;
            item.ignoreInMatching = true;
        }

        // Ensure the target node is in the filter so it's checked during matching
        if (!this._filter.has(node.code)) {
            const ni = new NodeItem(node, new Set());
            ni.matchAllLocations = true;
            this._filter.set(node.code, ni);
        }

        return this;
    }

    // -- withAnyNodeAfter --------------------------------------------------

    public withAnyNodeAfter(
        select: (nodes: Map<string, GmodNode>) => GmodNode,
    ): PathBuilder {
        const node = select(this._nodes);
        const fullPath = this.gmodPath.getFullPath();
        if (!fullPath.some((n) => n.code === node.code))
            throw new Error(`Node ${node.code} is not in the path`);

        let found = false;
        for (const pathNode of fullPath) {
            if (pathNode.code === node.code) {
                found = true;
                continue;
            }
            if (found) {
                const item = this._filter.get(pathNode.code);
                if (item) item.ignoreInMatching = true;
            }
        }

        // Ensure the target node is in the filter so it's checked during matching
        if (!this._filter.has(node.code)) {
            const ni = new NodeItem(node, new Set());
            ni.matchAllLocations = true;
            this._filter.set(node.code, ni);
        }

        return this;
    }

    // -- withoutLocations --------------------------------------------------

    public withoutLocations(): PathBuilder {
        for (const item of this._filter.values()) {
            item.locations = new Set();
            item.matchAllLocations = true;
        }
        return this;
    }
}

// ---------------------------------------------------------------------------
// NodesBuilder
// ---------------------------------------------------------------------------

export class NodesBuilder extends GmodPathQueryBuilder {
    /** @internal */
    constructor() {
        super();
    }

    public withNode(node: GmodNode, matchAllLocations: boolean): NodesBuilder;
    public withNode(node: GmodNode, ...locations: Location[]): NodesBuilder;
    public withNode(node: GmodNode, ...args: unknown[]): NodesBuilder {
        if (args.length === 1 && typeof args[0] === "boolean") {
            const matchAll = args[0] as boolean;
            const existing = this._filter.get(node.code);
            if (existing) {
                existing.locations = new Set();
                existing.matchAllLocations = matchAll;
            } else {
                const ni = new NodeItem(node, new Set());
                ni.matchAllLocations = matchAll;
                this._filter.set(node.code, ni);
            }
        } else {
            const locs = args as Location[];
            const newLocations = new Set(locs.map((l) => l.value));
            const existing = this._filter.get(node.code);
            if (existing) {
                existing.locations = newLocations;
            } else {
                this._filter.set(node.code, new NodeItem(node, newLocations));
            }
        }

        return this;
    }
}
