import { DataListDto } from "./data-list/DataList";
import { TimeSeriesDto } from "./time-series-data/TimeSeriesData";

export class Serializer {
    public static deserializeDataList(
        payload: string
    ): DataListDto.DataListPackage {
        return JSON.parse(payload);
    }

    public static serializeDataList(
        dataChannelList: DataListDto.DataListPackage
    ): string {
        return JSON.stringify(dataChannelList);
    }

    public static deserializeTimeSeriesData(
        payload: string
    ): TimeSeriesDto.TimeSeriesDataPackage {
        return JSON.parse(payload.toString());
    }

    public static serializeTimeSeriesData(
        timeSeriesData: TimeSeriesDto.TimeSeriesDataPackage
    ): string {
        return JSON.stringify(timeSeriesData);
    }
}
