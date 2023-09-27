import { GmodPath } from "./GmodPath";
import { ILocalIdBuilder } from "./ILocalIdBuilder";
import { MetadataTag } from "./MetadataTag";
import { VisVersion } from "./VisVersion";

export interface ILocalId {
    visVersion: VisVersion;
    verboseMode: boolean;
    primaryItem: GmodPath;
    secondaryItem: GmodPath | undefined;
    hasCustomTag: boolean;
    metadataTags: MetadataTag[];

    builder: ILocalIdBuilder;

    toString(): string;
}

export interface ILocalIdGeneric<T extends ILocalIdGeneric<T>>
    extends ILocalId {}
