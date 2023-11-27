import { VisVersion, Experimental, CodebookName, VIS } from "../../lib";
import { DataId } from "../../lib/experimental";
import { PMSLocalId } from "../../lib/experimental/PMSLocalId";

type Input = {
    primaryItem: string;
    secondaryItem?: string;
    content?: string;
    mainenanceCategory?: string;
    activityType?: string;
    verbose: boolean;
};

describe("LocalId", () => {
    const vis = VIS.instance;
    const visVersion = VisVersion.v3_6a;
    const gmodPromise = vis.getGmod(visVersion);
    const codebooksPromise = vis.getCodebooks(visVersion);
    const locationsPromise = vis.getLocations(visVersion);

    const createInput = (
        primaryItem: string,
        secondaryItem?: string,
        content?: string,
        mainenanceCategory?: string,
        activityType?: string,
        verbose = false
    ): Input => {
        return {
            primaryItem,
            secondaryItem,
            mainenanceCategory,
            content,
            activityType,
            verbose,
        };
    };

    const testData: { input: Input; output: string }[] = [
        {
            input: createInput(
                "632.32i-1/S110",
                undefined,
                "high.temperature.fresh.water",
                "preventive",
                "overhaul"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul",
        },

        {
            input: createInput(
                "632.32i-2/S110",
                undefined,
                "high.temperature.fresh.water",
                "preventive",
                "test"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/632.32i-2/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-test",
        },
        {
            input: createInput(
                "632.32i-2/S110",
                undefined,
                "low.temperature.fresh.water",
                "preventive",
                "inspection"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/632.32i-2/S110/meta/cnt-low.temperature.fresh.water/maint.cat-preventive/act.type-inspection",
        },

        {
            input: createInput(
                "632.32i-1/S110",
                undefined,
                "low.temperature.fresh.water",
                "preventive",
                "inspection"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-low.temperature.fresh.water/maint.cat-preventive/act.type-inspection",
        },

        {
            input: createInput(
                "641.11i-2/C662",
                undefined,
                undefined,
                "preventive",
                "refit"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/641.11i-2/C662/meta/maint.cat-preventive/act.type-refit",
        },

        {
            input: createInput(
                "411.1/C101.661i-F/C621",
                undefined,
                undefined,
                "preventive",
                "service"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-F/C621/meta/maint.cat-preventive/act.type-service",
        },

        {
            input: createInput(
                "411.1/C101.661i-A/C621",
                undefined,
                undefined,
                "preventive",
                "service"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-A/C621/meta/maint.cat-preventive/act.type-service",
        },

        {
            input: createInput(
                "641.11i-2/C662",
                undefined,
                undefined,
                "preventive",
                "refit"
            ),
            output: "/dnv-v2-experimental/vis-3-6a/641.11i-2/C662/meta/maint.cat-preventive/act.type-refit",
        },
    ];

    test.each(testData)(
        "LocalId valid build for $output",
        async ({ input, output }) => {
            const gmod = await gmodPromise;
            const codebooks = await codebooksPromise;
            const locations = await locationsPromise;

            const primaryItem = gmod.parsePath(input.primaryItem, locations);
            const secondaryItem = input.secondaryItem
                ? gmod.parsePath(input.secondaryItem, locations)
                : undefined;

            const localId = Experimental.PMSLocalIdBuilder.create(visVersion)
                .withPrimaryItem(primaryItem)
                .tryWithSecondaryItem(secondaryItem)
                .withVerboseMode(input.verbose)
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.MaintenanceCategory,
                        input.mainenanceCategory
                    )
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(CodebookName.Content, input.content)
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.ActivityType,
                        input.activityType
                    )
                );

            const localIdStr = localId.toString();

            expect(localIdStr).toEqual(output);
        }
    );

    test.each(testData)("LocalId equality for $output", async ({ input }) => {
        const gmod = await gmodPromise;
        const codebooks = await codebooksPromise;
        const locations = await locationsPromise;

        const primaryItem = gmod.parsePath(input.primaryItem, locations);
        const secondaryItem = input.secondaryItem
            ? gmod.parsePath(input.secondaryItem, locations)
            : undefined;

        const localId = Experimental.PMSLocalIdBuilder.create(visVersion)
            .withPrimaryItem(primaryItem)
            .tryWithSecondaryItem(secondaryItem)
            .withVerboseMode(input.verbose)
            .tryWithMetadataTag(
                codebooks.tryCreateTag(
                    CodebookName.MaintenanceCategory,
                    input.mainenanceCategory
                )
            )
            .tryWithMetadataTag(
                codebooks.tryCreateTag(CodebookName.Content, input.content)
            )
            .tryWithMetadataTag(
                codebooks.tryCreateTag(
                    CodebookName.ActivityType,
                    input.activityType
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

    test.each(testData)(
        "Without MaintenanceCategory $output",
        async ({ input }) => {
            const gmod = await gmodPromise;
            const codebooks = await codebooksPromise;
            const locations = await locationsPromise;

            const primaryItem = gmod.parsePath(input.primaryItem, locations);
            const secondaryItem = input.secondaryItem
                ? gmod.parsePath(input.secondaryItem, locations)
                : undefined;

            let localId = Experimental.PMSLocalIdBuilder.create(visVersion)
                .withPrimaryItem(primaryItem)
                .tryWithSecondaryItem(secondaryItem)
                .withVerboseMode(input.verbose)
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.MaintenanceCategory,
                        input.mainenanceCategory
                    )
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(CodebookName.Content, input.content)
                )
                .tryWithMetadataTag(
                    codebooks.tryCreateTag(
                        CodebookName.ActivityType,
                        input.activityType
                    )
                );

            localId = localId.withoutMetadataTag(
                CodebookName.MaintenanceCategory
            );

            expect(localId.maintenanceCategory).toBeFalsy();
        }
    );

    test.each(testData)("Parsing", async ({ output }) => {
        const pmsLocalId = await PMSLocalId.parseAsync(output);

        expect(pmsLocalId).not.toBeFalsy();
    });

    const otherTestData = [
        "/dnv-v2-experimental/vis-3-7a/623/meta/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/623/sec/411/meta/cmd-activate/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/623/meta/state-abnormal/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/623/sec/411/meta/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/621.24i/C623/sec/411/meta/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/631.22i/S110.2/E31/meta/maint.cat-preventive/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/411.1-3/C101/meta/maint.cat-preventive/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/411.1-3/C101/meta/maint.cat-preventive/act.type-inspection",
        "/dnv-v2-experimental/vis-3-7a/411.1-2/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-3/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-2/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-3/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-3/C101.663i/C663/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-2/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-2/C101.663i/C663/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-2/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-3/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-1/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-1/C101.663i/C663/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-4/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/411.1-1/C101/meta/maint.cat-preventive/act.type-test",
        "/dnv-v2-experimental/vis-3-7a/511.15-1/E32/meta/maint.cat-preventive/act.type-test"
    ];

    test.each(otherTestData)("Parsing DataId - %s", async (localId) => {
        const pmsLocalId = await DataId.parseAsync(localId);

        expect(pmsLocalId).not.toBeFalsy();
    });

});
