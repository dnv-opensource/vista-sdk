import { TimeSeriesDto } from "./TimeSeriesData";
import { DataId } from "../../domain/time-series-data/DataId";
import { AssetIdentifier } from "../../domain/AssetIdentifier";
import { TimeSeries } from "../../domain/time-series-data/TimeSeriesData";

export class Extensions {
    public static toJsonDto(
        domain: TimeSeries.TimeSeriesDataPackage
    ): TimeSeriesDto.TimeSeriesDataPackage {
        const p = domain.package;
        const h = domain.package.header;

        const newPackage: TimeSeriesDto.TimeSeriesDataPackage = {
            Package: {
                Header: h
                    ? {
                          Author: h.author,
                          DateCreated: h.dateCreated,
                          DateModified: h.dateModified,
                          AssetId: h.assetId.toString(),
                          SystemConfiguration:
                              h.systemConfiguration?.map<TimeSeriesDto.ConfigurationReference>(
                                  (s) => ({
                                      ID: s.id,
                                      TimeStamp: s.timeStamp,
                                  })
                              ),
                          TimeSpan: h.timeSpan
                              ? { End: h.timeSpan.end, Start: h.timeSpan.start }
                              : undefined,
                          ...h.customHeaders,
                      }
                    : undefined,
                TimeSeriesData:
                    p.timeSeriesData.map<TimeSeriesDto.TimeSeriesData>((t) => ({
                        DataConfiguration: t.dataConfiguration
                            ? {
                                  TimeStamp: t.dataConfiguration.timeStamp,
                                  ID: t.dataConfiguration.id,
                              }
                            : undefined,
                        EventData: t.eventData
                            ? {
                                  NumberOfDataSet: t.eventData.numberOfDataSet,
                                  DataSet:
                                      t.eventData.dataSet?.map<TimeSeriesDto.EventDataSet>(
                                          (d) => ({
                                              TimeStamp: d.timeStamp,
                                              DataId: d.dataId.toString(),
                                              Value: d.value,
                                              Quality: d.quality,
                                          })
                                      ),
                              }
                            : undefined,
                        TabularData:
                            t.tabularData?.map<TimeSeriesDto.TabularData>(
                                (d) => ({
                                    NumberOfDataSet: d.numberOfDataSet,
                                    NumberOfDataPoints: d.numberOfDataPoints,
                                    DataId: d.dataId?.map((i) => i.toString()),
                                    DataSet: d.dataSet?.map((td) => ({
                                        Quality: td.quality,
                                        TimeStamp: td.timeStamp,
                                        Value: td.value,
                                    })),
                                })
                            ),
                        ...t.customProperties,
                    })),
            },
        };
        return newPackage;
    }

    public static async toDomainModel(
        dto: TimeSeriesDto.TimeSeriesDataPackage
    ): Promise<TimeSeries.TimeSeriesDataPackage> {
        const p = dto.Package;
        const h = dto.Package.Header;

        const timeSeriesData: TimeSeries.TimeSeriesData[] = [];

        for (let t of p.TimeSeriesData) {
            const {
                DataConfiguration: _,
                EventData: __,
                TabularData: ___,
                ...customProperties
            } = t;

            const eventDataSet: TimeSeries.EventDataSet[] = [];
            const tabularData: TimeSeries.TabularData[] = [];

            if (t.EventData?.DataSet) {
                for (let ed of t.EventData.DataSet) {
                    const dataId = await DataId.parseAsync(ed.DataId);
                    eventDataSet.push({
                        dataId: dataId,
                        timeStamp: ed.TimeStamp,
                        value: ed.Value,
                        quality: ed.Quality,
                    });
                }
            }

            if (t.TabularData) {
                for (let td of t.TabularData) {
                    const dataChannelIds: DataId[] = [];
                    for (let i of td.DataId ?? []) {
                        const dataId = await DataId.parseAsync(i);
                        dataChannelIds.push(dataId);
                    }

                    tabularData.push({
                        numberOfDataSet: td.NumberOfDataSet,
                        numberOfDataPoints: td.NumberOfDataPoints,
                        dataId:
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
                dataConfiguration: t.DataConfiguration
                    ? {
                          timeStamp: t.DataConfiguration.TimeStamp,
                          id: t.DataConfiguration.ID,
                      }
                    : undefined,
                eventData: {
                    dataSet: eventDataSet.length > 0 ? eventDataSet : undefined,
                    numberOfDataSet: t.EventData?.NumberOfDataSet,
                },
                tabularData: tabularData.length > 0 ? tabularData : undefined,
                customProperties,
            });
        }

        let header: TimeSeries.Header | undefined = undefined;
        if (h) {
            const {
                AssetId,
                Author,
                DateCreated,
                DateModified,
                SystemConfiguration,
                TimeSpan,
                ...customHeaders
            } = h;

            header = {
                assetId: AssetIdentifier.parse(h.AssetId),
                timeSpan: h.TimeSpan
                    ? { end: h.TimeSpan.End, start: h.TimeSpan.Start }
                    : undefined,
                dateCreated: h.DateCreated,
                dateModified: h.DateModified,
                author: h.Author,
                systemConfiguration: h.SystemConfiguration?.map((c) => ({
                    id: c.ID,
                    timeStamp: c.TimeStamp,
                })),
                customHeaders,
            };
        }

        return {
            package: {
                header: header,
                timeSeriesData: timeSeriesData,
            },
        };
    }
}
