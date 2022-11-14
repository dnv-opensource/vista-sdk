import {
    LocalIdParsingErrorBuilder,
    ParsingState,
    UniversalId,
    UniversalIdBuilder,
    VIS,
    VisVersion,
} from "../lib";

describe("UniversalId", () => {
    const vis = VIS.instance;
    const visVersion = VisVersion.v3_4a;
    const gmodPromise = vis.getGmod(visVersion);
    const codebooksPromise = vis.getCodebooks(visVersion);
    const locationsPromise = vis.getLocations(visVersion);

    const validTestData = [
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.313-4/C469/meta/qty-temperature/state-high/pos-intake.side",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101/meta/qty-temperature",
    ];

    test("Valid parsing", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        validTestData.forEach((testData) => {
            console.log(testData);
            const errorBuilder = new LocalIdParsingErrorBuilder();
            const universalId = UniversalId.parse(
                testData,
                gmod,
                codebooks,
                locations,
                errorBuilder
            );

            expect(universalId.imoNumber.value).toBe(1234567);
            expect(universalId.builder.localId?.isValid).toBe(true);
            expect(universalId.builder.isValid).toBe(true);
            expect(errorBuilder.errors.length).toBe(0);
        });
    });

    const invalidTestData = [
        {
            testString:
                "data.not.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            errorTypes: [ParsingState.NamingEntity],
            numErrors: 1,
            validLocalId: true,
        },
        {
            testString:
                "data.dnv.com/IMO1234568/dnv-v2/vis-3-4a/411.1/C101.313-4/C469/meta/qty-temperature/state-high/pos-intake.side",
            errorTypes: [ParsingState.IMONumber],
            numErrors: 1,
            validLocalId: true,
        },
        {
            testString:
                "data.not.dnv.com/IMO1234568/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
            errorTypes: [ParsingState.NamingEntity, ParsingState.IMONumber],
            numErrors: 2,
            validLocalId: true,
        },
    ];

    test("Invalid parsing", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        invalidTestData.forEach((data) => {
            const errorBuilder = LocalIdParsingErrorBuilder.create();

            const universalIdBuilder = UniversalIdBuilder.tryParse(
                data.testString,
                gmod,
                codebooks,
                locations,
                errorBuilder
            );

            expect(universalIdBuilder).toBeTruthy();
            expect(universalIdBuilder?.localId?.isValid).toBe(
                data.validLocalId
            );
            expect(errorBuilder.errors).toHaveLength(data.numErrors);
            expect(errorBuilder.errors.map((e) => e.type).sort()).toEqual(
                data.errorTypes.sort()
            );
        });
    });
});
