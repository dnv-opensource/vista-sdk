import {
    Extensions as DataChannelListExtensions,
    DataChannelListDto,
    DataChannelExtension,
} from "./data-channel";
import {
    Extensions as TimeSeriesExtensions,
    TimeSeriesDto,
} from "./time-series-data";

class JSONExtensions {
    static TimeSeries = TimeSeriesExtensions;
    static DataChannelList = DataChannelListExtensions;
    static DataChannel = DataChannelExtension;
}

export { JSONExtensions };

export { TimeSeriesDto, DataChannelListDto };
export { Serializer as VistaJSONSerializer } from "./Serializer";
