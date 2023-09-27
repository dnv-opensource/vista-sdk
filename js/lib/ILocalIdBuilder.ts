import { CodebookName } from "./CodebookName";
import { GmodPath } from "./GmodPath";
import { ILocalId, ILocalIdGeneric } from "./ILocalId";
import { MetadataTag } from "./MetadataTag";
import { VisVersion } from "./VisVersion";
import { LocalIdParsingErrorBuilder } from "./internal/LocalIdParsingErrorBuilder";

export interface ILocalIdBuilder {
    visVersion?: VisVersion;
    verboseMode: boolean;
    primaryItem?: GmodPath;
    secondaryItem?: GmodPath;
    hasCustomTag: boolean;

    metadataTags: MetadataTag[];

    isValid: boolean;
    isEmpty: boolean;

    toString(): string;
    toString(builder?: string[]): string;
    equals(other?: ILocalIdBuilder): boolean;
    validate(
        errorBuilder?: LocalIdParsingErrorBuilder
    ): LocalIdParsingErrorBuilder;

    build(): ILocalId;
}

export interface ILocalIdBuilderGeneric<
    TBuilder extends ILocalIdBuilderGeneric<TBuilder, TResult>,
    TResult extends ILocalIdGeneric<TResult>
> extends ILocalIdBuilder {
    withVisVersion(visVersion: string): TBuilder;
    withVisVersion(visVersion: VisVersion): TBuilder;
    tryWithVisVersion(visVersion?: string): TBuilder;
    tryWithVisVersion(visVersion?: VisVersion): TBuilder;
    withoutVisVersion(): TBuilder;

    withVerboseMode(verbodeMode: boolean): TBuilder;

    withPrimaryItem(item: GmodPath): TBuilder;
    tryWithPrimaryItem(item?: GmodPath): TBuilder;
    withoutPrimaryItem(): TBuilder;

    withSecondaryItem(item: GmodPath): TBuilder;
    tryWithSecondaryItem(item?: GmodPath): TBuilder;
    withoutSecondaryItem(): TBuilder;

    withMetadataTag(metadataTag: MetadataTag): TBuilder;
    tryWithMetadataTag(metadataTag?: MetadataTag): TBuilder;
    withoutMetadataTag(name: CodebookName): TBuilder;

    build(): TResult;
}
