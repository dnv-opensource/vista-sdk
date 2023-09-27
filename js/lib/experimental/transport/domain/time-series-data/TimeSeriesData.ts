import { AssetIdentifier } from "../AssetIdentifier";
import { DataId } from "./DataId";

export namespace TimeSeries {
    export type TimeSeriesDataPackage = {
        package: Package;
    };

    export type Package = {
        header?: Header;
        timeSeriesData: TimeSeriesData[];
    };

    export type Header = {
        assetId: AssetIdentifier;
        timeSpan?: TimeSpan;
        dateCreated?: Date;
        dateModified?: Date;
        author?: string;
        systemConfiguration?: ConfigurationReference[];
        customHeaders?: { [k: string]: unknown };
    };

    export type ConfigurationReference = {
        id: string;
        timeStamp: Date;
    };

    export type TimeSpan = { start: Date; end: Date };

    export type TimeSeriesData = {
        dataConfiguration?: ConfigurationReference;
        tabularData?: TabularData[];
        eventData?: EventData;
        customProperties?: { [k: string]: unknown };
    };

    export type TabularData = {
        numberOfDataSet?: string;
        numberOfDataPoints?: string;
        dataId?: DataId[];
        dataSet?: TabularDataSet[];
    };

    export type TabularDataSet = {
        timeStamp: Date;
        value: string[];
        quality?: string[];
    };

    export type EventData = {
        numberOfDataSet?: string;
        dataSet?: EventDataSet[];
    };

    export type EventDataSet = {
        timeStamp: Date;
        dataId: DataId;
        value: string;
        quality?: string;
    };
}
