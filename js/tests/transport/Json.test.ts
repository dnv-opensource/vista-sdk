import * as fs from "fs-extra";
import { JSONExtensions } from "../../lib";
import { VistaJSONSerializer } from "../../lib";

describe("Transport JSON", () => {
    it("DataChannelList Extensions", async () => {
        const testDataPath = "../schemas/json/DataChannelList.sample.json";
        const sample = await fs
            .readFile(testDataPath)
            .then((res) => res.toString());

        const initDto = VistaJSONSerializer.deserializeDataChannelList(sample);

        const domain =
            await JSONExtensions.DataChannelList.toDomainModel(initDto);

        const dto = JSONExtensions.DataChannelList.toJsonDto(domain);

        expect(domain.package.header.shipId.imoNumber!.value).toEqual(1234567);
        expect(dto.Package.Header.ShipID).toEqual("IMO1234567");
    });

    it("TimeSeries Extensions", async () => {
        const testDataPath = "../schemas/json/TimeSeriesData.sample.json";
        const sample = await fs
            .readFile(testDataPath)
            .then((res) => res.toString());

        const initDto = VistaJSONSerializer.deserializeTimeSeriesData(sample);

        const domain = await JSONExtensions.TimeSeries.toDomainModel(initDto);

        const dto = JSONExtensions.TimeSeries.toJsonDto(domain);

        expect(domain.package.header!.shipId.imoNumber!.value).toEqual(1234567);
        expect(dto.Package.Header!.ShipID).toEqual("IMO1234567");
    });
});
