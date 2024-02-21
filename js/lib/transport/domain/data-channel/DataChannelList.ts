import { ShipId } from "..";
import { LocalId } from "../../..";
import { Version } from "./Version";

export namespace DataChannelList {
    export type DataChannelListPackage = {
        package: Package;
    };

    export type Package = {
        header: Header;
        dataChannelList: DataChannelList;
    };

    export type Header = {
        shipId: ShipId;
        dataChannelListId: ConfigurationReference;
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

    export type DataChannelList = {
        dataChannel: DataChannel[];
    };

    export type DataChannel = {
        dataChannelId: DataChannelId;
        property: Property;
    };

    export type DataChannelId = {
        localId: LocalId;
        shortId?: string;
        nameObject?: NameObject;
    };

    export type NameObject = {
        namingRule: string;
        customProperties?: { [k: string]: unknown };
    };

    export type Property = {
        dataChannelType: DataChannelType;
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

    export type DataChannelType = {
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
        Preserve = 'Preserve',
        Replace = 'Replace',
        Collapse = 'Collapse',
    }
}
