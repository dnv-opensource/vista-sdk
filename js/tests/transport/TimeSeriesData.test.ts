import { readFile } from "fs/promises";
import { validate } from "jsonschema";
import { DataChannelId, ShipId, TimeSeries } from "../../lib/transport/domain";

describe("TimeSeriesDataPackage", () => {
    const validLocalIdStr =
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature/cnt-cooling.water/pos-inlet";
    it("Model", async () => {
        // Values
        const dataChannelId = await DataChannelId.parseAsync(validLocalIdStr);
        const shipId = ShipId.parse("IMO1234567");
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
                                    dataChannelId: dataChannelId,
                                    timeStamp: now,
                                    value: value,
                                    quality: quality,
                                },
                            ],
                            numberOfDataSet: 1,
                        },
                        tabularData: [
                            {
                                dataChannelId: [dataChannelId],
                                dataSet: [
                                    {
                                        timeStamp: now,
                                        value: [value],
                                        quality: [quality],
                                    },
                                ],
                                numberOfDataChannel: 1,
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
                    shipId: shipId,
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
        expect(timeSeriesDataPackage.package.header!.shipId).toBe(shipId);

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
                .dataSet![0]!.dataChannelId
        ).toBe(dataChannelId);

        // TabularData
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .numberOfDataSet
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .numberOfDataChannel
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .dataChannelId![0]
        ).toBe(dataChannelId);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .dataSet![0]!.value.length
        ).toEqual(1);
        expect(
            timeSeriesDataPackage.package.timeSeriesData[0]!.tabularData![0]!
                .dataSet![0]!.value[0]
        ).toEqual(value);
    });

    it("JSONSchema validation", async () => {
        var sample = await readFile('../schemas/json/TimeSeriesData.sample.json', { encoding: 'utf8', flag: 'r' });
        var schema = await readFile('../schemas/json/TimeSeriesData.schema.json', { encoding: 'utf8', flag: 'r' });

        const result = validate(JSON.parse(sample), JSON.parse(schema));
        expect(result.errors).toHaveLength(0);
        expect(result.valid).toBe(true);
    });
});
