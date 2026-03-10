import {
    DataChannelExtension,
    DataChannelListDto,
    Extensions as DataChannelListExtensions,
} from "./data-channel";
import {
    TimeSeriesDto,
    Extensions as TimeSeriesExtensions,
} from "./time-series-data";

class JSONExtensions {
    static TimeSeries = TimeSeriesExtensions;
    static DataChannelList = DataChannelListExtensions;
    static DataChannel = DataChannelExtension;
}

export { JSONExtensions };

export { Serializer as JSONSerializer } from "./Serializer";
export { DataChannelListDto, TimeSeriesDto };
