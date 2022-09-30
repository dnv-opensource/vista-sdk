import { DataChannelId, ShipId, TimeSeries } from "../../domain";
import { TimeSeriesDto } from "./TimeSeriesData";

export class Extensions {
    public static toJsonDto(
        domain: TimeSeries.TimeSeriesDataPackage
    ): TimeSeriesDto.TimeSeriesDataPackage {
        const p = domain.package;
        const h = domain.package.header;

        return {
            Package: {
                Header: h
                    ? {
                          Author: h.author,
                          DateCreated: h.dateCreated,
                          DateModified: h.dateModified,
                          ShipID: h.shipId.toString(),
                          SystemConfiguration: h.systemConfiguration?.map(
                              (s) => ({
                                  ID: s.id,
                                  TimeStamp: s.timeStamp,
                              })
                          ),
                          TimeSpan: h.timeSpan
                              ? { End: h.timeSpan.end, Start: h.timeSpan.start }
                              : undefined,
                          AdditionalProperties: h.customHeaders,
                      }
                    : undefined,
                TimeSeriesData: p.timeSeriesData.map((t) => ({
                    DataConfiguration: t.dataConfiguration
                        ? {
                              TimeStamp: t.dataConfiguration.timeStamp,
                              ID: t.dataConfiguration.id,
                          }
                        : undefined,
                    EventData: t.eventData
                        ? {
                              NumberOfDataSet: t.eventData.numberOfDataSet,
                              DataSet: t.eventData.dataSet?.map((d) => ({
                                  TimeStamp: d.timeStamp,
                                  DataChannelID: d.dataChannelId.toString(),
                                  Value: d.value,
                                  Quality: d.quality,
                              })),
                          }
                        : undefined,
                    TabularData: t.tabularData?.map((d) => ({
                        NumberOfDataSet: d.numberOfDataSet,
                        NumberOfDataChannel: d.numberOfDataChannel,
                        DataChannelID: d.dataChannelId?.map((i) =>
                            i.toString()
                        ),
                        DataSet: d.dataSet?.map((td) => ({
                            Quality: td.quality,
                            TimeStamp: td.timeStamp,
                            Value: td.value,
                        })),
                    })),
                    AdditionalProperties: t.customProperties,
                })),
            },
        };
    }

    public static async toDomainModel(
        dto: TimeSeriesDto.TimeSeriesDataPackage
    ): Promise<TimeSeries.TimeSeriesDataPackage> {
        const p = dto.Package;
        const h = dto.Package.Header;

        const timeSeriesData: TimeSeries.TimeSeriesData[] = [];

        for (let t of p.TimeSeriesData) {
            const eventDataSet: TimeSeries.EventDataSet[] = [];
            const tabularData: TimeSeries.TabularData[] = [];

            if (t.EventData?.DataSet) {
                for (let ed of t.EventData.DataSet) {
                    const dataChannelId = await DataChannelId.parseAsync(
                        ed.DataChannelID
                    );
                    eventDataSet.push({
                        dataChannelId,
                        timeStamp: ed.TimeStamp,
                        value: ed.Value,
                        quality: ed.Quality,
                    });
                }
            }

            if (t.TabularData) {
                for (let td of t.TabularData) {
                    const dataChannelIds: DataChannelId[] = [];
                    for (let i of td.DataChannelID ?? []) {
                        const dataChannelId = await DataChannelId.parseAsync(i);
                        dataChannelIds.push(dataChannelId);
                    }

                    tabularData.push({
                        numberOfDataSet: td.NumberOfDataSet,
                        numberOfDataChannel: td.NumberOfDataChannel,
                        dataChannelId:
                            dataChannelIds.length > 0
                                ? dataChannelIds
                                : undefined,
                        dataSet: td.DataSet?.map((d) => ({
                            timeStamp: d.TimeStamp,
                            value: d.Value,
                            quality: d.Quality,
                        })),
                    });
                }
            }

            timeSeriesData.push({
                dataConfiguration: t.DataConfiguration && {
                    timeStamp: t.DataConfiguration.TimeStamp,
                    id: t.DataConfiguration.ID,
                },
                customProperties: t.AdditionalProperties,
                eventData: {
                    dataSet: eventDataSet.length > 0 ? eventDataSet : undefined,
                    numberOfDataSet: t.EventData?.NumberOfDataSet,
                },
                tabularData: tabularData.length > 0 ? tabularData : undefined,
            });
        }

        return {
            package: {
                header: h
                    ? {
                          shipId: ShipId.parse(h.ShipID),
                          timeSpan: h.TimeSpan
                              ? { end: h.TimeSpan.End, start: h.TimeSpan.Start }
                              : undefined,
                          dateCreated: h.DateCreated,
                          dateModified: h.DateModified,
                          author: h.Author,
                          systemConfiguration: h.SystemConfiguration?.map(
                              (c) => ({ id: c.ID, timeStamp: c.TimeStamp })
                          ),
                          customHeaders: h.AdditionalProperties,
                      }
                    : undefined,
                timeSeriesData: timeSeriesData,
            },
        };
    }
}
