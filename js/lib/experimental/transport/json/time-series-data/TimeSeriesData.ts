export namespace TimeSeriesDto {
    export interface TimeSeriesDataPackage {
        Package: Package;
    }
    export interface Package {
        Header?: Header;
        TimeSeriesData: TimeSeriesData[];
    }
    export interface Header {
        AssetId: string;
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
        NumberOfDataPoints?: number;
        DataId?: string[];
        DataSet?: TabularDataSet[];
    }
    export interface TabularDataSet {
        TimeStamp: Date;
        Value: string[];
        Quality?: string[];
    }
    export interface EventData {
        NumberOfDataSet?: number;
        DataSet?: EventDataSet[];
    }
    export interface EventDataSet {
        TimeStamp: Date;
        DataId: string;
        Value: string;
        Quality?: string;
    }
}
