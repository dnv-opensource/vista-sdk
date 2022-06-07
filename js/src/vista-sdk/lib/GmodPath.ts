import { GmodNode, Gmod } from ".";
import { TraversalHandlerResult } from "./types/Gmod";
import { PathNode, ParseContext } from "./types/GmodPath";
import { isNullOrWhiteSpace } from "./util/util";

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
        }

        this.parents = parents;
        this.node = node;
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

    public getNode(depth: number): GmodNode {
        if (depth < 0 || depth > this.parents.length)
            throw new RangeError("Depth is out of bounds");
        return depth < this.parents.length ? this.parents[depth] : this.node;
    }

    public withoutLocation(): GmodPath {
        return new GmodPath(
            this.parents.map((p) => p.withoutLocation()),
            this.node.withoutLocation()
        );
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
    public static parse(item: string, gmod: Gmod): GmodPath {
        const path = this.tryParse(item, gmod);
        if (!path) throw new Error("Couldnt parse GmodPath from item");

        return path;
    }

    public static tryParse(
        item: string | undefined,
        gmod: Gmod
    ): GmodPath | undefined {
        const ERROR_IDENTIFIER = "GModPath - TryParse: ";
        if (!item || !gmod) return;
        if (isNullOrWhiteSpace(item)) return;

        item = item.trim();

        if (item[0] === "/") {
            item.slice(1);
        }

        const parts: PathNode[] = [];

        for (const partStr of item.split("/")) {
            if (partStr.includes("-")) {
                const split = partStr.split("-");
                parts.push({ code: split[0], location: split[1] });
            } else {
                parts.push({ code: partStr });
            }
        }

        if (parts.length === 0) return;
        if (parts.find((p) => isNullOrWhiteSpace(p.code))) return;

        const toFind = parts.shift();
        if (!toFind)
            throw new Error(
                ERROR_IDENTIFIER + "Invalid queue operation - Shift empty array"
            );
        const baseNode = gmod.tryGetNode(toFind.code);
        if (!baseNode) return;

        const context: ParseContext = { parts, toFind, locations: new Map() };

        gmod.traverse(
            (parents, current, context) => {
                const toFind = context.toFind;
                const found = current.code === toFind.code;

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

                const endNode = toFind.location
                    ? current.withLocation(toFind.location)
                    : current;

                let startNode =
                    pathParents.length > 0 &&
                    pathParents[0].parents.length === 1
                        ? pathParents[0].parents[0]
                        : endNode.parents.length === 1
                        ? endNode.parents[0]
                        : undefined;
                if (!startNode) throw new Error("Couldnt find start node");

                while (startNode.parents.length === 1) {
                    pathParents.unshift(startNode);
                    startNode = startNode.parents[0];
                }

                pathParents.unshift(gmod.rootNode);

                context.path = new GmodPath(pathParents, endNode);
                return TraversalHandlerResult.Stop;
            },
            { state: context, rootNode: baseNode }
        );

        if (context.path === undefined) {
            return;
        }

        return context.path;
    }

    public static parseFromFullPath(item: string, gmod: Gmod): GmodPath {
        const path = this.tryParseFromFullPath(item, gmod);

        if (!path) {
            throw new Error("Couldn't parse path from full path");
        }

        return path;
    }

    public static tryParseFromFullPath(
        item: string,
        gmod: Gmod
    ): GmodPath | undefined {
        if (!item || !gmod) return;
        if (isNullOrWhiteSpace(item)) return;

        item = item.trim();

        if (item[0] === "/") {
            item.slice(1);
        }

        const parts: string[] = item.split("/");

        const endPathNode = parts.pop();
        if (!endPathNode) return;

        const endNode = gmod.getNode(endPathNode);
        return new GmodPath(
            parts.map((p) => gmod.getNode(p)),
            endNode
        );
    }

    public isValid(): boolean {
        // so far only validates the location
        return (
            this.isValidLocation(this.node.location) &&
            !this.parents.find((p) => !this.isValidLocation(p.location))
        );
    }

    private isValidLocation(location?: string): boolean {
        if (!location) return true;
        return !isNullOrWhiteSpace(location) && /^[A-Z0-9]/g.test(location);
    }

    public clone(): GmodPath {
        return new GmodPath(
            this.parents.map((p) => p.clone()),
            this.node.clone()
        );
    }
}
