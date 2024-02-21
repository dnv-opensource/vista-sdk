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
                          ...h.customHeaders,
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
                    ...t.customProperties,
                })),
            },
        };
    }

    public static async toDomainModel(
        dto: TimeSeriesDto.TimeSeriesDataPackage
    ): Promise<TimeSeries.TimeSeriesDataPackage> {
        const timeSeriesData: TimeSeries.TimeSeriesData[] = [];

        for (let {
            DataConfiguration,
            EventData,
            TabularData,
            ...customProperties
        } of dto.Package.TimeSeriesData) {
            const eventDataSet: TimeSeries.EventDataSet[] = [];
            const tabularData: TimeSeries.TabularData[] = [];

            if (EventData?.DataSet) {
                for (let ed of EventData.DataSet) {
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

            if (TabularData) {
                for (let td of TabularData) {
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
                dataConfiguration: DataConfiguration
                    ? {
                          timeStamp: DataConfiguration.TimeStamp,
                          id: DataConfiguration.ID,
                      }
                    : undefined,
                eventData: {
                    dataSet: eventDataSet.length > 0 ? eventDataSet : undefined,
                    numberOfDataSet: EventData?.NumberOfDataSet,
                },
                tabularData: tabularData.length > 0 ? tabularData : undefined,
                customProperties: customProperties,
            });
        }

        const mapHeader = (): TimeSeries.Header | undefined => {
            if (!dto.Package.Header) return undefined;

            const {
                ShipID,
                TimeSpan,
                DateCreated,
                DateModified,
                Author,
                SystemConfiguration,
                ...custom
            } = dto.Package.Header;
            return {
                shipId: ShipId.parse(ShipID),
                timeSpan: TimeSpan
                    ? { end: TimeSpan.End, start: TimeSpan.Start }
                    : undefined,
                dateCreated: DateCreated,
                dateModified: DateModified,
                author: Author,
                systemConfiguration: SystemConfiguration?.map((c) => ({
                    id: c.ID,
                    timeStamp: c.TimeStamp,
                })),
                customHeaders: custom,
            };
        };

        return {
            package: {
                header: mapHeader(),
                timeSeriesData: timeSeriesData,
            },
        };
    }
}
