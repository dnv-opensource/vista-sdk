import { GmodNode, GmodPath } from ".";

export class PmodNode {
    private _node: GmodNode;
    private _depth: number;

    public constructor(node: GmodNode, depth: number) {
        if (!this.hasValidInput(node))
            throw new Error(
                "Invalid GmodNode with code: " +
                    node.toString() +
                    " - Parents: " +
                    node.parents.map((s) => s.toString()).join(", ")
            );
        this._node = node;
        this._depth = depth;
    }

    private hasValidInput(node: GmodNode) {
        return node.parents.length <= 1;
    }

    public get id() {
        return this._node.id;
    }

    public get depth() {
        return this._depth;
    }

    public get node() {
        return this._node;
    }

    public get code() {
        return this._node.code;
    }

    public get children() {
        return this._node.children.map((c) => new PmodNode(c, this._depth + 1));
    }

    public get parents() {
        return this._node.parents.map((p) => new PmodNode(p, this._depth - 1));
    }

    public get path(): GmodPath {
        return new GmodPath(getParents(this._node), this._node);
    }

    public get parent(): PmodNode | undefined {
        return this.parents[this.parents.length - 1];
    }

    public get isValid() {
        return this._node.parents.length <= 1;
    }

    public addChild(child: PmodNode) {
        this._node.addChild(child.node);
    }

    public addParent(parent: PmodNode) {
        this._node.addParent(parent.node);
    }

    public withEmptyRelations() {
        return new PmodNode(this._node.withEmptyRelations(), this._depth);
    }

    public toString() {
        return this._node.toString();
    }

    public withId(id: string) {
        return new PmodNode(
            GmodNode.create(
                id,
                this._node.visVersion,
                this._node.code,
                this._node.metadata,
                this._node.location,
                this._node.parents,
                this._node.children
            ),
            this._depth
        );
    }
}

function getParents(node: GmodNode, parents: GmodNode[] = []): GmodNode[] {
    for (const parent of node.parents) {
        parents.unshift(parent);
        getParents(parent, parents);
    }

    return parents;
}
