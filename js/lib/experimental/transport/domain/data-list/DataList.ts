import { ILocalId } from "../../../../ILocalId";
import { Version } from "../../../../transport/domain/data-channel/Version";
import { AssetIdentifier } from "../AssetIdentifier";

export namespace DataList {
    export type DataListPackage = {
        package: Package;
    };

    export type Package = {
        header: Header;
        dataList: DataList;
    };

    export type Header = {
        assetId: AssetIdentifier;
        dataListId: ConfigurationReference;
        versionInformation?: VersionInformation;
        author?: string;
        dateCreated?: Date;
        customHeaders?: { [k: string]: unknown };
    };

    export type ConfigurationReference = {
        id: string;
        version?: Version;
        timestamp: Date;
    };

    export type VersionInformation = {
        namingRule: string;
        namingSchemeVersion: string;
        referenceUrl?: string;
    };

    export type DataList = {
        data: Data[];
    };

    export type Data = {
        dataId: DataId;
        property: Property;
    };

    export type DataId = {
        localId: ILocalId;
        shortId?: string;
        nameObject?: NameObject;
    };

    export type NameObject = {
        namingRule: string;
        customProperties?: { [k: string]: unknown };
    };

    export type Property = {
        dataType: DataType;
        format: Format;
        range?: Range;
        unit?: Unit;
        qualityCoding?: string;
        alertPriority?: string;
        name?: string;
        remarks?: string;
        customProperties?: { [k: string]: unknown };
    };

    export type Format = {
        type: string;
        restriction?: Restriction;
    };

    export type Range = {
        high: number;
        low: number;
    };

    export type Unit = {
        unitSymbol: string;
        quantityName?: string;
        customProperties?: { [k: string]: unknown };
    };

    export type DataType = {
        type: string;
        updateCycle?: number;
        calculationPeriod?: number;
    };

    export type Restriction = {
        enumeration?: string[];
        fractionDigits?: number;
        length?: number;
        maxExclusive?: number;
        maxInclusive?: number;
        maxLength?: number;
        minExclusive?: number;
        minInclusive?: number;
        minLength?: number;
        pattern?: string;
        totalDigits?: number;
        whiteSpace?: WhiteSpace;
    };

    export enum WhiteSpace {
        Preserve = "Preserve",
        Replace = "Replace",
        Collapse = "Collapse",
    }
}
