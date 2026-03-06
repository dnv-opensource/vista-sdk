import { Extensions as TimeSeriesExtensions } from "./time-series-data/Extensions";
import { Extensions as DataListExtensions } from "./data-list/Extensions";
import { TimeSeriesDto } from "./time-series-data/TimeSeriesData";
import { DataListDto } from "./data-list/DataList";

export class JSONExtensions {
    static DataList = DataListExtensions;
    static TimeSeries = TimeSeriesExtensions;
}

export { TimeSeriesDto, DataListDto };
export { Serializer as VistaJSONSerializer } from "./Serializer";
