export namespace TimeSeriesDto {
    /**
     * Generated from https://transform.tools/json-schema-to-typescript
     *
     * A TimeSeriesData package for ISO19848
     *
     * Make sure to
     * - replace string to Date where needed
     */
    export interface TimeSeriesDataPackage {
        Package: Package;
    }
    export interface Package {
        Header?: Header;
        TimeSeriesData: TimeSeriesData[];
    }
    export interface Header {
        ShipID: string;
        TimeSpan?: TimeSpan;
        DateCreated?: Date;
        DateModified?: Date;
        Author?: string;
        SystemConfiguration?: ConfigurationReference[];
        [k: string]: unknown;
    }
    export interface TimeSpan {
        Start: Date;
        End: Date;
    }
    export interface ConfigurationReference {
        ID: string;
        TimeStamp: Date;
    }
    export interface TimeSeriesData {
        DataConfiguration?: ConfigurationReference;
        TabularData?: TabularData[];
        EventData?: EventData;
        [k: string]: unknown;
    }
    export interface TabularData {
        NumberOfDataSet?: number;
        NumberOfDataChannel?: number;
        DataChannelID?: string[];
        DataSet?: DataSet_Tabular[];
    }
    export interface DataSet_Tabular {
        TimeStamp: Date;
        Value: string[];
        Quality?: string[];
    }
    export interface EventData {
        NumberOfDataSet?: number;
        DataSet?: DataSet_Event[];
    }
    export interface DataSet_Event {
        TimeStamp: Date;
        DataChannelID: string;
        Value: string;
        Quality?: string;
    }
}
