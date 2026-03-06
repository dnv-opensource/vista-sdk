import * as fs from "fs";
import {
    CodebookName,
    GmodPath,
    GmodPathQueryBuilder,
    LocalId,
    LocalIdQueryBuilder,
    MetadataTagsQueryBuilder,
    VIS,
    VisVersion,
} from "../lib";
import { Schemas, TestData } from "./fixtures";

// ---------------------------------------------------------------------------
// MetadataTagsQuery
// ---------------------------------------------------------------------------

describe("MetadataTagsQuery", () => {
    it("should match localId with subset tag", async () => {
        const localId = await LocalId.parseAsync(
            "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        );
        const query = MetadataTagsQueryBuilder.empty()
            .withTag(CodebookName.Content, "sea.water")
            .build();
        expect(query.match(localId)).toBe(true);
    });

    it("should not match with wrong tag value", async () => {
        const localId = await LocalId.parseAsync(
            "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        );
        const query = MetadataTagsQueryBuilder.empty()
            .withTag(CodebookName.Content, "heavy.fuel.oil")
            .build();
        expect(query.match(localId)).toBe(false);
    });

    it("empty query should match any localId (subset mode)", async () => {
        const localId = await LocalId.parseAsync(
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        );
        const query = MetadataTagsQueryBuilder.empty().build();
        expect(query.match(localId)).toBe(true);
    });

    it("exact mode with matching tags", async () => {
        const localId = await LocalId.parseAsync(
            "/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative",
        );
        const query = MetadataTagsQueryBuilder.from(localId, false).build();
        expect(query.match(localId)).toBe(true);
    });

    it("exact mode rejects extra tags", async () => {
        const localId = await LocalId.parseAsync(
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        );
        // Only specify one tag but require exact match
        const query = MetadataTagsQueryBuilder.empty()
            .withTag(CodebookName.Quantity, "temperature")
            .withAllowOtherTags(false)
            .build();
        expect(query.match(localId)).toBe(false);
    });
});

// ---------------------------------------------------------------------------
// GmodPathQuery — Path builder
// ---------------------------------------------------------------------------

describe("GmodPathQuery", () => {
    describe("Path builder", () => {
        const pathData = [
            {
                path: "411.1-1/C101",
                visVersion: VisVersion.v3_4a,
                params: [] as { node: string; locations: string[] | null }[],
                expected: true,
            },
            {
                path: "411.1-1/C101",
                visVersion: VisVersion.v3_4a,
                params: [{ node: "411.1", locations: ["1"] }],
                expected: true,
            },
            {
                path: "411.1-1/C101",
                visVersion: VisVersion.v3_4a,
                params: [{ node: "411.1", locations: ["A"] }],
                expected: false,
            },
            {
                path: "433.1-P/C322.31/C173",
                visVersion: VisVersion.v3_4a,
                params: [{ node: "C322.31", locations: null }],
                expected: true,
            },
            {
                path: "433.1-P/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [
                    { node: "433.1", locations: ["P"] },
                    { node: "C322.31", locations: null },
                ],
                expected: true,
            },
            {
                path: "433.1-P/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [
                    { node: "433.1", locations: ["A"] },
                    { node: "C322.31", locations: null },
                ],
                expected: false,
            },
            {
                path: "433.1-P/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [
                    { node: "433.1", locations: ["P"] },
                    { node: "C322.31", locations: ["1"] },
                ],
                expected: false,
            },
            {
                path: "433.1-A/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [
                    { node: "433.1", locations: ["P"] },
                    { node: "C322.31", locations: ["1"] },
                ],
                expected: false,
            },
            {
                path: "433.1-A/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [
                    { node: "433.1", locations: null },
                    { node: "C322.31", locations: null },
                ],
                expected: true,
            },
            {
                path: "433.1/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [{ node: "433.1", locations: ["A"] }],
                expected: false,
            },
            {
                path: "433.1/C322.31-2/C173",
                visVersion: VisVersion.v3_4a,
                params: [{ node: "433.1", locations: [] }],
                expected: true,
            },
        ];

        it.each(pathData)(
            "Path builder: $path ($expected)",
            async ({ path: pathStr, visVersion, params, expected }) => {
                const { gmod, locations } =
                    await VIS.instance.getVIS(visVersion);
                const gmodPath = gmod.parsePath(pathStr, locations);

                // Self-match without extra params should always be true
                const selfQuery = GmodPathQueryBuilder.from(gmodPath).build();
                expect(await selfQuery.match(gmodPath)).toBe(true);

                let builder = GmodPathQueryBuilder.from(gmodPath);
                for (const p of params) {
                    if (p.locations === null || p.locations.length === 0) {
                        builder = builder.withNode(
                            (nodes) => nodes.get(p.node)!,
                            true,
                        );
                    } else {
                        const locs = p.locations.map((l) => locations.parse(l));
                        builder = builder.withNode(
                            (nodes) => nodes.get(p.node)!,
                            ...locs,
                        );
                    }
                }
                const query = builder.build();
                expect(await query.match(gmodPath)).toBe(expected);
            },
        );
    });

    describe("Nodes builder", () => {
        const nodesData = [
            {
                path: "411.1-1/C101",
                visVersion: VisVersion.v3_4a,
                params: [{ node: "411.1", locations: ["1"] }],
                expected: true,
            },
            {
                path: "411.1-1/C101.61/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "411.1", locations: ["1"] }],
                expected: true,
            },
            {
                path: "411.1/C101.61-1/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "C101.61", locations: ["1"] }],
                expected: true,
            },
            {
                path: "511.11/C101.61-1/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "C101.61", locations: ["1"] }],
                expected: true,
            },
            {
                path: "411.1/C101.61-1/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "C101.61", locations: null }],
                expected: true,
            },
            {
                path: "511.11/C101.61-1/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "C101.61", locations: null }],
                expected: true,
            },
            {
                path: "221.11/C1141.421/C1051.7/C101.61-2/S203",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "C101.61", locations: null }],
                expected: true,
            },
            {
                path: "411.1/C101.61-1/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [
                    { node: "411.1", locations: null },
                    { node: "C101.61", locations: null },
                ],
                expected: true,
            },
            {
                path: "511.11/C101.61-1/S203.3/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [
                    { node: "411.1", locations: null },
                    { node: "C101.61", locations: null },
                ],
                expected: false,
            },
            {
                path: "411.1/C101.61/S203.3-1/S110.2/C101",
                visVersion: VisVersion.v3_7a,
                params: [{ node: "S203.3", locations: ["1"] }],
                expected: true,
            },
        ];

        it.each(nodesData)(
            "Nodes builder: $path ($expected)",
            async ({ path: pathStr, visVersion, params, expected }) => {
                const { gmod, locations } =
                    await VIS.instance.getVIS(visVersion);
                const gmodPath = gmod.parsePath(pathStr, locations);

                let builder = GmodPathQueryBuilder.empty();
                for (const p of params) {
                    const node = gmod.getNode(p.node);
                    if (p.locations === null || p.locations.length === 0) {
                        builder = builder.withNode(node, true);
                    } else {
                        const locs = p.locations.map((l) => locations.parse(l));
                        builder = builder.withNode(node, ...locs);
                    }
                }
                const query = builder.build();
                expect(await query.match(gmodPath)).toBe(expected);
            },
        );
    });

    describe("WithAnyNodeBefore", () => {
        it("should match paths with different parents", async () => {
            const { gmod, locations } = await VIS.instance.getVIS(
                VisVersion.v3_9a,
            );
            const basePath = gmod.parsePath("411.1/C101.31", locations);

            const query = GmodPathQueryBuilder.from(basePath)
                .withAnyNodeBefore((nodes) => nodes.get("C101")!)
                .withoutLocations()
                .build();

            expect(await query.match(basePath)).toBe(true);

            const pathDifferentParent = gmod.parsePath(
                "511.11/C101.31",
                locations,
            );
            expect(await query.match(pathDifferentParent)).toBe(true);

            const pathWithLocation = gmod.parsePath(
                "411.1/C101.31-2",
                locations,
            );
            expect(await query.match(pathWithLocation)).toBe(true);

            const pathDifferentCNode = gmod.parsePath(
                "411.1/C102.31",
                locations,
            );
            expect(await query.match(pathDifferentCNode)).toBe(false);

            const pathWithoutC101 = gmod.parsePath(
                "411.1/C101.61/S203",
                locations,
            );
            expect(await query.match(pathWithoutC101)).toBe(false);
        });

        it("should work with locations on set nodes", async () => {
            const { gmod, locations } = await VIS.instance.getVIS(
                VisVersion.v3_4a,
            );
            const basePath = gmod.parsePath("433.1-P/C322.31", locations);

            const query = GmodPathQueryBuilder.from(basePath)
                .withAnyNodeBefore((nodes) => nodes.get("C322")!)
                .build();

            expect(await query.match(basePath)).toBe(true);

            const pathDifferentParent = gmod.parsePath(
                "433.1-S/C322.31",
                locations,
            );
            expect(await query.match(pathDifferentParent)).toBe(true);

            const pathNoParentLocation = gmod.parsePath(
                "433.1/C322.31",
                locations,
            );
            expect(await query.match(pathNoParentLocation)).toBe(true);
        });

        it("S206 sensor matching", async () => {
            const { gmod, locations } = await VIS.instance.getVIS(
                VisVersion.v3_4a,
            );
            const sensorPath = gmod.parsePath("411.1/C101.63/S206", locations);
            const query = GmodPathQueryBuilder.from(sensorPath)
                .withoutLocations()
                .withAnyNodeBefore((nodes) => nodes.get("S206")!)
                .build();

            // Should match paths WITH S206
            expect(
                await query.match(
                    gmod.parsePath("411.1-1/C101.63/S206", locations),
                ),
            ).toBe(true);
            expect(
                await query.match(
                    gmod.parsePath("411.1-2/C101.63/S206", locations),
                ),
            ).toBe(true);

            // Should NOT match paths WITHOUT S206
            expect(
                await query.match(gmod.parsePath("411.1/C101.31-2", locations)),
            ).toBe(false);
            expect(
                await query.match(gmod.parsePath("411.1/C101.31-5", locations)),
            ).toBe(false);

            // Different sensor type
            expect(
                await query.match(
                    gmod.parsePath("411.1/C101.61/S203", locations),
                ),
            ).toBe(false);
        });
    });

    describe("WithAnyNodeAfter", () => {
        it("prefix matching", async () => {
            const { gmod, locations } = await VIS.instance.getVIS(
                VisVersion.v3_4a,
            );
            const basePath = gmod.parsePath("411.1/C101.31", locations);
            const query = GmodPathQueryBuilder.from(basePath)
                .withoutLocations()
                .withAnyNodeAfter((nodes) => nodes.get("411.1")!)
                .build();

            // Paths starting with 411.1 - should match
            expect(
                await query.match(gmod.parsePath("411.1/C101.31-2", locations)),
            ).toBe(true);
            expect(
                await query.match(
                    gmod.parsePath("411.1/C101.63/S206", locations),
                ),
            ).toBe(true);
            expect(
                await query.match(
                    gmod.parsePath("411.1-1/C101.61/S203", locations),
                ),
            ).toBe(true);
            expect(
                await query.match(gmod.parsePath("411.1-1/C102", locations)),
            ).toBe(true);

            // Paths NOT starting with 411.1 - should NOT match
            expect(await query.match(gmod.parsePath("411i", locations))).toBe(
                false,
            );
            expect(
                await query.match(
                    gmod.parsePath("511.11/C101.63/S206", locations),
                ),
            ).toBe(false);
            expect(
                await query.match(
                    gmod.parsePath("652.31/S90.3/S61", locations),
                ),
            ).toBe(false);
        });

        it("mid-path matching", async () => {
            const { gmod, locations } = await VIS.instance.getVIS(
                VisVersion.v3_4a,
            );
            const basePath = gmod.parsePath("411.1/C101.63/S206", locations);
            const query = GmodPathQueryBuilder.from(basePath)
                .withoutLocations()
                .withAnyNodeAfter((nodes) => nodes.get("C101.6")!)
                .build();

            // Paths with 411.1/C101.6/* - should match
            expect(
                await query.match(
                    gmod.parsePath("411.1/C101.63/S206", locations),
                ),
            ).toBe(true);
            expect(
                await query.match(
                    gmod.parsePath("411.1-1/C101.61/S203", locations),
                ),
            ).toBe(true);

            // Paths with different C-node - should NOT match
            expect(
                await query.match(gmod.parsePath("411.1/C101.31-2", locations)),
            ).toBe(false);
            expect(
                await query.match(gmod.parsePath("411.1/C101", locations)),
            ).toBe(false);
        });
    });
});

// ---------------------------------------------------------------------------
// LocalIdQuery
// ---------------------------------------------------------------------------

describe("LocalIdQuery", () => {
    describe("Parameterized match tests", () => {
        it("primary item without locations", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
            );
            const query = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromQuery(
                    GmodPathQueryBuilder.from(
                        await GmodPath.parseAsync(
                            "1021.1i-6P/H123",
                            VisVersion.v3_4a,
                        ),
                    )
                        .withoutLocations()
                        .build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);
        });

        it("match tag content", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
            );
            const query = LocalIdQueryBuilder.empty()
                .withTagsFromQuery(
                    MetadataTagsQueryBuilder.empty()
                        .withTag(CodebookName.Content, "sea.water")
                        .build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);
        });

        it("primary item with different location should not match", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            );
            const query = LocalIdQueryBuilder.empty()
                .withPrimaryItem(
                    await GmodPath.parseAsync(
                        "411.1/C101.31-1",
                        VisVersion.v3_4a,
                    ),
                )
                .build();
            expect(await query.match(localId)).toBe(false);
        });

        it("primary item without locations matches any location", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            );
            const query = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromQuery(
                    GmodPathQueryBuilder.from(
                        await GmodPath.parseAsync(
                            "411.1-2/C101.63/S206",
                            VisVersion.v3_4a,
                        ),
                    )
                        .withoutLocations()
                        .build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);
        });

        it("secondary item without locations", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            );
            const query = LocalIdQueryBuilder.empty()
                .withSecondaryItemFromQuery(
                    GmodPathQueryBuilder.from(
                        await GmodPath.parseAsync(
                            "411.1/C101.31-2",
                            VisVersion.v3_4a,
                        ),
                    )
                        .withoutLocations()
                        .build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);
        });

        it("primary item mismatch", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low",
            );
            const query = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromQuery(
                    GmodPathQueryBuilder.from(
                        await GmodPath.parseAsync("411.1", VisVersion.v3_4a),
                    )
                        .withoutLocations()
                        .build(),
                )
                .build();
            expect(await query.match(localId)).toBe(false);
        });

        it("from string with path builder configuring location", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative",
            );
            const query = (
                await LocalIdQueryBuilder.fromStr(
                    "/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity",
                )
            )
                .withPrimaryItemReconfigured((builder) =>
                    builder
                        .withNode((nodes) => nodes.get("433.1")!, true)
                        .build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);
        });
    });

    describe("Happy path", () => {
        it("should match all combinations", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo/state-high.high/pos-stage-3/detail-discharge",
            );
            const primaryItem = localId.primaryItem;
            const secondaryItem = localId.secondaryItem!;

            // Match exact
            const exactQuery = LocalIdQueryBuilder.from(localId).build();
            expect(await exactQuery.match(localId)).toBe(true);

            // Test both individualized and not
            for (const individualized of [true, false]) {
                let primaryQueryBuilder =
                    GmodPathQueryBuilder.from(primaryItem);
                let secondaryQueryBuilder =
                    GmodPathQueryBuilder.from(secondaryItem);

                if (!individualized) {
                    primaryQueryBuilder =
                        primaryQueryBuilder.withoutLocations();
                    secondaryQueryBuilder =
                        secondaryQueryBuilder.withoutLocations();
                }

                const primaryQuery = primaryQueryBuilder.build();
                const secondaryQuery = secondaryQueryBuilder.build();

                // Match primary
                let q = LocalIdQueryBuilder.empty()
                    .withPrimaryItemFromQuery(primaryQuery)
                    .build();
                expect(await q.match(localId)).toBe(true);

                // Match secondary
                q = LocalIdQueryBuilder.empty()
                    .withSecondaryItemFromQuery(secondaryQuery)
                    .build();
                expect(await q.match(localId)).toBe(true);

                // Match tags
                let tagBuilder = LocalIdQueryBuilder.empty();
                for (const tag of localId.metadataTags) {
                    tagBuilder = tagBuilder.withTags((tags) =>
                        tags.withTag(tag).build(),
                    );
                }
                q = tagBuilder.build();
                expect(await q.match(localId)).toBe(true);

                // Match primary and secondary
                q = LocalIdQueryBuilder.empty()
                    .withPrimaryItemFromQuery(primaryQuery)
                    .withSecondaryItemFromQuery(secondaryQuery)
                    .build();
                expect(await q.match(localId)).toBe(true);

                // Match primary and tags
                tagBuilder =
                    LocalIdQueryBuilder.empty().withPrimaryItemFromQuery(
                        primaryQuery,
                    );
                for (const tag of localId.metadataTags) {
                    tagBuilder = tagBuilder.withTags((tags) =>
                        tags.withTag(tag).build(),
                    );
                }
                q = tagBuilder.build();
                expect(await q.match(localId)).toBe(true);

                // Match secondary and tags
                tagBuilder =
                    LocalIdQueryBuilder.empty().withSecondaryItemFromQuery(
                        secondaryQuery,
                    );
                for (const tag of localId.metadataTags) {
                    tagBuilder = tagBuilder.withTags((tags) =>
                        tags.withTag(tag).build(),
                    );
                }
                q = tagBuilder.build();
                expect(await q.match(localId)).toBe(true);

                // Match primary, secondary, and tags
                tagBuilder = LocalIdQueryBuilder.empty()
                    .withPrimaryItemFromQuery(primaryQuery)
                    .withSecondaryItemFromQuery(secondaryQuery);
                for (const tag of localId.metadataTags) {
                    tagBuilder = tagBuilder.withTags((tags) =>
                        tags.withTag(tag).build(),
                    );
                }
                q = tagBuilder.build();
                expect(await q.match(localId)).toBe(true);
            }
        });
    });

    describe("MatchAll", () => {
        it("empty query matches any valid LocalId", async () => {
            const query = LocalIdQueryBuilder.empty().build();
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            );
            expect(await query.match(localId)).toBe(true);
        });
    });

    describe("Variations", () => {
        it("should handle location mismatch and withoutLocations", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo/state-high.high/pos-stage-3/detail-discharge",
            );

            const primaryItem = await GmodPath.parseAsync(
                "1036.13i-2/C662",
                VisVersion.v3_4a,
            );

            // Different location => no match
            let query = LocalIdQueryBuilder.empty()
                .withPrimaryItem(primaryItem)
                .build();
            expect(await query.match(localId)).toBe(false);

            // WithoutLocations => match
            query = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromPath(primaryItem, (b) =>
                    b.withoutLocations().build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);

            // Without location on path itself
            const primaryItemNoLoc = primaryItem.withoutLocation();
            query = LocalIdQueryBuilder.empty()
                .withPrimaryItem(primaryItemNoLoc)
                .build();
            expect(await query.match(localId)).toBe(false);

            query = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromPath(primaryItemNoLoc, (b) =>
                    b.withoutLocations().build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);
        });
    });

    describe("Samples", () => {
        const sampleIds = [
            "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature/state-high",
            "/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature/state-high",
            "/dnv-v2/vis-3-4a/412.723-A/C261/sec/411.1/C101/meta/qty-temperature/state-high/cmd-slow.down",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-high",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-running",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-failure",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-start",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-stop",
            "/dnv-v2/vis-3-4a/623.22i-1/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/cnt-lubricating.oil",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-remote.control",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-running",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-failure",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-start",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-stop",
            "/dnv-v2/vis-3-4a/623.22i-2/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/cnt-lubricating.oil",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-remote.control",
            "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-stand.by/cmd-start",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-control.location",
            "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/detail-stand.by.start.or.power.failure",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-high",
            "/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-high",
            "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/412.723-A/CS6d/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/411.1/C101.64i-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running",
        ];

        it.each(sampleIds)("self-match: %s", async (localIdStr) => {
            const localId = await LocalId.parseAsync(localIdStr);
            expect(localId).toBeTruthy();

            // From should self-match
            const fromQuery = LocalIdQueryBuilder.from(localId).build();
            expect(await fromQuery.match(localId)).toBe(true);

            // Empty should match anything
            const emptyQuery = LocalIdQueryBuilder.empty().build();
            expect(await emptyQuery.match(localId)).toBe(true);
        });
    });

    describe("Consistency smoke test", () => {
        it("all LocalIds.txt entries should self-match", async () => {
            const filePath = TestData.LocalIdsPath;
            const content = fs.readFileSync(filePath, "utf-8");
            const lines = content
                .split("\n")
                .map((l) => l.trim())
                .filter((l) => l.length > 0);

            const errored: { path: string; error?: string }[] = [];
            const conversionErrors: { path: string; error?: string }[] = [];

            for (const line of lines) {
                try {
                    const localId = await LocalId.parseAsync(line);
                    const query = LocalIdQueryBuilder.from(localId).build();
                    const match = await query.match(localId);
                    if (!match) {
                        errored.push({ path: line });
                    }
                } catch (e: any) {
                    const msg = e?.message ?? String(e);
                    // Known conversion limitations in GmodVersioning
                    if (
                        msg.includes("Uncovered case") ||
                        msg.includes("Failed to convert")
                    ) {
                        conversionErrors.push({ path: line, error: msg });
                    } else {
                        errored.push({ path: line, error: msg });
                    }
                }
            }

            if (conversionErrors.length > 0) {
                console.log(
                    `Skipped ${conversionErrors.length} entries with known conversion limitations`,
                );
            }

            if (errored.length > 0) {
                for (const { path: p, error } of errored) {
                    console.log(`Failed on ${p}`);
                    if (error) console.log(error);
                }
            }
            expect(errored).toHaveLength(0);
        });
    });

    describe("UnspecifiedSecondary", () => {
        it("should handle secondary item modes", async () => {
            const baseLocalId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power",
            );
            const otherLocalId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power",
            );

            const queryBuilder = LocalIdQueryBuilder.from(baseLocalId);

            // Default from() - secondary forbidden since base has none
            let query = queryBuilder.build();
            expect(await query.match(baseLocalId)).toBe(true);
            expect(await query.match(otherLocalId)).toBe(false);

            // Explicitly no secondary
            query = queryBuilder.withoutSecondaryItem().build();
            expect(await query.match(otherLocalId)).toBe(false);

            // Any secondary
            query = queryBuilder.withAnySecondaryItem().build();
            expect(await query.match(otherLocalId)).toBe(true);
        });
    });

    describe("WithAnyNodeBefore", () => {
        it("should match paths with any parent before specified node", async () => {
            const { gmod, locations, codebooks } = await VIS.instance.getVIS(
                VisVersion.v3_9a,
            );

            const basePath = gmod.parsePath("411.1/C101.31", locations);
            const baseLocalId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power",
            );

            const specificQuery = LocalIdQueryBuilder.from(baseLocalId)
                .withPrimaryItemReconfigured((p) =>
                    p.withAnyNodeBefore((nodes) => nodes.get("C101")!).build(),
                )
                .build();

            const generalQuery = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromPath(basePath, (p) =>
                    p.withAnyNodeBefore((nodes) => nodes.get("C101")!).build(),
                )
                .withTags((tags) =>
                    tags
                        .withTag(
                            codebooks.createTag(CodebookName.Quantity, "power"),
                        )
                        .build(),
                )
                .build();

            // Base - should match
            expect(await generalQuery.match(baseLocalId)).toBe(true);

            const l2 = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power",
            );
            expect(await specificQuery.match(l2)).toBe(true);
            expect(await generalQuery.match(l2)).toBe(true);

            const l3 = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power",
            );
            expect(await specificQuery.match(l3)).toBe(false); // No sec node
            expect(await generalQuery.match(l3)).toBe(true);

            const l4 = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C102.31/meta/qty-power",
            );
            expect(await generalQuery.match(l4)).toBe(false); // Different C-node
            expect(await specificQuery.match(l4)).toBe(false);

            const l5 = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.2/meta/qty-power",
            );
            expect(await specificQuery.match(l5)).toBe(false); // Different sec
            expect(await generalQuery.match(l5)).toBe(true);

            const l6 = await LocalId.parseAsync(
                "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.2/meta/qty-pressure",
            );
            expect(await specificQuery.match(l6)).toBe(false); // Different quantity
            expect(await generalQuery.match(l6)).toBe(false);
        });
    });

    describe("Use cases", () => {
        it("Use Case 1: match multiple locations", async () => {
            const locations = await VIS.instance.getLocations(
                VIS.latestVisVersion,
            );
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-7a/433.1-P/C322/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816",
            );
            const query = LocalIdQueryBuilder.from(localId)
                .withPrimaryItemReconfigured((builder) =>
                    builder
                        .withNode(
                            (nodes) => nodes.get("433.1")!,
                            locations.parse("P"),
                            locations.parse("S"),
                        )
                        .build(),
                )
                .build();

            expect(await query.match(localId)).toBe(true);
            expect(
                await query.matchStr(
                    "/dnv-v2/vis-3-7a/433.1-S/C322/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816",
                ),
            ).toBe(true);
        });

        it("Use Case 2: match node hierarchy", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-7a/511.31/C121/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816",
            );
            const gmod = await VIS.instance.getGmod(localId.visVersion);

            // Match all Wind turbine arrangements (uses NodesBuilder from scratch)
            let query = LocalIdQueryBuilder.from(localId)
                .withPrimaryItemFromNodes((builder) =>
                    builder.withNode(gmod.getNode("511.3"), true).build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);

            // Should not match Solar panel arrangements
            query = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromNodes((builder) =>
                    builder.withNode(gmod.getNode("511.4"), true).build(),
                )
                .build();
            expect(await query.match(localId)).toBe(false);
        });

        it("Use Case 3: exact tag matching", async () => {
            const localId = await LocalId.parseAsync(
                "/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative",
            );
            const visVersion = localId.visVersion;

            const query = LocalIdQueryBuilder.from(localId)
                .withTags((builder) =>
                    builder.withAllowOtherTags(false).build(),
                )
                .build();
            expect(await query.match(localId)).toBe(true);

            const { gmod, codebooks, locations } =
                await VIS.instance.getVIS(visVersion);

            // Extra tag -> should not match
            const l1 = localId.builder
                .withMetadataTag(
                    codebooks.createTag(CodebookName.Content, "random"),
                )
                .build();
            expect(await query.match(l1)).toBe(false);

            // Different primary -> should not match
            const l2 = localId.builder
                .withPrimaryItem(gmod.parsePath("433.1-1S", locations))
                .build();
            expect(await query.match(l2)).toBe(false);
        });

        it("Use Case 4: location-agnostic matching", async () => {
            const query = (
                await LocalIdQueryBuilder.fromStr(
                    "/dnv-v2/vis-3-7a/511.11/C101/meta/qty-pressure/cnt-lubricating.oil",
                )
            )
                .withPrimaryItemReconfigured((builder) =>
                    builder.withoutLocations().build(),
                )
                .build();
            const other = await LocalId.parseAsync(
                "/dnv-v2/vis-3-7a/511.11-1/C101/meta/qty-pressure/cnt-lubricating.oil",
            );
            expect(await query.match(other)).toBe(true);
        });
    });

    // -----------------------------------------------------------------------
    // DataChannelList filter
    // -----------------------------------------------------------------------

    describe("DataChannelList filter", () => {
        it("filters data channels with five queries", async () => {
            const { VistaJSONSerializer, JSONExtensions } =
                await import("../lib");

            const testDataPath = Schemas.DataChannelListSample;
            const sample = fs.readFileSync(testDataPath).toString();

            const initDto =
                VistaJSONSerializer.deserializeDataChannelList(sample);
            const domain =
                await JSONExtensions.DataChannelList.toDomainModel(initDto);

            const channels = domain.package.dataChannelList.dataChannel;

            const { gmod, codebooks, locations } = await VIS.instance.getVIS(
                VisVersion.v3_4a,
            );

            const pPath = gmod.parsePath("621.11i/H135", locations);
            const sPath = gmod.parsePath("1036.13i-1/C662.1/C661", locations);
            const tag = codebooks.createTag(
                CodebookName.Content,
                "heavy.fuel.oil",
            );
            const location = locations.parse("P");

            // Query 1: tag match (cnt-heavy.fuel.oil)
            const query1 = LocalIdQueryBuilder.empty()
                .withTags((b) => b.withTag(tag).build())
                .build();

            // Query 2: primary item ignoring locations
            const query2 = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromPath(pPath, (b) =>
                    b.withoutLocations().build(),
                )
                .build();

            // Query 3: primary item with specific location P on node 621.11i
            const query3 = LocalIdQueryBuilder.empty()
                .withPrimaryItemFromPath(pPath, (b) =>
                    b
                        .withNode((nodes) => nodes.get("621.11i")!, location)
                        .build(),
                )
                .build();

            // Query 4: secondary item exact match
            const query4 = LocalIdQueryBuilder.empty()
                .withSecondaryItem(sPath)
                .build();

            // Query 5: secondary item with node 1036.13i matching all locations
            const query5 = LocalIdQueryBuilder.empty()
                .withSecondaryItemFromPath(sPath, (b) =>
                    b.withNode((nodes) => nodes.get("1036.13i")!, true).build(),
                )
                .build();

            const queries = [query1, query2, query3, query4, query5];
            const expectedMatches = [3, 7, 3, 1, 4];

            for (let i = 0; i < queries.length; i++) {
                const query = queries[i];
                let matches = 0;
                for (const channel of channels) {
                    if (await query.match(channel.dataChannelId.localId)) {
                        matches++;
                    }
                }
                expect(matches).toBe(expectedMatches[i]);
            }
        });
    });
});
