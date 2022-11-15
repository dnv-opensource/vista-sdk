import { Gmod } from "./Gmod";
import { Location, Locations } from "./Location";
import { GmodNodeDto } from "./types/GmodDto";
import { GmodNodeMetadata } from "./types/GmodNode";
import { naturalSort } from "./util/util";
import { VIS } from "./VIS";
import { VisVersion } from "./VisVersion";

export class GmodNode {
    private _id: string;
    public visVersion: VisVersion;
    public code: string;
    public metadata: GmodNodeMetadata;
    public location?: Location;
    private _children: GmodNode[];
    private _parents: GmodNode[];

    private constructor(
        id: string,
        visVersion: VisVersion,
        code: string,
        metadata: GmodNodeMetadata,
        location?: Location,
        parents?: GmodNode[],
        children?: GmodNode[]
    ) {
        this._id = id;
        this.visVersion = visVersion;
        this.code = code;
        this.location = location || undefined;
        this.metadata = metadata;

        this._children = children ?? [];
        this._parents = parents ?? [];
    }

    public static createFromDto(visVersion: VisVersion, dto: GmodNodeDto) {
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
            id = code,
        } = dto;

        return new GmodNode(
            id,
            visVersion,
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
            undefined
        );
    }

    public static create(
        id: string,
        visVersion: VisVersion,
        code: string,
        metadata: GmodNodeMetadata,
        location?: Location,
        parents?: GmodNode[],
        children?: GmodNode[]
    ) {
        return new GmodNode(
            id,
            visVersion,
            code,
            metadata,
            location,
            parents,
            children
        );
    }

    public withEmptyRelations(): GmodNode {
        return new GmodNode(
            this._id,
            this.visVersion,
            this.code,
            this.metadata,
            this.location,
            [],
            []
        );
    }

    public withoutLocation(): GmodNode {
        return this.with((s) => (s.location = undefined));
    }

    public withLocation(location: Location): GmodNode {
        return this.with((s) => (s.location = location));
    }

    public tryWithLocation(
        location: string | undefined,
        locations: Locations
    ): GmodNode {
        const parsedLocation = locations.tryParse(location);
        return !parsedLocation
            ? this.withoutLocation()
            : this.withLocation(parsedLocation);
    }

    public async withLocationAsync(location: string) {
        const locations = await VIS.instance.getLocations(this.visVersion);
        const parsedLocation = locations.parse(location);
        return this.withLocation(parsedLocation);
    }
    public async tryWithLocationAsync(location?: string): Promise<GmodNode> {
        const locations = await VIS.instance.getLocations(this.visVersion);
        return this.tryWithLocation(location, locations);
    }

    public get id() {
        return this._id;
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
        if (this.isAsset) return false;

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
        return this.code === other.code && this.location === other.location;
    }

    public toString(): string {
        return this.location ? `${this.code}-${this.location}` : this.code;
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

    public with(u: { (state: GmodNode): void }): GmodNode {
        const n = this.clone();
        u && u(n);
        return n;
    }

    public clone(): GmodNode {
        return Object.assign(
            new GmodNode(
                this._id,
                this.visVersion,
                this.code,
                { ...this.metadata },
                this.location?.clone(),
                [...this._parents],
                [...this._children]
            ),
            this
        );
    }
}
