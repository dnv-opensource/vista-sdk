import { VisVersion, LocalIdBuilder, CodebookName } from "../lib";
import { LocalIdErrorBuilder } from "../lib/internal/LocalIdErrorBuilder";
import { VIS } from "../lib/VIS";

type Input = {
    primaryItem: string;
    secondaryItem?: string;
    quantity?: string;
    content?: string;
    position?: string;
    verbose: boolean;
};

describe("LocalId", () => {
    const vis = VIS.instance;
    const visVersion = VisVersion.v3_4a;
    const gmodPromise = vis.getGmod(visVersion);
    const codebooksPromise = vis.getCodebooks(visVersion);

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

        testData.forEach(({ input, output }) => {
            const primaryItem = gmod.parsePath(input.primaryItem);
            const secondaryItem = input.secondaryItem
                ? gmod.parsePath(input.secondaryItem)
                : undefined;

            const localId = LocalIdBuilder.create(visVersion)
                .withPrimaryItem(primaryItem)
                .withSecondaryItem(secondaryItem)
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

        testData.forEach(({ input, output }) => {
            const primaryItem = gmod.parsePath(input.primaryItem);
            const secondaryItem = input.secondaryItem
                ? gmod.parsePath(input.secondaryItem)
                : undefined;

            const localId = LocalIdBuilder.create(visVersion)
                .withPrimaryItem(primaryItem)
                .withSecondaryItem(secondaryItem)
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
                .withPrimaryItem(localId.primaryItem?.clone())
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
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta",
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        // "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
    ];

    test("LocalId parsing", async () => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;

        parseTestData.forEach((s) => {
            const errorBuilder = new LocalIdErrorBuilder();
            const localId = LocalIdBuilder.parse(
                s,
                gmod,
                codebooks,
                errorBuilder
            );

            expect(localId).toBeTruthy();
            expect(localId.toString()).toEqual(s);
        });
    });
});
