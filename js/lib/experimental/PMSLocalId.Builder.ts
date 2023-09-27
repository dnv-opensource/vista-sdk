import {
    CodebookName,
    Codebooks,
    Gmod,
    GmodPath,
    LocalIdParsingErrorBuilder,
    MetadataTag,
    ParsingState,
    VisVersion,
} from "..";
import { ILocalIdBuilderGeneric } from "../ILocalIdBuilder";
import { LocalIdItems } from "../LocalId.Items";
import { Locations } from "../Location";
import { PMSLocalId } from "./PMSLocalId";
import { PMSLocalIdParser } from "./PMSLocalId.Parser";

export class PMSLocalIdBuilder
    implements ILocalIdBuilderGeneric<PMSLocalIdBuilder, PMSLocalId>
{
    public static readonly namingRule = "dnv-v2-experimental";
    public static readonly usedCodebooks = [
        CodebookName.Quantity,
        CodebookName.Content,
        CodebookName.State,
        CodebookName.Command,
        CodebookName.FunctionalServices,
        CodebookName.MaintenanceCategory,
        CodebookName.ActivityType,
        CodebookName.Position,
        CodebookName.Detail,
    ];

    public visVersion?: VisVersion;
    public verboseMode = false;
    private _items: LocalIdItems;

    public quantity?: MetadataTag;
    public content?: MetadataTag;
    public state?: MetadataTag;
    public command?: MetadataTag;
    public functionalServices?: MetadataTag;
    public maintenanceCategory?: MetadataTag;
    public activityType?: MetadataTag;
    public position?: MetadataTag;
    public detail?: MetadataTag;

    protected constructor() {
        this._items = new LocalIdItems();
    }
    public build(): PMSLocalId {
        if (this.isEmpty)
            throw new Error(
                "Cant build to PMSLocalId from empty PMSLocalIdBuilder"
            );
        if (!this.isValid)
            throw new Error(
                "Cant build to PMSLocalId from invalid PMSLocalIdBuilder"
            );

        return new PMSLocalId(this);
    }

    public static create(visVersion: VisVersion): PMSLocalIdBuilder {
        return new PMSLocalIdBuilder().withVisVersion(visVersion);
    }

    public clone() {
        return Object.assign(new PMSLocalIdBuilder(), this);
    }

    public get primaryItem() {
        return this._items?.primaryItem;
    }

    public get secondaryItem() {
        return this._items?.secondaryItem;
    }

    public get isValid(): boolean {
        return (
            !!this._items.primaryItem &&
            !!this.visVersion &&
            !!this.activityType &&
            !this.isEmptyMetadata
        );
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
            this.functionalServices,
            this.state,
            this.command,
            this.maintenanceCategory,
            this.activityType,
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
        if (!this.activityType) {
            errorBuilder.push({
                type: ParsingState.MetaActivityType,
                message: "Missing activity type",
            });
        }

        return errorBuilder;
    }

    public equals(other?: PMSLocalIdBuilder, skipOther = false): boolean {
        if (!other) return false;
        if (this.visVersion !== other.visVersion)
            throw new Error(
                "Cant compare local IDs from different VisVersions"
            );

        return !!(
            (this.primaryItem?.equals(other.primaryItem) ?? true) &&
            (this.secondaryItem?.equals(other.secondaryItem) ?? true) &&
            (this.quantity?.equals(other.quantity) ?? true) &&
            (this.functionalServices?.equals(other.functionalServices) ??
                true) &&
            (this.content?.equals(other.content) ?? true) &&
            (this.position?.equals(other.position) ?? true) &&
            (this.state?.equals(other.state) ?? true) &&
            (this.command?.equals(other.command) ?? true) &&
            (this.maintenanceCategory?.equals(other.maintenanceCategory) ??
                true) &&
            (this.activityType?.equals(other.activityType) ?? true) &&
            (this.detail?.equals(other.detail) ?? true) &&
            (skipOther ? true : other.equals(this, true))
        );
    }

    public toString(builder: string[] = []): string {
        if (!this.visVersion)
            throw new Error("No VisVersion configured on LocalId");
        const namingRule = `/${PMSLocalIdBuilder.namingRule}/`;

        builder.push(namingRule);

        builder.push("vis-");
        builder.push(this.visVersion);
        builder.push("/");

        this._items.append(builder, this.verboseMode);

        builder.push("meta/");
        this.quantity?.append(builder);
        this.content?.append(builder);
        this.functionalServices?.append(builder);
        this.state?.append(builder);
        this.command?.append(builder);
        this.maintenanceCategory?.append(builder);
        this.activityType?.append(builder);
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
            case CodebookName.FunctionalServices:
                return this.functionalServices;
            case CodebookName.Quantity:
                return this.quantity;
            case CodebookName.State:
                return this.state;
            case CodebookName.Content:
                return this.content;
            case CodebookName.Command:
                return this.command;
            case CodebookName.MaintenanceCategory:
                return this.maintenanceCategory;
            case CodebookName.ActivityType:
                return this.activityType;
            case CodebookName.Detail:
                return this.detail;
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${name}`
                );
        }
    }

    public withVisVersion(visVersion: VisVersion): PMSLocalIdBuilder {
        return this.with((s) => (s.visVersion = visVersion));
    }

    public withoutVisVersion(): PMSLocalIdBuilder {
        return this.with((s) => (s.visVersion = undefined));
    }

    public tryWithVisVersion(visVersion: VisVersion): PMSLocalIdBuilder {
        if (!visVersion) return this;
        return this.withVisVersion(visVersion);
    }

    public withVerboseMode(verboseMode: boolean): PMSLocalIdBuilder {
        return this.with((s) => (s.verboseMode = verboseMode));
    }

    public withoutVerboseMode(): PMSLocalIdBuilder {
        return this.with((s) => (s.verboseMode = false));
    }

    public withPrimaryItem(item: GmodPath): PMSLocalIdBuilder {
        return this.with((s) => (s._items.primaryItem = item));
    }

    public withoutPrimaryItem(): PMSLocalIdBuilder {
        return this.with((s) => (s._items.primaryItem = undefined));
    }

    public tryWithPrimaryItem(item?: GmodPath): PMSLocalIdBuilder {
        if (!item) return this;
        return this.withPrimaryItem(item);
    }

    public withSecondaryItem(item: GmodPath): PMSLocalIdBuilder {
        return this.with((s) => (s._items.secondaryItem = item));
    }

    public withoutSecondaryItem(): PMSLocalIdBuilder {
        return this.with((s) => (s._items.secondaryItem = undefined));
    }

    public tryWithSecondaryItem(item?: GmodPath): PMSLocalIdBuilder {
        if (!item) return this;
        return this.withSecondaryItem(item);
    }

    public withQuantity(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.quantity = metadataTag));
    }

    public withFunctionalServices(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.functionalServices = metadataTag));
    }

    public withContent(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.content = metadataTag));
    }

    public withPosition(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.position = metadataTag));
    }

    public withState(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.state = metadataTag));
    }

    public withCommand(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.command = metadataTag));
    }

    public withMaintenanceCategory(
        metadataTag: MetadataTag
    ): PMSLocalIdBuilder {
        return this.with((s) => (s.maintenanceCategory = metadataTag));
    }

    public withActivityType(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.activityType = metadataTag));
    }

    public withDetail(metadataTag: MetadataTag): PMSLocalIdBuilder {
        return this.with((s) => (s.detail = metadataTag));
    }

    public tryWithMetadataTag(metadataTag?: MetadataTag): PMSLocalIdBuilder {
        if (!metadataTag) return this;
        return this.withMetadataTag(metadataTag);
    }

    public withMetadataTag(metadataTag: MetadataTag): PMSLocalIdBuilder {
        switch (metadataTag.name) {
            case CodebookName.Position:
                return this.withPosition(metadataTag);
            case CodebookName.FunctionalServices:
                return this.withFunctionalServices(metadataTag);
            case CodebookName.Quantity:
                return this.withQuantity(metadataTag);
            case CodebookName.State:
                return this.withState(metadataTag);
            case CodebookName.Content:
                return this.withContent(metadataTag);
            case CodebookName.Command:
                return this.withCommand(metadataTag);
            case CodebookName.MaintenanceCategory:
                return this.withMaintenanceCategory(metadataTag);
            case CodebookName.ActivityType:
                return this.withActivityType(metadataTag);
            case CodebookName.Detail:
                return this.withDetail(metadataTag);
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${metadataTag.name}`
                );
        }
    }

    public withoutMetadataTag(name: CodebookName): PMSLocalIdBuilder {
        switch (name) {
            case CodebookName.Position:
                return this.withoutPosition();
            case CodebookName.FunctionalServices:
                return this.withoutFunctionalServices();
            case CodebookName.Quantity:
                return this.withoutQuantity();
            case CodebookName.State:
                return this.withoutState();
            case CodebookName.Content:
                return this.withoutContent();
            case CodebookName.Command:
                return this.withoutCommand();
            case CodebookName.MaintenanceCategory:
                return this.withoutMaintenanceCategory();
            case CodebookName.ActivityType:
                return this.withoutActivityType();
            case CodebookName.Detail:
                return this.withoutDetail();
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${name}`
                );
        }
    }

    private withoutPosition() {
        return this.with((s) => (s.position = undefined));
    }

    private withoutQuantity() {
        return this.with((s) => (s.quantity = undefined));
    }
    private withoutContent() {
        return this.with((s) => (s.content = undefined));
    }
    private withoutCommand() {
        return this.with((s) => (s.command = undefined));
    }
    private withoutState() {
        return this.with((s) => (s.state = undefined));
    }
    private withoutFunctionalServices() {
        return this.with((s) => (s.functionalServices = undefined));
    }
    private withoutMaintenanceCategory() {
        return this.with((s) => (s.maintenanceCategory = undefined));
    }
    private withoutActivityType() {
        return this.with((s) => (s.activityType = undefined));
    }
    private withoutDetail() {
        return this.with((s) => (s.detail = undefined));
    }

    public with(u: { (state: PMSLocalIdBuilder): void }): PMSLocalIdBuilder {
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
        return PMSLocalIdParser.parse(
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
        return PMSLocalIdParser.parseAsync(localIdString, errorBuilder);
    }

    public static tryParse(
        localIdStr: string | undefined,
        gmod: Gmod,
        codebooks: Codebooks,
        locations: Locations,
        errorBuilder?: LocalIdParsingErrorBuilder
    ) {
        return PMSLocalIdParser.tryParse(
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
        return PMSLocalIdParser.tryParseAsync(localIdString, errorBuilder);
    }
}
