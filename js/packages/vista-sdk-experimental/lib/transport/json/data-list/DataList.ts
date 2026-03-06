export namespace DataListDto {
    export interface DataListPackage {
        Package: Package;
    }
    export interface Package {
        Header: Header;
        DataList: DataList;
    }
    export interface Header {
        AssetId: string;
        DataListID: ConfigurationReference;
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
    export interface DataList {
        Data: Data[];
    }
    export interface Data {
        DataID: DataID;
        Property: Property;
    }
    export interface DataID {
        LocalID: string;
        ShortID?: string;
        NameObject?: NameObject;
    }
    export interface NameObject {
        NamingRule: string;
        [k: string]: unknown;
    }
    export interface Property {
        DataType: DataType;
        Format: Format;
        Range?: Range;
        Unit?: Unit;
        QualityCoding?: string;
        AlertPriority?: string;
        Name?: string;
        Remarks?: string;
        [k: string]: unknown;
    }
    export interface DataType {
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
        Preserve = "Preserve",
        Replace = "Replace",
        Collapse = "Collapse",
    }
}
