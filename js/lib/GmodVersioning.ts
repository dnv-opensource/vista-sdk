import { VIS } from ".";
import { Gmod } from "./Gmod";
import { GmodNode } from "./GmodNode";
import { GmodPath, locationSetsVisitor } from "./GmodPath";
import { LocalId } from "./LocalId";
import { LocalIdBuilder } from "./LocalId.Builder";
import { TraversalHandlerResult } from "./types/Gmod";
import {
    ConversionType,
    GmodNodeConversion,
    GmodNodeConversionDto,
    GmodVersioningDto,
} from "./types/GmodVersioning";
import { VisVersion, VisVersionExtension, VisVersions } from "./VisVersion";

/**
 * Versioning node containing conversion rules for a specific VIS version
 */
class GmodVersioningNode {
    public readonly visVersion: VisVersion;
    private readonly _versioningNodeChanges: Map<string, GmodNodeConversion>;

    constructor(
        visVersion: VisVersion,
        dto: Record<string, GmodNodeConversionDto>,
    ) {
        this.visVersion = visVersion;
        this._versioningNodeChanges = new Map();

        for (const [code, nodeDto] of Object.entries(dto)) {
            this._versioningNodeChanges.set(code, {
                operations: new Set(
                    nodeDto.operations.map(
                        GmodVersioningNode.parseConversionType,
                    ),
                ),
                source: nodeDto.source,
                target: nodeDto.target ?? undefined,
                oldAssignment: nodeDto.oldAssignment ?? undefined,
                newAssignment: nodeDto.newAssignment ?? undefined,
                deleteAssignment: nodeDto.deleteAssignment ?? undefined,
            });
        }
    }

    public tryGetCodeChanges(code: string): GmodNodeConversion | undefined {
        return this._versioningNodeChanges.get(code);
    }

    private static parseConversionType(type: string): ConversionType {
        switch (type) {
            case "changeCode":
                return ConversionType.ChangeCode;
            case "merge":
                return ConversionType.Merge;
            case "move":
                return ConversionType.Move;
            case "assignmentChange":
                return ConversionType.AssignmentChange;
            case "assignmentDelete":
                return ConversionType.AssignmentDelete;
            default:
                throw new Error(`Invalid conversion type: ${type}`);
        }
    }
}

interface PathExistsContext {
    to: GmodNode;
    fromPath: GmodNode[];
    remainingParents: GmodNode[];
}

/**
 * Handles conversion of GMOD elements between different VIS versions.
 *
 * This class provides functionality to convert GmodNodes, GmodPaths, and LocalIds
 * from older VIS versions to newer versions, applying the appropriate conversion
 * rules for each version step.
 */
export class GmodVersioning {
    private readonly _versioningsMap: Map<VisVersion, GmodVersioningNode>;

    constructor(dto: Map<string, GmodVersioningDto>) {
        this._versioningsMap = new Map();

        for (const [versionStr, versionDto] of dto) {
            const parsedVersion = VisVersions.parse(versionStr);
            if (parsedVersion === undefined) {
                throw new Error(`Invalid VIS version string: ${versionStr}`);
            }
            const versioningNode = new GmodVersioningNode(
                parsedVersion,
                versionDto.items,
            );
            this._versioningsMap.set(parsedVersion, versioningNode);
        }
    }

    /**
     * Convert a GmodNode from source to target version.
     *
     * @param sourceVersion - The source VIS version
     * @param sourceNode - The node to convert
     * @param targetVersion - The target VIS version
     * @returns The converted node, or undefined if conversion failed
     */
    public async convertNode(
        sourceVersion: VisVersion,
        sourceNode: GmodNode,
        targetVersion: VisVersion,
    ): Promise<GmodNode | undefined> {
        this.validateSourceAndTargetVersions(sourceVersion, targetVersion);

        let node: GmodNode | undefined = sourceNode;
        let source = sourceVersion;

        // Step through each version one at a time
        while (VisVersionExtension.lessThan(source, targetVersion)) {
            if (!node) break;

            const target = VisVersionExtension.increment(source);
            if (!target) break;

            node = await this.convertNodeInternal(source, node, target);
            source = target;
        }

        return node;
    }

    private async convertNodeInternal(
        sourceVersion: VisVersion,
        sourceNode: GmodNode,
        targetVersion: VisVersion,
    ): Promise<GmodNode | undefined> {
        this.validateSourceAndTargetVersionPair(sourceVersion, targetVersion);
        let nextCode = sourceNode.code;

        const versioningNode = this.tryGetVersioningNode(targetVersion);
        if (versioningNode) {
            const change = versioningNode.tryGetCodeChanges(sourceNode.code);
            if (change?.target) {
                nextCode = change.target;
            }
        }

        const targetGmod = await VIS.instance.getGmod(targetVersion);
        const targetNode = targetGmod.tryGetNode(nextCode);
        if (!targetNode) return undefined;

        if (sourceNode.location) {
            const result = targetNode.tryWithLocation(sourceNode.location);
            if (
                !result ||
                result.location?.value !== sourceNode.location.value
            ) {
                throw new Error("Failed to set location");
            }
            return result;
        }

        return targetNode;
    }

    /**
     * Convert a LocalIdBuilder from source to target version.
     *
     * @param sourceLocalId - The LocalIdBuilder to convert
     * @param targetVersion - The target VIS version
     * @returns The converted LocalIdBuilder, or undefined if conversion failed
     */
    public async convertLocalIdBuilder(
        sourceLocalId: LocalIdBuilder,
        targetVersion: VisVersion,
    ): Promise<LocalIdBuilder | undefined> {
        if (sourceLocalId.visVersion === undefined) {
            throw new Error(
                "Cannot convert local ID without a specific VIS version",
            );
        }

        let targetLocalId = LocalIdBuilder.create(targetVersion);

        if (sourceLocalId.primaryItem) {
            const targetPrimaryItem = await this.convertPath(
                sourceLocalId.visVersion,
                sourceLocalId.primaryItem,
                targetVersion,
            );
            if (!targetPrimaryItem) return undefined;
            targetLocalId = targetLocalId.withPrimaryItem(targetPrimaryItem);
        }

        if (sourceLocalId.secondaryItem) {
            const targetSecondaryItem = await this.convertPath(
                sourceLocalId.visVersion,
                sourceLocalId.secondaryItem,
                targetVersion,
            );
            if (!targetSecondaryItem) return undefined;
            targetLocalId =
                targetLocalId.withSecondaryItem(targetSecondaryItem);
        }

        return targetLocalId
            .withVerboseMode(sourceLocalId.verboseMode)
            .tryWithMetadataTag(sourceLocalId.quantity)
            .tryWithMetadataTag(sourceLocalId.content)
            .tryWithMetadataTag(sourceLocalId.calculation)
            .tryWithMetadataTag(sourceLocalId.state)
            .tryWithMetadataTag(sourceLocalId.command)
            .tryWithMetadataTag(sourceLocalId.type)
            .tryWithMetadataTag(sourceLocalId.position)
            .tryWithMetadataTag(sourceLocalId.detail);
    }

    /**
     * Convert a LocalId from source to target version.
     *
     * @param sourceLocalId - The LocalId to convert
     * @param targetVersion - The target VIS version
     * @returns The converted LocalId, or undefined if conversion failed
     */
    public async convertLocalId(
        sourceLocalId: LocalId,
        targetVersion: VisVersion,
    ): Promise<LocalId | undefined> {
        const builder = await this.convertLocalIdBuilder(
            sourceLocalId.builder,
            targetVersion,
        );
        return builder?.build();
    }

    /**
     * Convert a GmodPath from source to target version.
     *
     * @param sourceVersion - The source VIS version
     * @param sourcePath - The path to convert
     * @param targetVersion - The target VIS version
     * @returns The converted path, or undefined if conversion failed
     */
    public async convertPath(
        sourceVersion: VisVersion,
        sourcePath: GmodPath,
        targetVersion: VisVersion,
    ): Promise<GmodPath | undefined> {
        const targetEndNode = await this.convertNode(
            sourceVersion,
            sourcePath.node,
            targetVersion,
        );

        if (!targetEndNode) return undefined;

        if (targetEndNode.isRoot) {
            return new GmodPath([], targetEndNode, true);
        }

        const targetGmod = await VIS.instance.getGmod(targetVersion);

        const qualifyingNodesPromises: Array<{
            sourceNode: GmodNode;
            targetNode: Promise<GmodNode | undefined>;
        }> = sourcePath.getFullPath().map((node) => ({
            sourceNode: node,
            targetNode: this.convertNode(sourceVersion, node, targetVersion),
        }));

        await Promise.all(qualifyingNodesPromises.map((qn) => qn.targetNode));

        const qualifyingNodes: Array<{
            sourceNode: GmodNode;
            targetNode: GmodNode;
        }> = [];
        for (const qnPromise of qualifyingNodesPromises) {
            const targetNode = await qnPromise.targetNode;
            if (!targetNode) {
                throw new Error("Could not convert node forward");
            }
            qualifyingNodes.push({
                sourceNode: qnPromise.sourceNode,
                targetNode,
            });
        }

        // Early return if simple conversion produces valid path
        const potentialParents = qualifyingNodes
            .slice(0, -1)
            .map((qn) => qn.targetNode);

        if (GmodPath.isValid(potentialParents, targetEndNode)) {
            return new GmodPath(potentialParents, targetEndNode, true);
        }

        // Build the path using complex conversion logic
        return this.buildPath(
            qualifyingNodes,
            targetGmod,
            targetEndNode,
            sourcePath,
        );
    }

    private buildPath(
        qualifyingNodes: Array<{ sourceNode: GmodNode; targetNode: GmodNode }>,
        targetGmod: Gmod,
        targetEndNode: GmodNode,
        sourcePath: GmodPath,
    ): GmodPath {
        const path: GmodNode[] = [];

        let i = 0;
        while (i < qualifyingNodes.length) {
            i = this.processQualifyingNode(
                qualifyingNodes,
                targetGmod,
                targetEndNode,
                path,
                i,
            );
        }

        return this.finalizePath(path, sourcePath);
    }

    private processQualifyingNode(
        qualifyingNodes: Array<{ sourceNode: GmodNode; targetNode: GmodNode }>,
        targetGmod: Gmod,
        targetEndNode: GmodNode,
        path: GmodNode[],
        i: number,
    ): number {
        const { sourceNode, targetNode } = qualifyingNodes[i];

        // Skip duplicate codes in sequence
        if (
            i > 0 &&
            targetNode.code === qualifyingNodes[i - 1].targetNode.code
        ) {
            // (Mes) The same as the current qualifying node, assumes same normal assignment [IF NOT THROW EXCEPTION, uncovered case] # noqa : E501
            if (
                sourceNode.productType &&
                sourceNode.productType.code !==
                    qualifyingNodes[i - 1].targetNode.productType?.code
            ) {
                throw new Error(
                    "Uncovered case: same code but different normal assignment",
                );
            }

            // Handle location propagation for skipped individualizable nodes
            if (targetNode.location) {
                const index = path.findIndex((n) => n.code === targetNode.code);
                if (index !== -1) {
                    if (
                        path[index].location &&
                        path[index].location?.value !==
                            targetNode.location.value
                    ) {
                        throw new Error(
                            `Failed to convert path at node ${targetNode}. ` +
                                "Uncovered case of multiple colliding locations",
                        );
                    }
                    if (!path[index].isIndividualizable(false, false)) {
                        throw new Error(
                            `Failed to convert path at node ${path[index]}. ` +
                                "Uncovered case of non-individualizable node",
                        );
                    }
                    // Dont overwrite existing location
                    if (!path[index].location) {
                        path[index] = path[index].withLocation(
                            targetNode.location,
                        );
                    }
                }
            }

            return i + 1;
        }

        // Check what changed
        const codeChanged = sourceNode.code !== targetNode.code;
        const sourceNormalAssignment = sourceNode.productType;
        const targetNormalAssignment = targetNode.productType;
        const normalAssignmentChanged = this.isNormalAssignmentChanged(
            sourceNormalAssignment,
            targetNormalAssignment,
        );

        if (codeChanged) {
            this.addToPath(targetGmod, path, targetNode);
        } else if (normalAssignmentChanged) {
            const wasDeleted =
                sourceNormalAssignment !== undefined &&
                targetNormalAssignment === undefined;

            if (!codeChanged) {
                this.addToPath(targetGmod, path, targetNode);
            }

            if (wasDeleted) {
                if (
                    targetNode.code === targetEndNode.code &&
                    i + 1 < qualifyingNodes.length
                ) {
                    const nextNode = qualifyingNodes[i + 1].targetNode;
                    if (nextNode.code !== targetNode.code) {
                        throw new Error(
                            "Normal assignment end node was deleted",
                        );
                    }
                }
                return i + 1;
            }

            if (
                targetNode.code !== targetEndNode.code &&
                targetNormalAssignment
            ) {
                this.addToPath(targetGmod, path, targetNormalAssignment);

                // Skip next node if assignment CHANGED (not new)
                if (
                    sourceNormalAssignment &&
                    targetNormalAssignment &&
                    sourceNormalAssignment.code !== targetNormalAssignment.code
                ) {
                    if (
                        i + 1 < qualifyingNodes.length &&
                        qualifyingNodes[i + 1].sourceNode.code !==
                            sourceNormalAssignment.code
                    ) {
                        throw new Error(
                            `Failed to convert path at node ${targetNode}. ` +
                                "Expected next qualifying source node to match target normal assignment",
                        );
                    }
                    i++;
                }
            }
        }

        // No changes - just add the target node
        if (!codeChanged && !normalAssignmentChanged) {
            this.addToPath(targetGmod, path, targetNode);
        }

        // Break if we've reached the target end node
        if (
            path.length > 0 &&
            path[path.length - 1].code === targetEndNode.code
        ) {
            return qualifyingNodes.length;
        }

        return i + 1;
    }

    private isNormalAssignmentChanged(
        source: GmodNode | undefined,
        target: GmodNode | undefined,
    ): boolean {
        return (
            (!source && !!target) ||
            (!!source && !target) ||
            (!!source && !!target && source.code !== target.code)
        );
    }

    private addToPath(
        targetGmod: Gmod,
        path: GmodNode[],
        node: GmodNode,
    ): void {
        if (path.length === 0) {
            path.push(node);
            return;
        }

        const prev = path[path.length - 1];
        if (prev.isChild(node)) {
            path.push(node);
            return;
        }

        // Traverse backwards removing parents until we find a valid connection
        for (let j = path.length - 1; j >= 0; j--) {
            const parent = path[j];
            const currentParents = path.slice(0, j + 1);
            const [exists, remaining] = this.pathExistsBetween(
                targetGmod,
                currentParents,
                node,
            );

            if (!exists) {
                // Check asset function node constraint
                const hasOtherAssetFunction = currentParents.some(
                    (n) => n.isAssetFunctionNode && n.code !== parent.code,
                );
                if (!hasOtherAssetFunction) {
                    throw new Error("Tried to remove last asset function node");
                }
                path.splice(j, 1);
            } else {
                // Found a valid connection - add all the missing intermediate nodes
                const nodes: GmodNode[] = [];
                if (node.location) {
                    for (const n of remaining) {
                        if (!n.isIndividualizable(false, true)) {
                            nodes.push(n);
                        } else {
                            nodes.push(n.withLocation(node.location));
                        }
                    }
                } else {
                    nodes.push(...remaining);
                }
                path.push(...nodes);
                break;
            }
        }

        path.push(node);
    }

    private pathExistsBetween(
        gmod: Gmod,
        fromPath: GmodNode[],
        toNode: GmodNode,
    ): [boolean, GmodNode[]] {
        // Find last asset function node to start traversal from
        let startNode = gmod.rootNode;
        for (let i = fromPath.length - 1; i >= 0; i--) {
            if (fromPath[i].isAssetFunctionNode) {
                startNode = fromPath[i];
                break;
            }
        }

        const state: PathExistsContext = {
            to: toNode,
            fromPath: fromPath,
            remainingParents: [],
        };

        const reachedEnd = gmod.traverse(
            (parents, node, state) => {
                if (node.code !== state.to.code) {
                    return TraversalHandlerResult.Continue;
                }

                let actualParents = [...parents];

                // Build full path to root
                while (!actualParents[0].isRoot) {
                    const parent = actualParents[0];
                    if (parent.parents.length !== 1) {
                        throw new Error("Invalid state - expected one parent");
                    }
                    actualParents.unshift(parent.parents[0]);
                }

                // Check if fromPath is prefix of current path
                if (actualParents.length < state.fromPath.length) {
                    return TraversalHandlerResult.Continue;
                }

                let match = true;
                for (let i = 0; i < state.fromPath.length; i++) {
                    if (actualParents[i].code !== state.fromPath[i].code) {
                        match = false;
                        break;
                    }
                }

                if (match) {
                    // Return remaining parents that are not in fromPath
                    state.remainingParents = actualParents.filter(
                        (p) => !state.fromPath.some((fp) => fp.code === p.code),
                    );
                    return TraversalHandlerResult.Stop;
                }

                return TraversalHandlerResult.Continue;
            },
            { rootNode: startNode, state },
        );

        return [!reachedEnd, state.remainingParents];
    }

    private finalizePath(path: GmodNode[], sourcePath: GmodPath): GmodPath {
        if (path.length === 0) {
            throw new Error(`Did not end up with valid path for ${sourcePath}`);
        }

        let potentialParents = path.slice(0, -1);
        let finalNode = path[path.length - 1];

        // Fix individualizable nodes with location
        const visit = locationSetsVisitor();
        for (let i = 0; i < potentialParents.length + 1; i++) {
            const n =
                i < potentialParents.length ? potentialParents[i] : finalNode;
            const setNodes = visit(n, i, potentialParents, finalNode);
            if (!setNodes) {
                if (n.location) break;
                continue;
            }

            const [startIdx, endIdx, location] = setNodes;
            if (startIdx === endIdx) continue;
            if (!location) continue;

            for (let j = startIdx; j <= endIdx; j++) {
                if (j < potentialParents.length) {
                    potentialParents[j] =
                        potentialParents[j].withLocation(location);
                } else {
                    finalNode = finalNode.withLocation(location);
                }
            }
        }

        if (!GmodPath.isValid(potentialParents, finalNode)) {
            throw new Error(`Did not end up with valid path for ${sourcePath}`);
        }

        return new GmodPath(potentialParents, finalNode, true);
    }

    private tryGetVersioningNode(
        visVersion: VisVersion,
    ): GmodVersioningNode | undefined {
        return this._versioningsMap.get(visVersion);
    }

    private validateSourceAndTargetVersions(
        sourceVersion: VisVersion,
        targetVersion: VisVersion,
    ): void {
        if (!VisVersionExtension.isValid(sourceVersion)) {
            throw new Error(`Invalid source VIS version: ${sourceVersion}`);
        }
        if (!VisVersionExtension.isValid(targetVersion)) {
            throw new Error(`Invalid target VIS version: ${targetVersion}`);
        }
        if (
            VisVersionExtension.greaterThanOrEqual(sourceVersion, targetVersion)
        ) {
            throw new Error("Source version must be less than target version");
        }
    }

    private validateSourceAndTargetVersionPair(
        sourceVersion: VisVersion,
        targetVersion: VisVersion,
    ): void {
        if (
            VisVersionExtension.greaterThanOrEqual(sourceVersion, targetVersion)
        ) {
            throw new Error("Source version must be less than target version");
        }
        const nextVersion = VisVersionExtension.increment(sourceVersion);
        if (nextVersion !== targetVersion) {
            throw new Error(
                "Target version must be exactly one version higher than source version",
            );
        }
    }
}
