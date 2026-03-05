import { DataChannelListDto } from "./data-channel/DataChannelList";
import { TimeSeriesDto } from "./time-series-data/TimeSeriesData";

export class Serializer {
    public static deserializeDataChannelList(
        payload: string,
    ): DataChannelListDto.DataChannelListPackage {
        return JSON.parse(payload);
    }

    public static deserializeTimeSeriesData(
        payload: string,
    ): TimeSeriesDto.TimeSeriesDataPackage {
        return JSON.parse(payload.toString());
    }

    public static serializeDataChannelList(
        dataChannelList: DataChannelListDto.DataChannelListPackage,
    ): string {
        return JSON.stringify(dataChannelList);
    }

    public static serializeTimeSeriesData(
        timeSeriesData: TimeSeriesDto.TimeSeriesDataPackage,
    ): string {
        return JSON.stringify(timeSeriesData);
    }
}
