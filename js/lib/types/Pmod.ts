import { GmodPath, ImoNumber, PmodNode, TreeNode } from "..";
import { TraversalHandlerResult } from "./Gmod";

export type PmodInfo = Partial<{
    vesselName: string;
    vesselId: string;
    imoNumber: ImoNumber;
    timestamp: Date;
}>;

export type TraversalHandler = (
    parents: PmodNode[],
    node: PmodNode
) => TraversalHandlerResult;
export type TraversalHandlerWithState<T> = (
    parents: PmodNode[],
    node: PmodNode,
    state: T
) => TraversalHandlerResult;

export type TraversalContext<T> = {
    parents: Parents;
    handler: TraversalHandlerWithState<T>;
    state: T;
};

export class NotRelevant<TNode extends TreeNode<TNode>> {
    public value;
    constructor(data: TNode) {
        this.value = data;
    }
}

export class Parents {
    private _codes: Set<string> = new Set();
    private _parents: PmodNode[];

    constructor(path: GmodPath) {
        this._parents = [];

        let depth = 0;
        for (const p of path.parents) {
            const pmodNode = new PmodNode(p, depth);
            this.push(pmodNode);
            depth++;
        }
    }

    public push(parent: PmodNode): void {
        this._codes.add(parent.code);
        this._parents.push(parent);
    }

    public unshift(parent: PmodNode): void {
        this._codes.add(parent.code);
        this._parents.unshift(parent);
    }

    public pop(): void {
        const popped = this._parents.pop();
        if (!popped) throw new Error("Cannot parents pop from empty array");

        this._codes.delete(popped.code);
    }

    public has(parent: PmodNode): boolean {
        return this._codes.has(parent.code);
    }

    public get asList() {
        return [...this._parents];
    }

    public get length() {
        return this._parents.length;
    }
}
