import { GmodNodeDto, Gmod } from ".";
import { GmodNodeMetadata } from "./types/GmodNode";
import { naturalSort } from "./util/util";

export class GmodNode {
    private _id: string;
    public code: string;
    public metadata: GmodNodeMetadata;
    private _location: string;
    private _children: GmodNode[];
    private _parents: GmodNode[];

    private constructor(
        id: string,
        code: string,
        metadata: GmodNodeMetadata,
        location?: string,
        parents?: GmodNode[],
        children?: GmodNode[]
    ) {
        this._id = id;
        this.code = code;
        this._location = location || "";
        this.metadata = metadata;

        this._children = children ?? [];
        this._parents = parents ?? [];
    }

    public static createFromDto(dto: GmodNodeDto) {
        const {
            code,
            category,
            type,
            name,
            commonName,
            commonDefinition,
            definition,
            installSubstructure,
            normalAssignmentNames,
            location,
            id = code,
        } = dto;

        return new GmodNode(
            id,
            code,
            {
                category,
                type,
                name,
                commonName: commonName ?? undefined,
                definition: definition ?? undefined,
                commonDefinition: commonDefinition ?? undefined,
                installSubstructure: installSubstructure ?? undefined,
                normalAssignmentNames: new Map<string, string>(
                    Object.entries(normalAssignmentNames ?? {})
                ),
            },
            location
        );
    }

    public withLocation(_location: string) {
        return new GmodNode(
            this._id,
            this.code,
            this.metadata,
            _location,
            this.parents,
            this.children
        );
    }

    public withoutLocation(): GmodNode {
        return new GmodNode(
            this._id,
            this.code,
            this.metadata,
            undefined,
            this.parents,
            this.children
        );
    }

    public get id() {
        return this._id;
    }

    public set location(location: string) {
        this._location = location;
    }

    public get location(): string {
        return this._location;
    }

    public get children() {
        return this._children;
    }

    public get parents() {
        return this._parents;
    }

    public get isMappable() {
        if (!!this.productType) return false;
        if (!!this.productSelection) return false;
        if (this.isProductSelection) return false;

        const lastChar = this.code.charAt(this.code.length - 1);
        return lastChar !== "a" && lastChar !== "s";
    }

    public get isProductSelection() {
        return Gmod.isProductSelection(this.metadata);
    }

    public get isProductType() {
        return Gmod.isProductType(this.metadata);
    }

    public get isAsset() {
        return Gmod.isAsset(this.metadata);
    }

    public get productType(): GmodNode | undefined {
        if (this._children.length !== 1) return;
        if (!this.metadata.category.includes("FUNCTION")) return;

        const child = this._children[0];
        if (child.metadata.category !== "PRODUCT") return;
        if (child.metadata.type !== "TYPE") return;
        return child;
    }

    public get productSelection(): GmodNode | undefined {
        if (this._children.length != 1) return;
        if (!this.metadata.category.includes("FUNCTION")) return;

        const child = this._children[0];
        if (!child.metadata.category.includes("PRODUCT")) return;
        if (child.metadata.type !== "SELECTION") return;

        return child;
    }

    public isChild(node: GmodNode | string): boolean {
        return typeof node === "string"
            ? this.isChildFromCode(node)
            : this.isChildFromCode(node.code);
    }

    private isChildFromCode(code: string) {
        for (let i = 0; i < this._children.length; i++) {
            if (this._children[i].code == code) return true;
        }

        return false;
    }

    public equals(other?: GmodNode): boolean {
        if (!other) return false;
        return this.code === other.code && this._location === other._location;
    }

    public toString(): string {
        return this._location ? `${this.code}-${this._location}` : this.code;
    }

    public addChild(child: GmodNode) {
        this._children.push(child);
        this._children = this._children.sort((a, b) =>
            naturalSort(this.getSortedString(a), this.getSortedString(b))
        );
    }

    public addParent(parent: GmodNode) {
        this._parents.push(parent);
        this._parents = this._parents.sort((a, b) =>
            naturalSort(this.getSortedString(a), this.getSortedString(b))
        );
    }

    private getSortedString(node: GmodNode) {
        return node.location ? node.code + "-" + node.location : node.code;
    }

    public get isLeafNode(): boolean {
        return Gmod.isLeafNode(this.metadata);
    }
    public get isFunctionNode(): boolean {
        return Gmod.isFunctionNode(this.metadata);
    }
    public clone(): GmodNode {
        return new GmodNode(
            this._id,
            this.code,
            { ...this.metadata },
            this._location,
            [...this._parents],
            [...this._children]
        );
    }
}
