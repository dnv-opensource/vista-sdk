import { LocalId } from "../../../lib";
import {
    AssetIdentifier,
    DataId,
    JSONExtensions,
    TimeSeries,
    VistaJSONSerializer,
} from "../../../lib/experimental";
import { DataList } from "../../../lib/experimental/transport/domain/data-list/DataList";

describe("Transport JSON", () => {
    test("DataChannelList Extensions", async () => {
        const p: DataList.DataListPackage = {
            package: {
                header: {
                    assetId: await AssetIdentifier.parse("IMO1234567"),
                    dataListId: { id: "some-id", timestamp: new Date() },
                },
                dataList: {
                    data: [
                        {
                            dataId: {
                                localId: await LocalId.parseAsync(
                                    "/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature/cnt-cooling.water/pos-inlet"
                                ),
                            },
                            property: {
                                dataType: { type: "" },
                                format: { type: "" },
                            },
                        },
                    ],
                },
            },
        };

        const dto = JSONExtensions.DataList.toJsonDto(p);
        const domain = await JSONExtensions.DataList.toDomainModel(dto);

        expect(domain.package.header.assetId.imoNumber!.value).toEqual(1234567);
        expect(dto.Package.Header.AssetId).toEqual("IMO1234567");
    });

    test("TimeSeries Extensions", async () => {
        const p: TimeSeries.TimeSeriesDataPackage = {
            package: {
                header: {
                    assetId: await AssetIdentifier.parse("IMO1234567"),
                },
                timeSeriesData: [
                    {
                        dataConfiguration: {
                            id: "DataChannelList.xml",
                            timeStamp: new Date(),
                        },
                        tabularData: [
                            {
                                dataId: [await DataId.parseAsync("0010")],
                                dataSet: [
                                    { timeStamp: new Date(), value: ["00"] },
                                ],
                            },
                        ],
                        eventData: {
                            dataSet: [
                                {
                                    dataId: await DataId.parseAsync("0010"),
                                    timeStamp: new Date(),
                                    value: "00",
                                },
                            ],
                        },
                    },
                ],
            },
        };

        const dto = JSONExtensions.TimeSeries.toJsonDto(p);
        const jsonStr = VistaJSONSerializer.serializeTimeSeriesData(dto);
        const json = VistaJSONSerializer.deserializeTimeSeriesData(jsonStr);
        const domain = await JSONExtensions.TimeSeries.toDomainModel(json);

        expect(domain.package.header!.assetId.imoNumber!.value).toEqual(
            1234567
        );
        expect(dto.Package.Header!.AssetId).toEqual("IMO1234567");
    });
});
