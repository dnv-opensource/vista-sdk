import * as fs from "fs-extra";
import { JSONExtensions, JSONSerializer } from "../../lib";
import { Schemas } from "../fixtures";

describe("Transport JSON", () => {
    it("DataChannelList Extensions", async () => {
        const testDataPath = Schemas.DataChannelListSample;
        const sample = await fs
            .readFile(testDataPath)
            .then((res) => res.toString());

        const initDto = JSONSerializer.deserializeDataChannelList(sample);

        const domain =
            await JSONExtensions.DataChannelList.toDomainModel(initDto);

        const dto = JSONExtensions.DataChannelList.toJsonDto(domain);

        expect(domain.package.header.shipId.imoNumber!.value).toEqual(1234567);
        expect(dto.Package.Header.ShipID).toEqual("IMO1234567");
    });

    it("TimeSeries Extensions", async () => {
        const testDataPath = Schemas.TimeSeriesDataSample;
        const sample = await fs
            .readFile(testDataPath)
            .then((res) => res.toString());

        const initDto = JSONSerializer.deserializeTimeSeriesData(sample);

        const domain = await JSONExtensions.TimeSeries.toDomainModel(initDto);

        const dto = JSONExtensions.TimeSeries.toJsonDto(domain);

        expect(domain.package.header!.shipId.imoNumber!.value).toEqual(1234567);
        expect(dto.Package.Header!.ShipID).toEqual("IMO1234567");
    });
});
