import { GmodPath } from "./GmodPath";
import {
    GmodPathQuery,
    GmodPathQueryBuilder,
    NodesBuilder,
    PathBuilder,
} from "./GmodPathQuery";
import { LocalId } from "./LocalId";
import {
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
} from "./MetadataTagsQuery";
import { VIS } from "./VIS";
import { VisVersionExtension } from "./VisVersion";

// Callback signatures mirroring C# delegates
export type PathQueryConfiguration = (path: PathBuilder) => GmodPathQuery;
export type NodesQueryConfiguration = (nodes: NodesBuilder) => GmodPathQuery;

// ---------------------------------------------------------------------------
// LocalIdQuery – immutable query object
// ---------------------------------------------------------------------------

export class LocalIdQuery {
    private readonly _builder: LocalIdQueryBuilder;

    /** @internal */
    constructor(builder: LocalIdQueryBuilder) {
        this._builder = builder;
    }

    /** Match against a LocalId instance. */
    public match(other: LocalId): Promise<boolean> {
        return this._builder.match(other);
    }

    /** Parse a string and match. */
    public async matchStr(other: string): Promise<boolean> {
        return this._builder.matchStr(other);
    }
}

// ---------------------------------------------------------------------------
// LocalIdQueryBuilder
// ---------------------------------------------------------------------------

export class LocalIdQueryBuilder {
    private _primaryItem: GmodPathQuery | undefined;
    private _secondaryItem: GmodPathQuery | undefined;
    private _tags: MetadataTagsQuery | undefined;
    private _requireSecondaryItem: boolean | undefined;

    private constructor(init?: {
        primaryItem?: GmodPathQuery;
        secondaryItem?: GmodPathQuery;
        tags?: MetadataTagsQuery;
        requireSecondaryItem?: boolean;
    }) {
        this._primaryItem = init?.primaryItem;
        this._secondaryItem = init?.secondaryItem;
        this._tags = init?.tags;
        this._requireSecondaryItem = init?.requireSecondaryItem;
    }

    // -- Accessors ---------------------------------------------------------

    public get primaryItem(): GmodPath | undefined {
        return this._primaryItem?.builder instanceof PathBuilder
            ? this._primaryItem.builder.gmodPath
            : undefined;
    }

    public get secondaryItem(): GmodPath | undefined {
        return this._secondaryItem?.builder instanceof PathBuilder
            ? this._secondaryItem.builder.gmodPath
            : undefined;
    }

    // -- Factory -----------------------------------------------------------

    public build(): LocalIdQuery {
        return new LocalIdQuery(this);
    }

    public static empty(): LocalIdQueryBuilder {
        return new LocalIdQueryBuilder();
    }

    /**
     * Create from an existing LocalId object.
     * Pre‐populates primary item, secondary item (or forbids it), and tags.
     */
    public static from(localId: LocalId): LocalIdQueryBuilder {
        let builder = LocalIdQueryBuilder.empty().withPrimaryItemFromQuery(
            GmodPathQueryBuilder.from(localId.primaryItem).build(),
        );

        builder =
            localId.secondaryItem !== undefined
                ? builder.withSecondaryItemFromQuery(
                      GmodPathQueryBuilder.from(localId.secondaryItem).build(),
                  )
                : builder.withoutSecondaryItem();

        builder = builder.withTagsFromQuery(
            MetadataTagsQueryBuilder.from(localId).build(),
        );
        return builder;
    }

    /**
     * Parse a local‐id string and create a builder.
     */
    public static async fromStr(
        localIdStr: string,
    ): Promise<LocalIdQueryBuilder> {
        const localId = await LocalId.parseAsync(localIdStr);
        return LocalIdQueryBuilder.from(localId);
    }

    // -- Primary item configuration ----------------------------------------

    /** Configure primary item from a nodes‐based query. */
    public withPrimaryItemFromNodes(
        configure: NodesQueryConfiguration,
    ): LocalIdQueryBuilder {
        return this.withPrimaryItemFromQuery(
            configure(GmodPathQueryBuilder.empty()),
        );
    }

    /** Configure primary item from an existing GmodPath with path‐based builder. */
    public withPrimaryItemFromPath(
        primaryItem: GmodPath,
        configure: PathQueryConfiguration,
    ): LocalIdQueryBuilder {
        const builder = GmodPathQueryBuilder.from(primaryItem);
        return this.withPrimaryItemFromQuery(configure(builder));
    }

    /** Reconfigure existing primary path item (must already be set from a Path builder). */
    public withPrimaryItemReconfigured(
        configure: PathQueryConfiguration,
    ): LocalIdQueryBuilder {
        if (!this.primaryItem) throw new Error("Primary item is null");
        return this.withPrimaryItemFromPath(this.primaryItem, configure);
    }

    /** Set primary item directly from a GmodPath. */
    public withPrimaryItem(primaryItem: GmodPath): LocalIdQueryBuilder {
        return this.withPrimaryItemFromQuery(
            GmodPathQueryBuilder.from(primaryItem).build(),
        );
    }

    /** Set primary item from a pre‐built query. */
    public withPrimaryItemFromQuery(
        primaryItem: GmodPathQuery,
    ): LocalIdQueryBuilder {
        return this._with({ primaryItem });
    }

    // -- Secondary item configuration --------------------------------------

    /** Configure secondary item from a nodes‐based query. */
    public withSecondaryItemFromNodes(
        configure: NodesQueryConfiguration,
    ): LocalIdQueryBuilder {
        return this.withSecondaryItemFromQuery(
            configure(GmodPathQueryBuilder.empty()),
        );
    }

    /** Reconfigure existing secondary path item (must already be set from a Path builder). */
    public withSecondaryItemReconfigured(
        configure: PathQueryConfiguration,
    ): LocalIdQueryBuilder {
        if (!this.secondaryItem) throw new Error("Secondary item is null");
        return this.withSecondaryItemFromPath(this.secondaryItem, configure);
    }

    /** Configure secondary item from an existing GmodPath with path‐based builder. */
    public withSecondaryItemFromPath(
        secondaryItem: GmodPath,
        configure: PathQueryConfiguration,
    ): LocalIdQueryBuilder {
        const builder = GmodPathQueryBuilder.from(secondaryItem);
        return this.withSecondaryItemFromQuery(configure(builder));
    }

    /** Set secondary item directly from a GmodPath. */
    public withSecondaryItem(secondaryItem: GmodPath): LocalIdQueryBuilder {
        return this.withSecondaryItemFromQuery(
            GmodPathQueryBuilder.from(secondaryItem).build(),
        );
    }

    /** Set secondary item from a pre‐built query. */
    public withSecondaryItemFromQuery(
        secondaryItem: GmodPathQuery,
    ): LocalIdQueryBuilder {
        return this._with({ secondaryItem, requireSecondaryItem: true });
    }

    /** Match any LocalId regardless of whether it has a secondary item. */
    public withAnySecondaryItem(): LocalIdQueryBuilder {
        return this._with({
            secondaryItem: undefined,
            requireSecondaryItem: undefined,
        });
    }

    /** Match only LocalIds that do NOT have a secondary item. */
    public withoutSecondaryItem(): LocalIdQueryBuilder {
        return this._with({
            secondaryItem: undefined,
            requireSecondaryItem: false,
        });
    }

    // -- Tags configuration ------------------------------------------------

    /** Configure tags using a builder function. */
    public withTags(
        configure: (builder: MetadataTagsQueryBuilder) => MetadataTagsQuery,
    ): LocalIdQueryBuilder {
        const builder = this._tags?.builder ?? MetadataTagsQueryBuilder.empty();
        return this.withTagsFromQuery(configure(builder));
    }

    /** Set tags from a pre‐built query. */
    public withTagsFromQuery(tags: MetadataTagsQuery): LocalIdQueryBuilder {
        return this._with({ tags });
    }

    // -- Convenience -------------------------------------------------------

    /** Strip all location constraints from primary and secondary items. */
    public withoutLocations(): LocalIdQueryBuilder {
        let b: LocalIdQueryBuilder = this;
        if (this.primaryItem) {
            b = b.withPrimaryItemReconfigured((p) =>
                p.withoutLocations().build(),
            );
        }
        if (this.secondaryItem) {
            b = b.withSecondaryItemReconfigured((s) =>
                s.withoutLocations().build(),
            );
        }
        return b;
    }

    // -- Match logic -------------------------------------------------------

    /** @internal */
    async matchStr(other: string): Promise<boolean> {
        const localId = await LocalId.parseAsync(other);
        return this.match(localId);
    }

    /** @internal */
    async match(other: LocalId): Promise<boolean> {
        let localId = other;
        if (
            VisVersionExtension.lessThan(other.visVersion, VIS.latestVisVersion)
        ) {
            const converted = await VIS.instance.convertLocalId(
                other,
                VIS.latestVisVersion,
            );
            if (!converted) throw new Error("Failed to convert local id");
            localId = converted;
        }

        if (
            this._primaryItem &&
            (await this._primaryItem.match(localId.primaryItem)) === false
        )
            return false;
        if (
            this._secondaryItem &&
            (await this._secondaryItem.match(localId.secondaryItem)) === false
        )
            return false;
        if (this._requireSecondaryItem !== undefined) {
            const hasSecondary = localId.secondaryItem !== undefined;
            if (this._requireSecondaryItem !== hasSecondary) return false;
        }
        if (this._tags && this._tags.match(localId) === false) return false;

        return true;
    }

    // -- Immutable copy helper ---------------------------------------------

    private _with(overrides: {
        primaryItem?: GmodPathQuery;
        secondaryItem?: GmodPathQuery;
        tags?: MetadataTagsQuery;
        requireSecondaryItem?: boolean;
    }): LocalIdQueryBuilder {
        return new LocalIdQueryBuilder({
            primaryItem:
                "primaryItem" in overrides
                    ? overrides.primaryItem
                    : this._primaryItem,
            secondaryItem:
                "secondaryItem" in overrides
                    ? overrides.secondaryItem
                    : this._secondaryItem,
            tags: "tags" in overrides ? overrides.tags : this._tags,
            requireSecondaryItem:
                "requireSecondaryItem" in overrides
                    ? overrides.requireSecondaryItem
                    : this._requireSecondaryItem,
        });
    }
}
