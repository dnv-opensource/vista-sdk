import { CodebookName } from "./CodebookName";
import { LocalId } from "./LocalId";
import { MetadataTag } from "./MetadataTag";

/**
 * Immutable query for matching metadata tags on a LocalId.
 *
 * Built via `MetadataTagsQueryBuilder`, supports:
 * - Subset matching (default): all specified tags must be present, extras are OK
 * - Exact matching: tag sets must be identical
 */
export class MetadataTagsQuery {
    /** @internal */
    public readonly builder: MetadataTagsQueryBuilder;

    /** @internal */
    constructor(builder: MetadataTagsQueryBuilder) {
        this.builder = builder;
    }

    /**
     * Test whether the given LocalId matches this tag query.
     */
    public match(localId: LocalId | undefined): boolean {
        return this.builder.match(localId);
    }
}

/**
 * Builder for constructing MetadataTagsQuery instances.
 *
 * @example
 * ```ts
 * const query = MetadataTagsQueryBuilder.empty()
 *     .withTag(CodebookName.Content, "sea.water")
 *     .build();
 * query.match(localId); // true if localId has content=sea.water
 * ```
 */
export class MetadataTagsQueryBuilder {
    private readonly _tags: Map<CodebookName, MetadataTag>;
    private _matchExact: boolean;

    private constructor(
        tags?: Map<CodebookName, MetadataTag>,
        matchExact?: boolean,
    ) {
        this._tags = tags ? new Map(tags) : new Map();
        this._matchExact = matchExact ?? false;
    }

    public static empty(): MetadataTagsQueryBuilder {
        return new MetadataTagsQueryBuilder();
    }

    /**
     * Create a builder pre-populated with all tags from the given LocalId.
     *
     * @param localId - Source LocalId to extract tags from
     * @param allowOtherTags - If true (default), matched LocalIds may have additional tags
     */
    public static from(
        localId: LocalId,
        allowOtherTags = true,
    ): MetadataTagsQueryBuilder {
        let builder = new MetadataTagsQueryBuilder();
        for (const tag of localId.metadataTags) {
            builder = builder.withTag(tag);
        }
        builder = builder.withAllowOtherTags(allowOtherTags);
        return builder;
    }

    public build(): MetadataTagsQuery {
        return new MetadataTagsQuery(this);
    }

    public withTag(name: CodebookName, value: string): MetadataTagsQueryBuilder;
    public withTag(tag: MetadataTag): MetadataTagsQueryBuilder;
    public withTag(
        nameOrTag: CodebookName | MetadataTag,
        value?: string,
    ): MetadataTagsQueryBuilder {
        const tag =
            nameOrTag instanceof MetadataTag
                ? nameOrTag
                : new MetadataTag(nameOrTag, value!);
        const newTags = new Map(this._tags);
        newTags.set(tag.name, tag);
        return new MetadataTagsQueryBuilder(newTags, this._matchExact);
    }

    /**
     * Control whether exact or subset matching is used.
     *
     * @param allowOthers - true = subset matching (default), false = exact matching
     */
    public withAllowOtherTags(allowOthers: boolean): MetadataTagsQueryBuilder {
        return new MetadataTagsQueryBuilder(this._tags, !allowOthers);
    }

    /** @internal */
    public match(localId: LocalId | undefined): boolean {
        if (!localId) return false;

        const metadataTags = new Map<CodebookName, MetadataTag>();
        for (const tag of localId.metadataTags) {
            metadataTags.set(tag.name, tag);
        }

        if (this._tags.size > 0) {
            if (this._matchExact) {
                if (this._tags.size !== metadataTags.size) return false;
                for (const [name, tag] of this._tags) {
                    const other = metadataTags.get(name);
                    if (!other || !tag.equals(other)) return false;
                }
                return true;
            } else {
                for (const [, tag] of this._tags) {
                    const other = metadataTags.get(tag.name);
                    if (!other || !tag.equals(other)) return false;
                }
                return true;
            }
        } else {
            return !this._matchExact;
        }
    }
}
