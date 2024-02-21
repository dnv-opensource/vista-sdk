export namespace DataChannelListDto {
    /**
     * Generated from https://transform.tools/json-schema-to-typescript
     *
     * A DataChannelList package for ISO19848
     *
     * Make sure to
     * - replace string to Date where needed
     */
    export interface DataChannelListPackage {
        Package: Package;
    }
    export interface Package {
        Header: Header;
        DataChannelList: DataChannelList;
    }
    export interface Header {
        ShipID: string;
        DataChannelListID: ConfigurationReference;
        VersionInformation?: VersionInformation;
        Author?: string;
        DateCreated?: Date;
        [k: string]: unknown;
    }
    export interface ConfigurationReference {
        ID: string;
        Version?: string;
        TimeStamp: Date;
    }
    export interface VersionInformation {
        NamingRule: string;
        NamingSchemeVersion: string;
        ReferenceURL?: string;
    }
    export interface DataChannelList {
        DataChannel: DataChannel[];
    }
    export interface DataChannel {
        DataChannelID: DataChannelID;
        Property: Property;
    }
    export interface DataChannelID {
        LocalID: string;
        ShortID?: string;
        NameObject?: NameObject;
    }
    export interface NameObject {
        NamingRule: string;
        [k: string]: unknown;
    }
    export interface Property {
        DataChannelType: DataChannelType;
        Format: Format;
        Range?: Range;
        Unit?: Unit;
        QualityCoding?: string;
        AlertPriority?: string;
        Name?: string;
        Remarks?: string;
        [k: string]: unknown;
    }
    export interface DataChannelType {
        Type: string;
        UpdateCycle?: number;
        CalculationPeriod?: number;
    }
    export interface Format {
        Type: string;
        Restriction?: Restriction;
    }
    export interface Restriction {
        Enumeration?: string[];
        FractionDigits?: number;
        Length?: number;
        MaxExclusive?: number;
        MaxInclusive?: number;
        MaxLength?: number;
        MinExclusive?: number;
        MinInclusive?: number;
        MinLength?: number;
        Pattern?: string;
        TotalDigits?: number;
        WhiteSpace?: WhiteSpace;
    }
    export interface Range {
        High: number;
        Low: number;
    }
    export interface Unit {
        UnitSymbol: string;
        QuantityName?: string;
        [k: string]: unknown;
    }
    export enum WhiteSpace {
        Preserve = 'Preserve',
        Replace = 'Replace',
        Collapse = 'Collapse',
    }
}
