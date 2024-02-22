import { LocalId, LocalIdBuilder, VisVersion } from "../../../lib";
import { AssetIdentifier } from "../../../lib/experimental";
import { DataList } from "../../../lib/experimental/transport/domain/data-list/DataList";
import { Version } from "../../../lib/transport/domain/data-channel/Version";

describe("Data", () => {
    const validLocalIdStr =
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/meta/qty-temperature/cnt-cooling.water/pos-inlet";

    it("Model", async () => {
        const localId = await LocalId.parseAsync(validLocalIdStr);

        const shipId = AssetIdentifier.parse("IMO1234567");
        const now = new Date();
        const idStr = "test-id";
        const namingRule = LocalIdBuilder.namingRule;
        const version = Version.parse(VisVersion.v3_4a);

        const dataListPackage: DataList.DataListPackage = {
            package: {
                header: {
                    dataListId: {
                        id: idStr,
                        timestamp: now,
                        version: version,
                    },
                    dateCreated: now,
                    assetId: shipId,
                    author: "author",
                    versionInformation: {
                        namingRule: namingRule,
                        namingSchemeVersion: "naming-scheme-version",
                        referenceUrl: "www.referenceurl.test",
                    },
                },
                dataList: {
                    data: [
                        {
                            dataId: {
                                localId: localId,
                                shortId: "unique-id",
                                nameObject: { namingRule: namingRule },
                            },
                            property: {
                                dataType: {
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
                                        totalDigits: 1,
                                        whiteSpace: DataList.WhiteSpace.Preserve,
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
        expect(dataListPackage.package.header.dataListId.id).toEqual(idStr);
        expect(dataListPackage.package.header.assetId).toBe(shipId);
        expect(dataListPackage.package.header.dataListId.version).toBe(version);
        expect(
            dataListPackage.package.header.dataListId.version!.toString()
        ).toEqual(VisVersion.v3_4a);

        // Datas
        expect(dataListPackage.package.dataList.data).toHaveLength(1);
        expect(dataListPackage.package.dataList.data[0]!.dataId.localId).toBe(
            localId
        );
        expect(
            dataListPackage.package.dataList.data[0]!.dataId.nameObject!
                .namingRule
        ).toEqual(namingRule);
        expect(
            dataListPackage.package.dataList.data[0]!.property.format
        ).toBeTruthy();
        expect(
            dataListPackage.package.dataList.data[0]!.property.dataType.type
        ).toEqual("Inst");
    });
});
