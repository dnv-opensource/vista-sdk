import { VIS, VisVersion } from ".";
import { Gmod, PotentialParentScopeTypes } from "./Gmod";
import { GmodNode, isIndividualizable } from "./GmodNode";
import { Locations, Location } from "./Location";
import { TraversalHandlerResult } from "./types/Gmod";
import { PathNode, ParseContext } from "./types/GmodPath";
import { match, isNullOrWhiteSpace } from "./util/util";

export class GmodIndividualizableSet {
    public get location(): Location | undefined {
        return this.getNode(this._nodes[0]).location;
    }

    public set location(value: Location | undefined) {
        for (const i of this._nodes) {
            if (!value) {
                if (i < this._path.parents.length)
                    this._path.parents[i] =
                        this._path.parents[i].withoutLocation();
                else this._path.node = this._path.node.withoutLocation();
            } else {
                if (i < this._path.parents.length)
                    this._path.parents[i] =
                        this._path.parents[i].withLocation(value);
                else this._path.node = this._path.node.withLocation(value);
            }
        }
    }

    public get nodes(): GmodNode[] {
        return this._nodes.map((i) => this.getNode(i));
    }

    public get nodesIndices(): number[] {
        return this._nodes;
    }

    public get codes(): string[] {
        return this._nodes.map((i) => this.getNode(i).code);
    }

    constructor(private _nodes: number[], private _path: GmodPath) {
        if (_nodes.length === 0)
            throw new Error("GmodIndividualizableSet cant be empty");
        if (
            _nodes.some(
                (i) =>
                    !isIndividualizable(
                        this.getNode(i),
                        i === _path.parents.length,
                        _nodes.length > 1
                    )
            )
        )
            throw new Error(
                "GmodIndividualizableSet nodes must be individualizable"
            );
        if (
            new Set<string | undefined>(
                _nodes.map((i) => this.getNode(i).location?.value)
            ).size !== 1
        )
            throw new Error(
                "GmodIndividualizableSet must have a common location"
            );
        if (
            !_nodes.some(
                (i) =>
                    this.getNode(i).equals(_path.node) ||
                    this.getNode(i).isLeafNode
            )
        )
            throw new Error(
                "GmodIndividualizableSet has no nodes that are part of short path"
            );

        this._path = _path.clone();
    }

    private getNode(i: number): GmodNode {
        return i < this._path.parents.length
            ? this._path.parents[i]
            : this._path.node;
    }

    public build(): GmodPath {
        const path = this._path;
        if (!path) throw new Error("Tried to build individualizable set twice");
        this._path = undefined!;
        return path;
    }

    public toString(): string {
        return this._nodes
            .filter(
                (i, j) =>
                    this.getNode(i).isLeafNode || j === this._nodes.length - 1
            )
            .map((i) => this.getNode(i).toString())
            .join("/");
    }
}

export class GmodPath {
    public parents: GmodNode[];
    public node: GmodNode;

    public constructor(
        parents: GmodNode[],
        node: GmodNode,
        skipVerify = false
    ) {
        if (!skipVerify) {
            if (parents.length === 0 && node.code !== "VE")
                throw new Error(
                    `Invalid gmod path - no parents, and ${node.code} is not the root of gmod`
                );
            if (parents.length > 0 && parents[0].code !== "VE")
                throw new Error(
                    `Invalid gmod path - first parent should be root of gmod (VE), but was ${parents[0].code}`
                );

            for (let i = 0; i < parents.length; i++) {
                const parent = parents[i];
                const nextIndex = i + 1;
                const child =
                    nextIndex < parents.length ? parents[nextIndex] : node;
                if (!parent.isChild(child))
                    throw new Error(
                        `Invalid gmod path - ${child.code} not child of ${parent.code}`
                    );
            }

            const visit = locationSetsVisitor();
            for (let i = 0; i < parents.length; i++) {
                const n = i < parents.length ? parents[i] : node;
                let _ = visit(n, i, parents, node);
            }
        }

        this.parents = parents;
        this.node = node;
    }

    private static isValid(parents: GmodNode[], node: GmodNode): boolean {
        if (parents.length === 0 && node.code !== "VE") return false;
        if (parents.length > 0 && parents[0].code !== "VE") return false;

        for (let i = 0; i < parents.length; i++) {
            const parent = parents[i];
            const nextIndex = i + 1;
            const child =
                nextIndex < parents.length ? parents[nextIndex] : node;
            if (!parent.isChild(child)) return false;
        }

        return true;
    }

    public static create(parents: GmodNode[], node: GmodNode) {
        return new GmodPath(parents, node, false);
    }

    public get length(): number {
        return this.parents.length + 1;
    }

    public get isMappable(): boolean {
        return this.node.isMappable;
    }

    public getNode(node: GmodNode): GmodNode;
    public getNode(code: string): GmodNode;
    public getNode(depth: number): GmodNode;
    public getNode(node: unknown): GmodNode {
        if (node instanceof GmodNode || typeof node === "string") {
            let code: string;

            if (node instanceof GmodNode) code = node.code;
            else code = node;

            if (code === this.node.code) return this.node;
            const parent = this.parents.find((p) => p.code === code);

            if (!parent) throw new Error("Parent not found");
            return parent;
        } else if (typeof node === "number") {
            const depth = node;
            if (depth < 0 || depth > this.parents.length)
                throw new RangeError("Depth is out of bounds");
            return depth < this.parents.length
                ? this.parents[depth]
                : this.node;
        }
        throw new Error("Unsupported method parameter");
    }

    public withoutLocation(): GmodPath {
        return new GmodPath( // TODO inst sets
            this.parents.map((p) => p.withoutLocation()),
            this.node.withoutLocation()
        );
    }

    public get individualizableSets(): GmodIndividualizableSet[] {
        const result: GmodIndividualizableSet[] = [];

        const visit = locationSetsVisitor();
        for (let i = 0; i < this.length; i++) {
            const node = i < this.parents.length ? this.parents[i] : this.node;
            const set = visit(node, i, this.parents, this.node);
            if (set === null) continue;

            const [start, end, _] = set;
            if (start === end) {
                result.push(new GmodIndividualizableSet([start], this));
                continue;
            }

            const nodes = [];
            for (let j = start; j <= end; j++) nodes.push(j);

            result.push(new GmodIndividualizableSet(nodes, this));
        }

        return result;
    }

    public get isIndividualizable(): boolean {
        const visit = locationSetsVisitor();
        for (let i = 0; i < this.length; i++) {
            const node = i < this.parents.length ? this.parents[i] : this.node;
            const set = visit(node, i, this.parents, this.node);
            if (set === null) continue;

            return true;
        }

        return false;
    }

    public toString(): string {
        const s: string[] = [];

        for (const parent of this.parents) {
            if (!Gmod.isLeafNode(parent.metadata)) continue;
            s.push(parent.toString());
        }

        return s.concat(this.node.toString()).join("/");
    }

    public toFullPathString(): string {
        return this.getFullPath().join("/");
    }

    public toNamesString(): string {
        return this.getCommonNames()
            .map((n) => n.name)
            .join("/");
    }

    public equals(other?: GmodPath): boolean {
        if (!other) return false;
        if (this.parents.length !== other.parents.length) return false;

        for (let i = 0; i < this.parents.length; i++) {
            if (!this.parents[i].equals(other.parents[i])) return false;
        }

        return this.node.equals(other.node);
    }

    public equalsWithoutLocation(other?: GmodPath): boolean {
        if (!other) return false;

        return this.withoutLocation().equals(other.withoutLocation());
    }

    public getFullPath(): GmodNode[] {
        return this.parents.concat(this.node);
    }

    public getNormalAssignmentName(nodeDepth: number): string | undefined {
        const node = this.getNode(nodeDepth);
        const normalAssignmentNames = node.metadata.normalAssignmentNames;
        if (normalAssignmentNames.size === 0) return;

        for (let i = this.length - 1; i >= 0; i--) {
            const child = this.getNode(i);
            const name = normalAssignmentNames.get(child.code);
            if (name) return name;
        }
        return;
    }

    public getAllNormalAssignmentNames = (nodeDepth: number): string[] => {
        const node = this.getNode(nodeDepth);
        return Array.from(node.metadata.normalAssignmentNames.values());
    };

    public getCurrentCommonName(): string {
        return this.getCommonNames().pop()?.name || this.node.metadata.name;
    }

    public getCommonNames(): { depth: number; name: string }[] {
        const commonNames: { depth: number; name: string }[] = [];
        const fullPath = this.getFullPath();

        for (let depth = 0; depth < fullPath.length; depth++) {
            const node = fullPath[depth];
            const isTarget = depth === this.parents.length;

            if (!(node.isLeafNode || isTarget) || !node.isFunctionNode)
                continue;

            let name = node.metadata.commonName ?? node.metadata.name;
            const normalAssignmentNames = node.metadata.normalAssignmentNames;

            if (normalAssignmentNames.size !== 0) {
                const assignment = normalAssignmentNames.get(this.node.code);
                if (assignment) {
                    name = assignment;
                }

                for (let i = this.parents.length - 1; i >= depth; i--) {
                    const assignment = normalAssignmentNames.get(
                        this.parents[i].code
                    );
                    if (!assignment) continue;
                    name = assignment;
                }
            }
            commonNames.push({ depth, name });
        }
        return commonNames;
    }

    public static parse(
        item: string,
        locations: Locations,
        gmod: Gmod
    ): GmodPath {
        const path = this.tryParse(item, locations, gmod);
        if (!path) throw new Error("Couldnt parse GmodPath from item");

        return path;
    }

    public static tryParse(
        item: string | undefined,
        locations: Locations,
        gmod: Gmod
    ): GmodPath | undefined {
        const result = this.parseInternal(item, locations, gmod);
        const out = match<GmodParsePathResult, GmodPath | undefined>(result)
            .ofType(GmodParsePathResult.Ok)
            .do((result) => {
                return result.path;
            })
            .ofType(GmodParsePathResult.Err)
            .do(() => {
                return undefined;
            })
            .unwrap();
        return out;
    }

    private static parseInternal(
        item: string | undefined,
        locations: Locations,
        gmod: Gmod
    ): GmodParsePathResult {
        try {
            if (!item || !gmod || !locations)
                return new GmodParsePathResult.Err("Missing parameters");
            if (isNullOrWhiteSpace(item))
                return new GmodParsePathResult.Err("Item is empty");

            item = item.trim();

            if (item[0] === "/") {
                item.slice(1);
            }

            const parts: PathNode[] = [];

            for (const partStr of item.split("/")) {
                if (partStr.includes("-")) {
                    const split = partStr.split("-");
                    const parsedLocation = locations.tryParse(split[1]);
                    if (!gmod.tryGetNode(split[0]))
                        return new GmodParsePathResult.Err(
                            `Failed to get GmodNode for ${split[0]}`
                        );
                    if (!parsedLocation)
                        return new GmodParsePathResult.Err(
                            `Failed to parse location ${split[1]}`
                        );
                    parts.push({ code: split[0], location: parsedLocation });
                } else {
                    if (!gmod.tryGetNode(partStr))
                        return new GmodParsePathResult.Err(
                            `Failed to get GmodNode for ${partStr}`
                        );
                    parts.push({ code: partStr });
                }
            }

            if (parts.length === 0)
                return new GmodParsePathResult.Err("Failed find any parts");

            if (parts.find((p) => isNullOrWhiteSpace(p.code)))
                return new GmodParsePathResult.Err("Failed find any parts");

            const toFind = parts.shift();
            if (!toFind)
                return new GmodParsePathResult.Err(
                    "Invalid queue operation - Shift empty array"
                );
            const baseNode = gmod.tryGetNode(toFind.code);
            if (!baseNode)
                return new GmodParsePathResult.Err("Failed to find base node");

            const context: ParseContext = {
                parts,
                toFind,
                locations: new Map<string, Location>(),
            };

            gmod.traverse(
                (parents, current, context) => {
                    const toFind = context.toFind;
                    const found = current.code === toFind.code;
                    const leafNode = Gmod.isLeafNode(current.metadata);
                    if (!found && Gmod.isLeafNode(current.metadata))
                        return TraversalHandlerResult.SkipSubtree;
                    if (!found) return TraversalHandlerResult.Continue;

                    if (toFind.location !== undefined) {
                        context.locations.set(toFind.code, toFind.location);
                    }

                    if (context.parts.length > 0) {
                        const newToFind = context.parts.shift();
                        if (newToFind) {
                            context.toFind = newToFind;
                            return TraversalHandlerResult.Continue;
                        }
                    }

                    const pathParents: GmodNode[] = [];

                    for (const parent of parents) {
                        const location = context.locations.get(parent.code);
                        if (location) {
                            pathParents.push(parent.withLocation(location));
                        } else {
                            pathParents.push(parent);
                        }
                    }

                    let endNode = toFind.location
                        ? current.withLocation(toFind.location)
                        : current;

                    const firstParentHasSingleParent =
                        pathParents.length > 0 &&
                        pathParents[0].parents.length === 1;
                    const currentNodeHasSingleParent =
                        endNode.parents.length === 1;

                    let startNode = firstParentHasSingleParent
                        ? pathParents[0].parents[0]
                        : currentNodeHasSingleParent
                        ? endNode.parents[0]
                        : undefined;

                    // Stop if there is no startNode or the parent doesnt have a direct path to root.
                    if (!startNode || startNode.parents.length > 1)
                        return TraversalHandlerResult.Stop;

                    while (startNode.parents.length === 1) {
                        pathParents.unshift(startNode);
                        startNode = startNode.parents[0];
                        if (startNode.parents.length > 1)
                            return TraversalHandlerResult.Stop;
                    }

                    pathParents.unshift(gmod.rootNode);

                    const visit = locationSetsVisitor();
                    for (let i = 0; i < pathParents.length + 1; i++) {
                        var n =
                            i < pathParents.length ? pathParents[i] : endNode;
                        const set = visit(n, i, pathParents, endNode);
                        if (set === null) {
                            if (!!n.location)
                                return TraversalHandlerResult.Stop;
                            continue;
                        }

                        const [start, end, location] = set;
                        if (start === end) continue;

                        for (let j = start; j <= end; j++) {
                            if (j < pathParents.length) {
                                if (!!location)
                                    pathParents[j] =
                                        pathParents[j].withLocation(location);
                                else
                                    pathParents[j] =
                                        pathParents[j].withoutLocation();
                            } else {
                                if (!!location)
                                    endNode = endNode.withLocation(location);
                                else endNode = endNode.withoutLocation();
                            }
                        }
                    }

                    context.path = new GmodPath(pathParents, endNode);
                    return TraversalHandlerResult.Stop;
                },
                { state: context, rootNode: baseNode }
            );

            if (!context.path) {
                return new GmodParsePathResult.Err(
                    "Failed to find path after travesal"
                );
            }

            return new GmodParsePathResult.Ok(context.path);
        } catch {
            return new GmodParsePathResult.Err(
                "Unknown error occured during parsing"
            );
        }
    }

    public static parseFromFullPath(
        item: string,
        gmod: Gmod,
        locations: Locations
    ): GmodPath {
        const path = this.tryParseFromFullPath(item, gmod, locations);

        if (!path) {
            throw new Error("Couldn't parse path from full path");
        }

        return path;
    }

    public static async parseAsync(
        item: string,
        visVersion: VisVersion
    ): Promise<GmodPath> {
        const path = await this.tryParseAsync(item, visVersion);

        if (!path) {
            throw new Error("Couldn't parse path from full path");
        }

        return path;
    }

    public static async parseAsyncFromFullPath(
        item: string,
        visVersion: VisVersion
    ): Promise<GmodPath> {
        const path = await this.tryParseFromFullPathAsync(item, visVersion);

        if (!path) {
            throw new Error("Couldn't parse path from full path");
        }

        return path;
    }
    public static tryParseFromFullPath(
        item: string,
        gmod: Gmod,
        locations: Locations
    ) {
        const result = this.parseFromFullPathInternal(item, gmod, locations);
        const out = match<GmodParsePathResult, GmodPath | undefined>(result)
            .ofType(GmodParsePathResult.Ok)
            .do((result) => {
                return result.path;
            })
            .ofType(GmodParsePathResult.Err)
            .do(() => {
                return undefined;
            })
            .unwrap();
        return out;
    }

    private static parseFromFullPathInternal(
        item: string,
        gmod: Gmod,
        locations: Locations
    ): GmodParsePathResult {
        if (isNullOrWhiteSpace(item))
            return new GmodParsePathResult.Err("Item is empty");

        if (!item.startsWith(gmod.rootNode.code))
            return new GmodParsePathResult.Err(
                `Path must start with ${gmod.rootNode.code}`
            );

        const parts: string[] = item.split("/");

        for (const part of parts)
            if (!VIS.isISOString(part))
                return new GmodParsePathResult.Err(
                    `Input string contains invalid characters in part - ${part}`
                );

        const endPathNode = parts.pop();
        if (!endPathNode)
            return new GmodParsePathResult.Err(
                "No nodes found in input string"
            );

        const getNode = (
            code: string,
            gmod: Gmod,
            locations: Locations
        ): GmodNode | undefined => {
            const dashIndex = code.indexOf("-");
            if (dashIndex === -1) {
                const node = gmod.tryGetNode(code);
                if (!node) return;
                return node;
            } else {
                const node = gmod.tryGetNode(code.substring(0, dashIndex));
                if (!node) return;
                const location = locations.tryParse(
                    code.substring(dashIndex + 1)
                );
                if (!location) return;
                return node.withLocation(location);
            }
        };

        let endNode = getNode(endPathNode, gmod, locations);
        if (!endNode)
            return new GmodParsePathResult.Err("Failed to get end node");

        const parents = parts.map((code) => getNode(code, gmod, locations));
        if (parents.some((n) => n === undefined))
            return new GmodParsePathResult.Err("Failed to find any nodes");

        const pathParents = parents as GmodNode[];
        if (!GmodPath.isValid(pathParents, endNode))
            return new GmodParsePathResult.Err("Sequence of nodes are invalid");

        const visit = locationSetsVisitor();
        let prevNonNullLocation: number | null = null;
        const sets: [number, number][] = [];
        let setCounter = 0;
        for (let i = 0; i < pathParents.length + 1; i++) {
            const n = i < pathParents.length ? pathParents[i] : endNode;
            const set = visit(n, i, pathParents, endNode);
            if (set === null) {
                if (prevNonNullLocation === null && !!n.location)
                    prevNonNullLocation = i;
                continue;
            }

            const [start, end, location] = set;

            if (prevNonNullLocation !== null) {
                for (let j = prevNonNullLocation; j < start; j++) {
                    const pn =
                        i < pathParents.length ? pathParents[i] : endNode;
                    if (pn.location !== undefined)
                        return new GmodParsePathResult.Err(
                            `Expected all nodes in the set to be without individualization. Found ${pn.toString()}`
                        );
                }
            }
            prevNonNullLocation = null;
            sets[setCounter++] = [start, end];
            if (start === end) continue;

            for (let j = start; j <= end; j++) {
                if (j < pathParents.length) {
                    if (!!location)
                        pathParents[j] = pathParents[j].withLocation(location);
                    else pathParents[j] = pathParents[j].withoutLocation();
                } else {
                    if (!!location) endNode = endNode.withLocation(location);
                    else endNode = endNode.withoutLocation();
                }
            }
        }

        let currentSet: [number, number] = [1, -1];
        let currentSetIndex = 0;
        for (let i = 0; i < pathParents.length + 1; i++) {
            while (currentSetIndex < setCounter && currentSet[1] < i)
                currentSet = sets[currentSetIndex++];

            let insideSet = i >= currentSet[0] && i <= currentSet[1];

            var n = i < pathParents.length ? pathParents[i] : endNode;
            var expectedLocationNode =
                currentSet[1] != -1 && currentSet[1] < pathParents.length
                    ? pathParents[currentSet[1]]
                    : endNode;

            if (insideSet) {
                if (n.location?.equals(expectedLocationNode.location) === false)
                    return new GmodParsePathResult.Err(
                        `Expected all nodes in the set to be individualized the same. Found ${n.code} with location ${n.location}. Expected location ${expectedLocationNode.location}`
                    );
            } else {
                if (n.location !== undefined)
                    return new GmodParsePathResult.Err(
                        `Expected all nodes in the set to be without individualization. Found ${n}`
                    );
            }
        }

        return new GmodParsePathResult.Ok(
            new GmodPath(pathParents, endNode, true)
        );
    }

    public static async tryParseAsync(item: string, visVersion: VisVersion) {
        const gmod = await VIS.instance.getGmod(visVersion);
        const locations = await VIS.instance.getLocations(visVersion);

        return this.tryParse(item, locations, gmod);
    }

    public static async tryParseFromFullPathAsync(
        item: string,
        visVersion: VisVersion
    ) {
        const gmod = await VIS.instance.getGmod(visVersion);
        const locations = await VIS.instance.getLocations(visVersion);

        return this.tryParseFromFullPath(item, gmod, locations);
    }

    public clone(): GmodPath {
        return new GmodPath(
            this.parents.map((p) => p.clone()),
            this.node.clone()
        );
    }
}

function locationSetsVisitor() {
    let currentParentStart = -1;

    return (
        node: GmodNode,
        i: number,
        parents: GmodNode[],
        target: GmodNode
    ): [number, number, Location | undefined] | null => {
        const isParent = PotentialParentScopeTypes.includes(node.metadata.type);
        const isTargetNode = i === parents.length;
        if (currentParentStart === -1) {
            if (isParent) currentParentStart = i;
            if (isIndividualizable(node, isTargetNode))
                return [i, i, node.location];
        } else {
            if (isParent || isTargetNode) {
                let nodes: [number, number, Location | undefined] | null = null;
                if (currentParentStart + 1 === i) {
                    if (isIndividualizable(node, isTargetNode))
                        nodes = [i, i, node.location];
                } else {
                    let skippedOne = -1;
                    let hasComposition = false;
                    for (let j = currentParentStart + 1; j <= i; j++) {
                        const setNode =
                            j < parents.length ? parents[j] : target;
                        if (
                            !isIndividualizable(
                                setNode,
                                j == parents.length,
                                true
                            )
                        ) {
                            if (nodes !== null) skippedOne = j;
                            continue;
                        }

                        if (
                            nodes !== null &&
                            !!nodes[2] &&
                            !!setNode.location &&
                            !setNode.location.equals(nodes[2])
                        ) {
                            throw new Error(
                                `Mapping error: different locations in the same nodeset: ${nodes[2]}, ${setNode.location}`
                            );
                        }

                        if (skippedOne !== -1) {
                            throw new Error(
                                "Cant skip in the middle of individualizable set"
                            );
                        }

                        if (setNode.isFunctionComposition)
                            hasComposition = true;

                        const location: Location | undefined =
                            nodes === null || !nodes[2]
                                ? setNode.location
                                : nodes[2];
                        const start: number = !!nodes ? nodes[0] : j;
                        const end: number = j;
                        nodes = [start, end, location];
                    }

                    if (
                        nodes !== null &&
                        nodes[0] === nodes[1] &&
                        hasComposition
                    )
                        nodes = null;
                }

                currentParentStart = i;
                if (nodes !== null) {
                    let hasLeafNode = false;
                    for (let j = nodes[0]; j <= nodes[1]; j++) {
                        const setNode =
                            j < parents.length ? parents[j] : target;
                        if (setNode.isLeafNode || j === parents.length) {
                            hasLeafNode = true;
                            break;
                        }
                    }
                    if (hasLeafNode) return nodes;
                }
            }

            if (isTargetNode && isIndividualizable(node, isTargetNode))
                return [i, i, node.location];
        }
        return null;
    };
}

abstract class GmodParsePathResult {
    private constructor() {}
    static Ok = class Ok extends GmodParsePathResult {
        constructor(public path: GmodPath) {
            super();
        }
    };

    static Err = class Err extends GmodParsePathResult {
        constructor(public error: string) {
            super();
        }
    };
}
