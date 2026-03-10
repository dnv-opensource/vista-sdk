import { DataListDto } from "./data-list/DataList";
import { Extensions as DataListExtensions } from "./data-list/Extensions";
import { Extensions as TimeSeriesExtensions } from "./time-series-data/Extensions";
import { TimeSeriesDto } from "./time-series-data/TimeSeriesData";

export class JSONExtensions {
    static DataList = DataListExtensions;
    static TimeSeries = TimeSeriesExtensions;
}

export { Serializer as JSONSerializer } from "./Serializer";
export { DataListDto, TimeSeriesDto };
