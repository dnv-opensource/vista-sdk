import { LocalId, LocalIdBuilder, VisVersion } from "../../lib";
import { DataChannelList, ShipId } from "../../lib/transport/domain";
import { Version } from "../../lib/transport/domain/data-channel/Version";
import { validate } from "jsonschema";
import { readFile } from "fs/promises";

describe("DataChannel", () => {
    const validLocalIdStr =
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature/cnt-cooling.water/pos-inlet";

    it("Model", async () => {
        const localId = await LocalId.parseAsync(validLocalIdStr);

        const shipId = ShipId.parse("IMO1234567");
        const now = new Date();
        const idStr = "test-id";
        const namingRule = LocalIdBuilder.namingRule;
        const version = Version.parse(VisVersion.v3_5a);

        const dataChannelListPackage: DataChannelList.DataChannelListPackage = {
            package: {
                header: {
                    dataChannelListId: {
                        id: idStr,
                        timestamp: now,
                        version: version,
                    },
                    dateCreated: now,
                    shipId: shipId,
                    author: "author",
                    versionInformation: {
                        namingRule: namingRule,
                        namingSchemeVersion: "naming-scheme-version",
                        referenceUrl: "www.referenceurl.test",
                    },
                },
                dataChannelList: {
                    dataChannel: [
                        {
                            dataChannelId: {
                                localId: localId,
                                shortId: "unique-id",
                                nameObject: { namingRule: namingRule },
                            },
                            property: {
                                dataChannelType: {
                                    type: "Inst",
                                    calculationPeriod: undefined,
                                    updateCycle: 1,
                                },
                                format: {
                                    type: "formatType",
                                    restriction: {
                                        enumeration: ["0", "1"],
                                        fractionDigits: 1,
                                        length: 1,
                                        maxExclusive: 1,
                                        minExclusive: 1,
                                        maxLength: 1,
                                        maxInclusive: 1,
                                        minInclusive: 1,
                                        minLength: 1,
                                        pattern: "pattern",
                                        totalDigits: 2,
                                        whiteSpace:
                                            DataChannelList.WhiteSpace.Collapse,
                                    },
                                },
                                alertPriority: "alertPriority",
                                name: "Cooling system / Propulsion Engine",
                                qualityCoding: "qualityCoding",
                                range: { high: 100, low: 0 },
                                remarks: "This is a remark",
                                unit: {
                                    quantityName: "celcius",
                                    unitSymbol: "C",
                                },
                            },
                        },
                    ],
                },
            },
        };

        // Header
        expect(
            dataChannelListPackage.package.header.dataChannelListId.id
        ).toEqual(idStr);
        expect(dataChannelListPackage.package.header.shipId).toBe(shipId);
        expect(
            dataChannelListPackage.package.header.dataChannelListId.version
        ).toBe(version);
        expect(
            dataChannelListPackage.package.header.dataChannelListId.version!.toString()
        ).toEqual(VisVersion.v3_5a);

        // DataChannels
        expect(
            dataChannelListPackage.package.dataChannelList.dataChannel
        ).toHaveLength(1);
        expect(
            dataChannelListPackage.package.dataChannelList.dataChannel[0]!
                .dataChannelId.localId
        ).toBe(localId);
        expect(
            dataChannelListPackage.package.dataChannelList.dataChannel[0]!
                .dataChannelId.nameObject!.namingRule
        ).toEqual(namingRule);
        expect(
            dataChannelListPackage.package.dataChannelList.dataChannel[0]!
                .property.format
        ).toBeTruthy();
        expect(
            dataChannelListPackage.package.dataChannelList.dataChannel[0]!
                .property.dataChannelType.type
        ).toEqual("Inst");
    });

    it("JSONSchema validation", async () => {
        var sample = await readFile('../schemas/json/DataChannelList.sample.json', { encoding: 'utf8', flag: 'r' });
        var schema = await readFile('../schemas/json/DataChannelList.schema.json', { encoding: 'utf8', flag: 'r' });

        const result = validate(JSON.parse(sample), JSON.parse(schema));
        expect(result.errors).toHaveLength(0);
        expect(result.valid).toBe(true);
    });
});
