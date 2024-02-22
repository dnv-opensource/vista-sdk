import { AssetIdentifier, DataId, TimeSeries } from "../../../lib/experimental";

describe("TimeSeriesDataPackage", () => {
    const validLocalIdStr =
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature/cnt-cooling.water/pos-inlet";
    it("Model", async () => {
        // Values
        const dataId = await DataId.parseAsync(validLocalIdStr);
        const shipId = AssetIdentifier.parse("IMO1234567");
        const value = "1";
        const quality = "quality";
        const now = new Date();
        const idStr = "test-id";

        const timeSeriesDataPackage: TimeSeries.TimeSeriesDataPackage = {
            package: {
                timeSeriesData: [
                    {
                        eventData: {
                            dataSet: [
                                {
                                    dataId: dataId,
                                    timeStamp: now,
                                    value: value,
                                    quality: quality,
                                },
                            ],
                            numberOfDataSet: 1,
                        },
                        tabularData: [
                            {
                                dataId: [dataId],
                                dataSet: [
                                    {
                                        timeStamp: now,
                                        value: [value],
                                        quality: [quality],
                                    },
                                ],
                                numberOfDataPoints: 1,
                                numberOfDataSet: 1,
                            },
                        ],
                        dataConfiguration: {
                            id: idStr,
                            timeStamp: now,
                        },
                    },
                ],
                header: {
                    assetId: shipId,
                    dateCreated: now,
                    dateModified: now,
                    systemConfiguration: [{ timeStamp: now, id: idStr }],
                    author: "author",
                    timeSpan: { end: now, start: now },
                },
            },
        };

        // Header
        expect(timeSeriesDataPackage.package.header!.dateCreated).toBe(now);
        expect(timeSeriesDataPackage.package.header!.assetId).toBe(shipId);

        // Data
        expect(timeSeriesDataPackage.package.timeSeriesData.length).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.dataConfiguration!
                .id
        ).toEqual(idStr);

        // EventData
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.eventData!
                .numberOfDataSet
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.eventData!
                .dataSet![0]!.value
        ).toEqual(value);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.eventData!
                .dataSet![0]!.dataId
        ).toBe(dataId);

        // TabularData
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .numberOfDataSet
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .numberOfDataPoints
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .dataId![0]
        ).toBe(dataId);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .dataSet![0]!.value.length
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .dataSet![0]!.value[0]
        ).toEqual(value);
    });
});
