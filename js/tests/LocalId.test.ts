import { VisVersion, LocalIdBuilder, CodebookName } from "../lib";
import { LocalIdParsingErrorBuilder } from "../lib/internal/LocalIdParsingErrorBuilder";
import { VIS } from "../lib/VIS";
import * as fs from "fs-extra";
import * as InvalidData from "../../testdata/InvalidLocalIds.json";
import readline from "readline";

type Input = {
    primaryItem: string;
    secondaryItem?: string;
    quantity?: string;
    content?: string;
    position?: string;
    verbose: boolean;
};

type Errored = {
    localIdStr: string;
    parsedLocalIdStr?: string;
    error?: any;
    errorBuilder?: LocalIdParsingErrorBuilder;
};

describe("LocalId", () => {
    const vis = VIS.instance;
    const visVersion = VisVersion.v3_4a;
    const gmodPromise = vis.getGmod(visVersion);
    const codebooksPromise = vis.getCodebooks(visVersion);
    const locationsPromise = vis.getLocations(visVersion);
    const testDataPath = "../testdata/LocalIds.txt";

    const createInput = (
        primaryItem: string,
        secondaryItem?: string,
        quantity?: string,
        content?: string,
        position?: string,
        verbose = false
    ): Input => {
        return {
            primaryItem,
            secondaryItem,
            quantity,
            content,
            position,
            verbose,
        };
    };

    const testData: { input: Input; output: string }[] = [
        {
            input: createInput("411.1/C101.31-2"),
            output: "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta",
        },
        {
            input: createInput(
                "411.1/C101.31-2",
                undefined,
                "temperature",
                "exhaust.gas",
                "inlet"
            ),
            output: "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        },
        {
            input: createInput(
                "411.1/C101.63/S206",
                "411.1/C101.31-5",
                "temperature",
                "exhaust.gas",
                "inlet",
                true
            ),
            output: "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        },
    ];

    test("LocalId valid build", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        testData.forEach(({ input, output }) => {
            const primaryItem = gmod.parsePath(input.primaryItem, locations);
            const secondaryItem = input.secondaryItem
                ? gmod.parsePath(input.secondaryItem, locations)
                : undefined;

            const localId = LocalIdBuilder.create(visVersion)
                .withPrimaryItem(primaryItem)
                .tryWithSecondaryItem(secondaryItem)
                .withVerboseMode(input.verbose)
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.Quantity,
                        input.quantity
                    )
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(CodebookName.Content, input.content)
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.Position,
                        input.position
                    )
                );
            const localIdStr = localId.toString();

            expect(localIdStr).toEqual(output);
        });
    });

    test("LocalId equality", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        testData.forEach(({ input, output }) => {
            const primaryItem = gmod.parsePath(input.primaryItem, locations);
            const secondaryItem = input.secondaryItem
                ? gmod.parsePath(input.secondaryItem, locations)
                : undefined;

            const localId = LocalIdBuilder.create(visVersion)
                .withPrimaryItem(primaryItem)
                .tryWithSecondaryItem(secondaryItem)
                .withVerboseMode(input.verbose)
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.Quantity,
                        input.quantity
                    )
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(CodebookName.Content, input.content)
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.Position,
                        input.position
                    )
                );

            let otherLocalId = localId;

            expect(localId).toEqual(otherLocalId);
            expect(localId.equals(otherLocalId)).toBe(true);
            expect(localId).toBe(otherLocalId);

            otherLocalId = localId.clone();
            expect(localId.equals(otherLocalId)).toBe(true);
            expect(localId).not.toBe(otherLocalId);

            otherLocalId = localId
                .tryWithPrimaryItem(localId.primaryItem?.clone())
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.Position,
                        localId.position?.value
                    )
                );
            expect(localId).toEqual(otherLocalId);
            expect(localId.equals(otherLocalId)).toBe(true);
            expect(localId).not.toBe(otherLocalId);
        });
    });

    const parseTestData = [
        "/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
        "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1/C101.313-4/C469/meta/qty-temperature/state-high/pos-intake.side",
    ];
    test("LocalId parsing", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        parseTestData.forEach((s) => {
            const errorBuilder = new LocalIdParsingErrorBuilder();
            const localId = LocalIdBuilder.parse(
                s,
                gmod,
                codebooks,
                locations,
                errorBuilder
            );

            expect(localId).toBeTruthy();
            expect(localId.toString()).toEqual(s);
        });
    });

    test("LocalId async parsing", async () => {
        for (const s of parseTestData) {
            const errorBuilder = new LocalIdParsingErrorBuilder();
            const localId = await LocalIdBuilder.parseAsync(s, errorBuilder);

            expect(localId).toBeTruthy();
            expect(localId.toString()).toEqual(s);
        }
    });

    const invalidParseTestData: string =
        "/dnv-v2/vis-3-4a/1021.1i-3AC/H121/meta/qty-temperature/cnt-cargo/cal";
    test("LocalId invalid parsing", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        const errorBuilder = new LocalIdParsingErrorBuilder();
        const localId = LocalIdBuilder.tryParse(
            invalidParseTestData,
            gmod,
            codebooks,
            locations,
            errorBuilder
        );
        expect(errorBuilder.hasError).toBeTruthy();
        expect(localId).toBeUndefined();
    });

    test.skip("LocalId smoketest parsing", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        const fileStream = fs.createReadStream(testDataPath);
        const rl = readline.createInterface({
            input: fileStream,
            crlfDelay: Infinity,
        });

        const errored: Errored[] = [];
        for await (let localIdStr of rl) {
            try {
                const errorBuilder = new LocalIdParsingErrorBuilder();
                if (
                    localIdStr.includes("qty-content") ||
                    localIdStr.includes(
                        "/dnv-v2/vis-3-4a/621.1/sec/625.21/C823/meta/qty-level/cnt-marine.gas.oil/state-low"
                    )
                )
                    continue;
                const localId = LocalIdBuilder.tryParse(
                    localIdStr,
                    gmod,
                    codebooks,
                    locations,
                    errorBuilder
                );
                const parsedLocalIdStr = localId?.toString();

                if (localId?.isEmpty || !localId?.isValid()) {
                    errored.push({
                        localIdStr,
                        parsedLocalIdStr,
                        error: "Not valid or Empty",
                        errorBuilder: errorBuilder,
                    });
                }
                // expect(parsedLocalIdStr).toEqual(localIdStr);
                // expect(localId).toBeTruthy();
            } catch (error) {
                // Quick fix to skip invalid location e.g. primaryItem 511.11-1SO
                if (
                    error instanceof Error &&
                    error.message.includes("location")
                )
                    continue;
                errored.push({ localIdStr, error });
            }
        }
        errored.length > 0 &&
            console.warn("Number of errors in dataset:", errored.length);
    });

    test.each(
        InvalidData.InvalidLocalIds.map((l) => [
            l.input,
            l.expectedErrorMessages,
        ])
    )(
        "LocalId parsing validation - %s",
        async (input, expectedErrorMessages) => {
            const gmod = await gmodPromise;
            const codeBooks = await codebooksPromise;
            const locations = await locationsPromise;

            const errorBuilder = new LocalIdParsingErrorBuilder();
            const localId = LocalIdBuilder.tryParse(
                input as string,
                gmod,
                codeBooks,
                locations,
                errorBuilder
            );

            expect(localId).toBeUndefined();
            expect(errorBuilder.errors.map((e) => e.message)).toEqual(
                expectedErrorMessages
            );
        }
    );

    test("LocalId Metadata Equality", async () => {
        const t1 = await LocalIdBuilder.parseAsync(
            "/dnv-v2/vis-3-4a/087/meta/qty-time/detail-one.more"
        );
        const t2 = await LocalIdBuilder.parseAsync(
            "/dnv-v2/vis-3-4a/087/meta/qty-time"
        );

        expect(t1.equals(t2)).toEqual(false);
        expect(t2.equals(t1)).toEqual(false);
    });
});
