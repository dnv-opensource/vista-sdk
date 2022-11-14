import { CodebookName, CodebookNames } from "./CodebookName";
import { Codebooks } from "./Codebooks";
import { Gmod } from "./Gmod";
import { GmodPath } from "./GmodPath";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";
import { LocalId } from "./LocalId";
import { LocalIdItems } from "./LocalId.Items";
import { LocalIdParser } from "./LocalId.Parsing";
import { Locations } from "./Location";
import { MetadataTag } from "./MetadataTag";
import { ParsingState } from "./types/LocalId";
import { VisVersion } from "./VisVersion";

export class LocalIdBuilder {
    public static readonly namingRule = "dnv-v2";
    public visVersion?: VisVersion;
    public verboseMode = false;
    private _items: LocalIdItems;

    public quantity?: MetadataTag;
    public content?: MetadataTag;
    public calculation?: MetadataTag;
    public state?: MetadataTag;
    public command?: MetadataTag;
    public type?: MetadataTag;
    public position?: MetadataTag;
    public detail?: MetadataTag;

    private constructor() {
        this._items = new LocalIdItems();
    }

    public static create(visVersion: VisVersion): LocalIdBuilder {
        return new LocalIdBuilder().withVisVersion(visVersion);
    }

    public clone() {
        return Object.assign(new LocalIdBuilder(), this);
    }

    public build(): LocalId {
        return new LocalId(this);
    }

    public get primaryItem() {
        return this._items?.primaryItem;
    }

    public get secondaryItem() {
        return this._items?.secondaryItem;
    }

    public get isValid(): boolean {
        return !!this._items.primaryItem && !this.isEmptyMetadata;
    }

    public get hasCustomTag(): boolean {
        return this.metadataTags.some((t) => t.isCustom);
    }

    public get isEmpty(): boolean {
        return (
            !this._items.primaryItem &&
            !this._items.secondaryItem &&
            this.isEmptyMetadata
        );
    }

    public get isEmptyMetadata(): boolean {
        return this.metadataTags.length === 0;
    }

    public get metadataTags(): MetadataTag[] {
        const metaDataList = [
            this.quantity,
            this.content,
            this.calculation,
            this.state,
            this.command,
            this.type,
            this.position,
            this.detail,
        ];
        return metaDataList.filter((m) => m) as MetadataTag[];
    }

    public validate(
        errorBuilder = new LocalIdParsingErrorBuilder()
    ): LocalIdParsingErrorBuilder {
        // Add validation for primaryItem
        if (!this.primaryItem) {
            errorBuilder.push(ParsingState.PrimaryItem);
        }
        if (this.isEmptyMetadata) {
            errorBuilder.push({
                type: ParsingState.EmptyState,
                message: "Missing any metadata tag",
            });
        }

        return errorBuilder;
    }

    public equals(other?: LocalIdBuilder, skipOther = false): boolean {
        if (!other) return false;
        if (this.visVersion !== other.visVersion)
            throw new Error(
                "Cant compare local IDs from different VisVersions"
            );

        return !!(
            (this.primaryItem?.equals(other.primaryItem) ?? true) &&
            (this.secondaryItem?.equals(other.secondaryItem) ?? true) &&
            (this.quantity?.equals(other.quantity) ?? true) &&
            (this.calculation?.equals(other.calculation) ?? true) &&
            (this.content?.equals(other.content) ?? true) &&
            (this.position?.equals(other.position) ?? true) &&
            (this.state?.equals(other.state) ?? true) &&
            (this.command?.equals(other.command) ?? true) &&
            (this.type?.equals(other.type) ?? true) &&
            (this.detail?.equals(other.detail) ?? true) &&
            (skipOther ? true : other.equals(this, true))
        );
    }

    public toString(builder: string[] = []): string {
        if (!this.visVersion)
            throw new Error("No VisVersion configured on LocalId");
        const namingRule = `/${LocalIdBuilder.namingRule}/`;

        builder.push(namingRule);

        builder.push("vis-");
        builder.push(this.visVersion);
        builder.push("/");

        this._items.append(builder, this.verboseMode);

        builder.push("meta/");
        this.quantity?.append(builder);
        this.content?.append(builder);
        this.calculation?.append(builder);
        this.state?.append(builder);
        this.command?.append(builder);
        this.type?.append(builder);
        this.position?.append(builder);
        this.detail?.append(builder);

        if (builder[builder.length - 1].endsWith("/")) {
            builder[builder.length - 1] = builder[builder.length - 1].slice(
                0,
                -1
            );
        }

        return builder.join("");
    }

    public getMetadataTag(name: CodebookName): MetadataTag | undefined {
        switch (name) {
            case CodebookName.Position:
                return this.position;
            case CodebookName.Calculation:
                return this.calculation;
            case CodebookName.Quantity:
                return this.quantity;
            case CodebookName.State:
                return this.state;
            case CodebookName.Content:
                return this.content;
            case CodebookName.Command:
                return this.command;
            case CodebookName.Type:
                return this.type;
            case CodebookName.Detail:
                return this.detail;
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${name}`
                );
        }
    }

    public withVisVersion(visVersion: VisVersion): LocalIdBuilder {
        return this.with((s) => (s.visVersion = visVersion));
    }

    public withVerboseMode(verboseMode: boolean): LocalIdBuilder {
        return this.with((s) => (s.verboseMode = verboseMode));
    }

    public withPrimaryItem(item?: GmodPath): LocalIdBuilder {
        return this.with((s) => (s._items.primaryItem = item));
    }

    public withSecondaryItem(item?: GmodPath): LocalIdBuilder {
        return this.with((s) => (s._items.secondaryItem = item));
    }

    public withQuantity(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.quantity = metadataTag));
    }

    public withCalculation(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.calculation = metadataTag));
    }

    public withContent(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.content = metadataTag));
    }

    public withPosition(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.position = metadataTag));
    }

    public withState(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.state = metadataTag));
    }

    public withCommand(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.command = metadataTag));
    }

    public withType(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.type = metadataTag));
    }

    public withDetail(metadataTag: MetadataTag): LocalIdBuilder {
        return this.with((s) => (s.detail = metadataTag));
    }

    public tryWithMetadataTag(metadataTag?: MetadataTag): LocalIdBuilder {
        if (!metadataTag) return this;
        return this.withMetadataTag(metadataTag);
    }

    public withMetadataTag(metadataTag: MetadataTag): LocalIdBuilder {
        switch (metadataTag.name) {
            case CodebookName.Position:
                return this.withPosition(metadataTag);
            case CodebookName.Calculation:
                return this.withCalculation(metadataTag);
            case CodebookName.Quantity:
                return this.withQuantity(metadataTag);
            case CodebookName.State:
                return this.withState(metadataTag);
            case CodebookName.Content:
                return this.withContent(metadataTag);
            case CodebookName.Command:
                return this.withCommand(metadataTag);
            case CodebookName.Type:
                return this.withType(metadataTag);
            case CodebookName.Detail:
                return this.withDetail(metadataTag);
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${metadataTag.name}`
                );
        }
    }

    public withoutMetadataTag(tagName: CodebookName): LocalIdBuilder {
        return this.with(
            // eslint-disable-next-line @typescript-eslint/ban-ts-comment
            // @ts-ignore
            (s) => (s[CodebookNames.toString(tagName)] = undefined)
        );
    }

    public with(u: { (state: LocalIdBuilder): void }): LocalIdBuilder {
        const n = this.clone();
        u && u(n);
        return n;
    }

    public static parse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return LocalIdParser.parse(
            localIdStr,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
    }

    public static async parseAsync(
        localIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return LocalIdParser.parseAsync(localIdString, errorBuilder);
    }

    public static tryParse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return LocalIdParser.tryParse(
            localIdStr,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
    }

    public static async tryParseAsync(
        localIdString: string | undefined,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return LocalIdParser.tryParseAsync(localIdString, errorBuilder);
    }
}
