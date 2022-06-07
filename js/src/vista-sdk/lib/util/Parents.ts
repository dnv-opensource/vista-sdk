import { GmodNode } from "..";

export class Parents {
    private _codes: Set<string> = new Set();
    private _parents: GmodNode[] = [];

    public push(parent: GmodNode): void {
        this._codes.add(parent.code);
        this._parents.push(parent);
    }

    public pop(): void {
        const popped = this._parents.pop();
        if (!popped) throw new Error("Cannot parents pop from empty array");

        this._codes.delete(popped.code);
    }

    public has(parent: GmodNode): boolean {
        return this._codes.has(parent.code);
    }

    public get asList() {
        return [...this._parents];
    }
}
