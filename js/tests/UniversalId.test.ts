import {
    LocalIdParsingErrorBuilder,
    ParsingState,
    UniversalId,
    UniversalIdBuilder,
    VIS,
    VisVersion,
} from "../lib";

describe("UniversalId", () => {
    const visVersion = VisVersion.v3_4a;
    const validTestData = [
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101.313-4/C469/meta/qty-temperature/state-high/pos-intake.side",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/411.1/C101/meta/qty-temperature",
    ];

    it("Valid parsing", async () => {
        const { gmod, codebooks, locations } = await VIS.instance.getVIS(
            visVersion
        );

        validTestData.forEach((testData) => {
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

    it("Invalid parsing", async () => {
        const { gmod, codebooks, locations } = await VIS.instance.getVIS(
            visVersion
        );

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

    const testCase = [
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate",
        "data.dnv.com/IMO1234567/dnv-v2/vis-3-7a/612.21/C701.23/C633/sec/411.1/C101/meta/calc~accumulate",
    ];
    it.each(testCase)(" parsing %s", async (s) => {
        const errors = new LocalIdParsingErrorBuilder();
        const universalId = await UniversalIdBuilder.tryParseAsync(s, errors);

        expect(universalId?.isValid).toEqual(true);
    });
});
