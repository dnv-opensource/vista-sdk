import { Gmod } from "./Gmod";
import { Location, Locations } from "./Location";
import { GmodNodeDto } from "./types/GmodDto";
import { GmodNodeMetadata } from "./types/GmodNode";
import { VIS } from "./VIS";
import { VisVersion } from "./VisVersion";

export class GmodNode {
    private _id: string;
    private _visVersion: VisVersion;
    public readonly code: string;
    public readonly metadata: GmodNodeMetadata;
    private _location?: Location;
    private _children: GmodNode[];
    private _parents: GmodNode[];

    private constructor(previous: GmodNode);
    private constructor(
        id: string,
        visVersion: VisVersion,
        code: string,
        metadata: GmodNodeMetadata,
        location?: Location,
        parents?: GmodNode[],
        children?: GmodNode[],
    );
    private constructor(
        arg1: string | GmodNode,
        visVersion?: VisVersion,
        code?: string,
        metadata?: GmodNodeMetadata,
        location?: Location,
        parents?: GmodNode[],
        children?: GmodNode[],
    ) {
        if (arg1 instanceof GmodNode) {
            const previous = arg1;
            this._id = previous._id;
            this._visVersion = previous._visVersion;
            this.code = previous.code;
            this.metadata = previous.metadata;
            this._location = previous._location;
            this._children = previous._children;
            this._parents = previous._parents;
        } else {
            this._id = arg1;
            this._visVersion = visVersion!;
            this.code = code!;
            this.metadata = metadata!;
            this._location = location;
            this._children = children ?? [];
            this._parents = parents ?? [];
        }
    }

    public get visVersion() {
        return this._visVersion;
    }

    public get location() {
        return this._location;
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
                    Object.entries(normalAssignmentNames ?? {}),
                ),
            },
            undefined,
        );
    }

    public static create(
        id: string,
        visVersion: VisVersion,
        code: string,
        metadata: GmodNodeMetadata,
        location?: Location,
        parents?: GmodNode[],
        children?: GmodNode[],
    ) {
        return new GmodNode(
            id,
            visVersion,
            code,
            metadata,
            location,
            parents,
            children,
        );
    }

    public withEmptyRelations(): GmodNode {
        return new GmodNode(
            this._id,
            this._visVersion,
            this.code,
            this.metadata,
            this._location,
            [],
            [],
        );
    }

    public withoutLocation(): GmodNode {
        return this.with((s) => (s._location = undefined));
    }

    public withLocation(
        location: string | undefined,
        locations: Locations,
    ): GmodNode;
    public withLocation(location: Location): GmodNode;
    public withLocation(location: unknown, locations?: unknown): GmodNode {
        let result: GmodNode | undefined;
        if (location instanceof Location) {
            result = this.tryWithLocation(location);
        } else if (
            typeof location === "string" &&
            locations instanceof Locations
        ) {
            result = this.tryWithLocation(location, locations);
        }

        if (!result) throw new Error("Node not individualizable");
        return result;
    }

    public tryWithLocation(location: Location): GmodNode | undefined;
    public tryWithLocation(
        location: string | undefined,
        locations: Locations,
    ): GmodNode | undefined;
    public tryWithLocation(
        location: unknown,
        locations?: unknown,
    ): GmodNode | undefined {
        // if (!isIndividualizable(this, false, false)) return undefined; // TODO what to do about this case
        if (location instanceof Location) {
            return this.with((s) => (s._location = location));
        } else if (
            typeof location === "string" &&
            locations instanceof Locations
        ) {
            const parsedLocation = locations.tryParse(location);
            if (!parsedLocation) return undefined;
            return this.with((s) => (s._location = parsedLocation));
        }
    }

    public async withLocationAsync(location: string): Promise<GmodNode> {
        const locations = await VIS.instance.getLocations(this._visVersion);
        const parsedLocation = locations.parse(location);
        return this.withLocation(parsedLocation);
    }
    public async tryWithLocationAsync(
        location?: string,
    ): Promise<GmodNode | undefined> {
        const locations = await VIS.instance.getLocations(this._visVersion);
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

    public get isRoot() {
        return this.code === "VE";
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
        if (!!this._location !== !!other._location) return false;
        return (
            this.code === other.code &&
            (this._location?.equals(other._location) ?? true)
        );
    }

    public toString(): string {
        return this._location ? `${this.code}-${this._location}` : this.code;
    }

    public addChild(child: GmodNode) {
        this._children.push(child);
    }

    public addParent(parent: GmodNode) {
        this._parents.push(parent);
    }

    private getSortedString(node: GmodNode) {
        return node._location ? node.code + "-" + node._location : node.code;
    }

    public get isLeafNode(): boolean {
        return Gmod.isLeafNode(this.metadata);
    }
    public get isFunctionNode(): boolean {
        return Gmod.isFunctionNode(this.metadata);
    }

    public get isFunctionComposition(): boolean {
        return (
            (this.metadata.category === "ASSET FUNCTION" &&
                this.metadata.type === "COMPOSITION") ||
            (this.metadata.category === "PRODUCT FUNCTION" &&
                this.metadata.type === "COMPOSITION")
        );
    }

    public get isAssetFunctionNode(): boolean {
        return Gmod.isAssetFunctionNode(this.metadata);
    }

    public isIndividualizable(
        isTargetNode: boolean = false,
        isInSet: boolean = false,
    ): boolean {
        if (this.metadata.type === "GROUP") return false;
        if (this.metadata.type === "SELECTION") return false;
        if (this.isProductType) return false;
        if (this.metadata.category === "ASSET" && this.metadata.type == "TYPE")
            return false;
        if (this.isFunctionComposition)
            return (
                this.code[this.code.length - 1] === "i" ||
                isTargetNode ||
                isInSet
            );
        return true;
    }

    private with(u: { (state: GmodNode): void }): GmodNode {
        const n = this.clone();
        u && u(n);
        return n;
    }

    public clone(): GmodNode {
        return new GmodNode(this);
    }
}
