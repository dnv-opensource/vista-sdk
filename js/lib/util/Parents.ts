import { GmodNode } from "..";

export class Parents {
    private readonly _occurrences: Map<string, number> = new Map();
    private readonly _parents: GmodNode[] = [];

    public push(parent: GmodNode): void {
        this._parents.push(parent);
        const prev = this._occurrences.get(parent.code) ?? 0;
        this._occurrences.set(parent.code, prev + 1);
    }

    public pop(): void {
        const popped = this._parents.pop();
        if (!popped) throw new Error("Cannot parents pop from empty array");
        const prev = this._occurrences.get(popped.code)!;
        if (prev === 1) this._occurrences.delete(popped.code);
        else this._occurrences.set(popped.code, prev - 1);
    }

    public occurrences(node: GmodNode): number {
        return this._occurrences.get(node.code) ?? 0;
    }

    public last(): GmodNode | undefined {
        return this._parents.length > 0
            ? this._parents[this._parents.length - 1]
            : undefined;
    }

    public get asList() {
        return [...this._parents];
    }
}
