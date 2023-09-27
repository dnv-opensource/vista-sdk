import { GmodNode, GmodPath, ILocalId, Result, VisVersion } from ".";
import { ILocalIdBuilder } from "./ILocalIdBuilder";
import { LocalId } from "./LocalId";
import { LocalIdBuilder } from "./LocalId.Builder";
import { PmodNode } from "./PmodNode";
import { TraversalHandlerResult } from "./types/Gmod";
import {
    NotRelevant,
    Parents,
    PmodInfo,
    TraversalContext,
    TraversalHandler,
    TraversalHandlerWithState,
} from "./types/Pmod";
import { Ok } from "./types/Result";
import {
    FormatNode,
    TreeHandler,
    TreeHandlerWithState,
    TreeNode,
} from "./types/Tree";
import {
    isNodeMergeable,
    isNodeSkippable,
    isNullOrWhiteSpace,
    naturalSort,
} from "./util/util";

export class Pmod {
    public visVersion: VisVersion;
    private _info?: PmodInfo;
    private _rootNode: PmodNode;
    private _nodeMap: Map<string, PmodNode>;

    private constructor(
        visVersion: VisVersion,
        rootNode: PmodNode,
        nodeMap: Map<string, PmodNode>,
        info?: PmodInfo
    ) {
        this.visVersion = visVersion;
        this._info = info;
        this._rootNode = rootNode;
        this._nodeMap = nodeMap;
    }

    public static createFromPaths(
        visVersion: VisVersion,
        paths: GmodPath[],
        info?: PmodInfo
    ) {
        const nodeMap = new Map<string, PmodNode>();

        const addToNodeMap = (key: string, node: GmodNode, depth: number) => {
            const pmodNode = new PmodNode(node, depth)
                .withEmptyRelations()
                .withId(key);
            nodeMap.set(key, pmodNode);
        };

        for (const path of paths) {
            const fullPath = path.getFullPath();

            for (let i = 0; i < fullPath.length; i++) {
                const pathNode = fullPath[i].withEmptyRelations();
                if (i === 0 && pathNode.code !== "VE")
                    throw new Error("Root node is not VE");

                if (pathNode.code === "VE") {
                    if (!nodeMap.has("VE")) addToNodeMap("VE", pathNode, 0);
                    continue;
                }

                const nodeId = fullPath
                    .slice(0, i + 1)
                    .map((p) => p.toString())
                    .join("/");

                const parentId = fullPath
                    .slice(0, i)
                    .map((p) => p.toString())
                    .join("/");

                if (isNullOrWhiteSpace(parentId) || isNullOrWhiteSpace(nodeId))
                    throw new Error(
                        "Invalid generated pathId for node:" +
                            pathNode.toString()
                    );

                if (nodeMap.has(nodeId)) continue;

                addToNodeMap(nodeId, pathNode, i);

                const childNode = nodeMap.get(nodeId);
                const parentNode = nodeMap.get(parentId);

                if (!childNode)
                    throw new Error(
                        "Unable to get node: " + pathNode.toString()
                    );

                if (!parentNode)
                    throw new Error(
                        "Unable to get parent node for node: " +
                            pathNode.toString()
                    );

                childNode.addParent(parentNode);
                parentNode.addChild(childNode);
            }
        }

        const rootNode = nodeMap.get("VE");
        if (!rootNode) throw Error("Failed to get rootNode 'VE'");

        return new Pmod(visVersion, rootNode, nodeMap, info);
    }

    public static createFromLocalIds(
        visVersion: VisVersion,
        localIds: ILocalId[] | ILocalIdBuilder[],
        info?: PmodInfo
    ) {
        const paths = localIds
            .flatMap((localId) => [localId.primaryItem, localId.secondaryItem])
            .filter((l) => l) as GmodPath[];

        return this.createFromPaths(visVersion, paths, info);
    }

    public get info() {
        return this._info;
    }

    public get rootNode() {
        return this._rootNode;
    }

    public get isValid() {
        return !Array.from(this._nodeMap.values()).find((n) => !n.isValid);
    }

    public get maxDepth() {
        return Math.max(
            ...Array.from(this._nodeMap.values()).map((n) => n.depth)
        );
    }

    public get numNodes() {
        return this._nodeMap.size;
    }

    public getNodeByPath(path: GmodPath) {
        return this._nodeMap.get(path.toFullPathString());
    }

    public getNodesByCode(code: string) {
        return Array.from(this._nodeMap.values()).filter(
            (n) => n.node.code === code
        );
    }

    public getNodesByPath(
        query: GmodPath,
        withoutLocation?: boolean
    ): PmodNode[] {
        const nodes: PmodNode[] = [];

        for (const value of this._nodeMap.values()) {
            if (
                withoutLocation
                    ? value.path.equalsWithoutLocation(query)
                    : value.path.equals(query)
            )
                nodes.push(value);
        }

        return nodes;
    }

    // Traversal
    public traverse<T>(
        handler: TraversalHandler | TraversalHandlerWithState<T>,
        params?: {
            fromPath?: GmodPath;
            state?: T;
        }
    ): boolean {
        const { fromPath = new GmodPath([], this._rootNode.node), state } =
            params || {};

        if (!state) {
            return this.traverseFromNodeWithState(
                handler as TraversalHandler,
                fromPath,
                (parents, node, handler) => handler(parents, node)
            );
        }

        return this.traverseFromNodeWithState(
            state,
            fromPath,
            handler as TraversalHandlerWithState<T>
        );
    }

    public traverseFromNodeWithState<T>(
        state: T,
        fromPath: GmodPath,
        handler: TraversalHandlerWithState<T>
    ): boolean {
        const parents = new Parents(fromPath);

        const context: TraversalContext<T> = {
            parents,
            handler,
            state,
        };

        const pmodNode = this.getNodeByPath(fromPath);
        if (!pmodNode) throw new Error("Start path not found in Pmod");
        return (
            this.traverseNode<T>(context, pmodNode) ===
            TraversalHandlerResult.Continue
        );
    }

    private traverseNode<T>(
        context: TraversalContext<T>,
        node: PmodNode
    ): TraversalHandlerResult {
        if (context.parents.has(node)) return TraversalHandlerResult.Continue;

        let result = context.handler(
            context.parents.asList,
            node,
            context.state
        );

        if (
            result === TraversalHandlerResult.Stop ||
            result === TraversalHandlerResult.SkipSubtree
        )
            return result;

        context.parents.push(node);

        for (const child of node.children) {
            result = this.traverseNode(context, child);
            if (result === TraversalHandlerResult.Stop) return result;
            else if (result === TraversalHandlerResult.SkipSubtree) continue;
        }

        context.parents.pop();
        return TraversalHandlerResult.Continue;
    }

    public getVisualizableTreeNodes<
        TNode extends TreeNode<TNode> = TreeNode,
        TState = unknown
    >(
        handler: TreeHandler<TNode> | TreeHandlerWithState<TState, TNode>,
        params?: {
            fromPath?: GmodPath;
            formatNode?: FormatNode<TNode>;
            /**@description matches paths without location. Then resolves least common parent and traverse tree from there.  */
            withoutLocation?: boolean;
            state?: TState;
        }
    ): Ok<TNode> | NotRelevant<TNode> {
        const {
            fromPath = new GmodPath([], this._rootNode.node),
            formatNode,
            withoutLocation,
            state,
        } = params ?? {};

        const hits = this.getNodesByPath(fromPath, withoutLocation);

        if (hits.length === 0)
            return new NotRelevant(
                this.getVisualizableTreeNodesInternal(handler, {
                    fromPath: undefined,
                    state,
                    formatNode,
                })
            );

        if (hits.length === 1) {
            return new Ok(
                this.getVisualizableTreeNodesInternal(handler, {
                    fromPath: hits[0].path,
                    state,
                    formatNode,
                })
            );
        }

        let maxDepth = Math.min(...hits.map((h) => h.depth));

        let commonParent: GmodNode | undefined = undefined;

        let commonDepth = 0;

        while (commonDepth < maxDepth) {
            let found = false;
            commonParent = undefined;
            for (const hit of hits) {
                const path = hit.path;
                const parent =
                    commonDepth === hit.depth
                        ? path.node
                        : path.parents[commonDepth];
                if (!commonParent) {
                    commonParent = parent;
                    continue;
                }
                if (!parent.equals(commonParent)) {
                    found = true;
                    commonDepth--;
                    break;
                }
            }
            if (found) break;
            commonDepth++;
        }

        const usedHit = hits[0];
        let result: GmodPath;
        if (commonDepth === usedHit.depth) {
            result = usedHit.path;
        } else {
            result = new GmodPath(
                hits[0].path.parents.slice(0, commonDepth) ?? [],
                hits[0].path.parents.slice(commonDepth)[0]
            );
        }

        return new Ok(
            this.getVisualizableTreeNodesInternal(handler, {
                fromPath: result,
                state,
                formatNode,
            })
        );
    }

    /**
     * @description Filter the Pmod accordingly to rules of visualization
     *      * Skip - isNodeSkippable
     *      * Merge - isNodeMergeable
     * @param handler Callback on each iteration
     * @param params Allows user to track its own state or decide root path
     * @returns A set of nodes from root path
     */

    private getVisualizableTreeNodesInternal<
        TNode extends TreeNode<TNode> = TreeNode,
        TState = unknown
    >(
        handler: TreeHandler<TNode> | TreeHandlerWithState<TState, TNode>,
        params?: {
            fromPath?: GmodPath;
            formatNode?: FormatNode<TNode>;
            state?: TState;
        }
    ): TNode {
        const {
            fromPath = new GmodPath([], this._rootNode.node),
            state,
            formatNode,
        } = params || {};

        const formatter: FormatNode<TNode> = formatNode
            ? formatNode
            : (node) => node as TNode;

        if (!state) {
            return this.createVisualizableTree<TreeHandler<TNode>, TNode>(
                handler as TreeHandler<TNode>,
                fromPath,
                formatter,
                (node, handler) => handler(node)
            );
        }

        return this.createVisualizableTree(
            state,
            fromPath,
            formatter,
            handler as TreeHandlerWithState<TState, TNode>
        );
    }

    private createVisualizableTree<
        TState,
        TNode extends TreeNode<TNode> = TreeNode
    >(
        userState: TState,
        fromPath: GmodPath,
        formatNode: FormatNode<TNode>,
        handler: TreeHandlerWithState<TState, TNode>
    ): TNode {
        type LocalTraverseContext = {
            nodes: TNode[];
            nodeMap: Map<string, TNode>;
            skippedNodesMap: Map<string, TNode>;
            fromPath: GmodPath;
            userState: TState;
        };

        const handleIteration = (
            parents: GmodNode[],
            node: GmodNode,
            context: LocalTraverseContext,
            handler?: TreeHandlerWithState<TState, TNode>
        ) => {
            const path = new GmodPath(parents, node);
            const key = path.toFullPathString();

            const parentNode = parents[parents.length - 1];

            let parent: TNode | undefined;

            if (parentNode) {
                const parentPath = new GmodPath(
                    parents.slice(0, parents.length - 1),
                    parentNode
                );
                const pKey = parentPath.toFullPathString();

                parent =
                    context.skippedNodesMap.get(pKey) ??
                    context.nodeMap.get(pKey);
            }

            const treeNode: TNode = formatNode({
                key,
                parent,
                path,
                children: [],
            });

            if (node.equals(context.fromPath.node)) {
                context.nodes.push(treeNode);
            }

            if (!parent) {
                context.nodeMap.set(key, treeNode);

                return TraversalHandlerResult.Continue;
            }

            const state = this.getMetadataState(parentNode, node);

            switch (state) {
                case "skip":
                    /**
                     * SKIP NODE:
                     * Condition:
                     *      See isNodeSkippable
                     *
                     * Change:
                     *      Skip node. No visual changes
                     *
                     * Logic:
                     *      Add reference to true parent to skippedNodesMap
                     *      Continue
                     *      In next iteration, the node check if current parent was skipped
                     *      Use the true parent reference as parent
                     */
                    context.skippedNodesMap.set(key, parent);
                    return TraversalHandlerResult.Continue;
                case "merge":
                    /**
                     * MERGE NODE:
                     * Condition:
                     *      See isNodeMergeable
                     *
                     * Change:
                     *      Merge parent with node
                     *
                     * Logic:
                     *      Add parent to 'treeNode.mergedNode'
                     *      Get the parent's parent
                     *      Set new parent to parent's parent
                     *      Delete current parent
                     *      Add child to new parent
                     */
                    if (!parent.parent) {
                        throw new Error(
                            "Unexpected state: No parent of parent when merge"
                        );
                    }

                    treeNode.mergedNode = Object.assign({}, parent);

                    const parentAsChildIndex = parent.parent.children.findIndex(
                        (c) => c.key === parent?.key
                    );
                    if (parentAsChildIndex === -1)
                        throw new Error(
                            "Unexpected state: Parent not found as child of parents parent"
                        );

                    parent = parent.parent as TNode;
                    parent.children.splice(parentAsChildIndex, 1);

                    treeNode.parent = parent;
                    break;
            }
            context.nodeMap.set(key, treeNode);

            parent.children.push(treeNode);

            return handler && handler(treeNode, context.userState);
        };

        const context: LocalTraverseContext = {
            nodes: [],
            nodeMap: new Map(),
            skippedNodesMap: new Map(),
            userState,
            fromPath,
        };

        const initParents = fromPath.parents;

        for (let i = 0; i < initParents.length; i++) {
            const parents = initParents.slice(0, i) ?? [];
            const node = initParents[i];

            handleIteration(parents, node, context);
        }

        this.traverse(
            (parents, node, context) => {
                const result = handleIteration(
                    parents.map((n) => n.node),
                    node.node,
                    context,
                    handler
                );
                if (!result) throw new Error("Expected result from iteration");
                return result;
            },
            { state: context, fromPath }
        );

        if (context.nodes.length > 1)
            throw new Error("Unexpected state: Should only find one root node");
        context.nodes.forEach((n) => this.sortChildren(n));

        return context.nodes[0];
    }

    private sortChildren(parent: TreeNode) {
        parent.children.sort((a, b) => naturalSort(a.key, b.key));
        parent.children.forEach((c) => this.sortChildren(c));
    }

    private getMetadataState(
        parent: GmodNode | PmodNode,
        node: GmodNode | PmodNode
    ): "skip" | "merge" | undefined {
        if (isNodeSkippable(parent, node)) {
            return "skip";
        } else if (isNodeMergeable(parent, node)) {
            return "merge";
        }
        return undefined;
    }
}
