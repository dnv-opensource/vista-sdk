import { GmodPath, LocalIdBuilder, VIS, VisVersion, VisVersions } from "../lib";
import { TestData } from "./fixtures";

const GmodPaths = TestData.GmodPaths;

describe("GmodVersioning", () => {
    // Test data for path conversion - [inputPath, expectedPath, sourceVersion?, targetVersion?]
    const validTestDataPath: Array<
        [string, string] | [string, string, VisVersion, VisVersion]
    > = [
        ["411.1/C101.72/I101", "411.1/C101.72/I101"],
        ["323.51/H362.1", "323.61/H362.1"],
        ["321.38/C906", "321.39/C906"],
        ["511.331/C221", "511.31/C121.31/C221"],
        ["511.11/C101.663i/C663.5/CS6d", "511.11/C101.663i/C663.6/CS6d"],
        ["511.11-1/C101.663i/C663.5/CS6d", "511.11-1/C101.663i/C663.6/CS6d"],
        [
            "1012.21/C1147.221/C1051.7/C101.22",
            "1012.21/C1147.221/C1051.7/C101.93",
        ],
        [
            "1012.21/C1147.221/C1051.7/C101.61/S203.6",
            "1012.21/C1147.221/C1051.7/C101.311/C467.5",
        ],
        ["001", "001"],
        ["038.7/F101.2/F71", "038.7/F101.2/F71"],
        [
            "1012.21/C1147.221/C1051.7/C101.61/S203.6/S61",
            "1012.21/C1147.221/C1051.7/C101.311/C467.5/S61",
        ],
        ["000a", "000a"],
        [
            "1012.21/C1147.221/C1051.7/C101.61/S203.2/S101",
            "1012.21/C1147.221/C1051.7/C101.61/S203.3/S110.1/S101",
        ],
        [
            "1012.21/C1147.221/C1051.7/C101.661i/C624",
            "1012.21/C1147.221/C1051.7/C101.661i/C621",
        ],
        [
            "1012.22/S201.1/C151.2/S110.2/C101.64i",
            "1012.22/S201.1/C151.2/S110.2/C101.64",
        ],
        [
            "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
            "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
        ],
        [
            "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
            "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
        ],
        [
            "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
            "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
        ],
        ["221.31/C1141.41/C664.2/C471", "221.31/C1141.41/C664.2/C471"],
        ["514/E15", "514"],
        [
            "1346/S201.1/C151.31/S110.2/C111.1/C109.16/C509",
            "1346/S201.1/C151.31/S110.2/C111.1/C109.126/C509",
            VisVersion.v3_7a,
            VisVersion.v3_8a,
        ],
    ];

    // Test data for full path conversion
    const validTestDataFullPath: Array<[string, string]> = [
        [
            "VE/600a/630/632/632.3/632.32/632.32i-2/S110",
            "VE/600a/630/632/632.3/632.32/632.32i-2/SS5/S110",
        ],
    ];

    // Test data for node conversion - [inputCode, location, expectedCode]
    const validTestDataNode: Array<[string, string | null, string]> = [
        ["1014.211", null, "1014.211"],
        ["323.5", null, "323.6"],
        ["412.72", null, "412.7i"],
        ["323.4", null, "323.5"],
        ["323.51", null, "323.61"],
        ["323.6", null, "323.7"],
        ["C101.212", null, "C101.22"],
        ["C101.22", null, "C101.93"],
        ["511.31", null, "C121.1"],
        ["C101.31", "5", "C101.31"],
    ];

    describe("convertPath", () => {
        test.each(validTestDataPath)(
            "converts path %s to %s",
            async (...args) => {
                const [
                    inputPath,
                    expectedPath,
                    sourceVersion = VisVersion.v3_4a,
                    targetVersion = VisVersion.v3_6a,
                ] = args;

                const vis = VIS.instance;

                // Parse source and target paths
                const sourcePath = await GmodPath.parseAsync(
                    inputPath,
                    sourceVersion,
                );

                expect(sourcePath).not.toBeUndefined();
                expect(sourcePath?.toString()).toBe(inputPath);

                // Convert path
                const targetPath = await vis.convertPath(
                    sourceVersion,
                    sourcePath!,
                    targetVersion,
                );

                expect(targetPath).not.toBeUndefined();
                expect(targetPath?.toString()).toBe(expectedPath);

                // Verify target path parses correctly on target Gmod
                const targetGmod = await vis.getGmod(targetVersion);
                const targetLocations = await vis.getLocations(targetVersion);
                const parsedTarget = targetGmod.tryParsePath(
                    expectedPath,
                    targetLocations,
                );
                expect(parsedTarget).not.toBeUndefined();
                expect(parsedTarget?.toString()).toBe(expectedPath);
            },
        );
    });

    describe("convertFullPath", () => {
        test.each(validTestDataFullPath)(
            "converts full path %s to %s",
            async (inputPath, expectedPath) => {
                const vis = VIS.instance;
                const sourceVersion = VisVersion.v3_4a;
                const targetVersion = VisVersion.v3_6a;

                // Parse source full path
                const sourcePath = await GmodPath.parseAsyncFromFullPath(
                    inputPath,
                    sourceVersion,
                );

                expect(sourcePath).not.toBeUndefined();
                expect(sourcePath?.toFullPathString()).toBe(inputPath);

                // Convert path
                const targetPath = await vis.convertPath(
                    sourceVersion,
                    sourcePath!,
                    targetVersion,
                );

                expect(targetPath).not.toBeUndefined();
                expect(targetPath?.toFullPathString()).toBe(expectedPath);

                // Verify target path parses correctly
                const parsedTarget = await GmodPath.parseAsyncFromFullPath(
                    expectedPath,
                    targetVersion,
                );
                expect(parsedTarget?.toFullPathString()).toBe(expectedPath);
            },
        );
    });

    describe("convertNode", () => {
        test.each(validTestDataNode)(
            "converts node %s to %s",
            async (inputCode, location, expectedCode) => {
                const vis = VIS.instance;
                const sourceVersion = VisVersion.v3_4a;
                const targetVersion = VisVersion.v3_6a;

                const sourceGmod = await vis.getGmod(sourceVersion);
                let sourceNode = sourceGmod.tryGetNode(inputCode);

                expect(sourceNode).not.toBeUndefined();

                // Set location if provided
                if (location) {
                    const locations = await vis.getLocations(sourceVersion);
                    sourceNode = sourceNode?.tryWithLocation(
                        location,
                        locations,
                    );
                    expect(sourceNode).not.toBeUndefined();
                    expect(sourceNode?.location?.toString()).toBe(location);
                }

                // Convert node
                const targetNode = await vis.convertNode(
                    sourceVersion,
                    sourceNode!,
                    targetVersion,
                );

                expect(targetNode).not.toBeUndefined();
                expect(targetNode?.code).toBe(expectedCode);

                // Verify location is preserved
                if (location) {
                    expect(targetNode?.location?.toString()).toBe(location);
                }
            },
        );
    });

    describe("versioning edge cases", () => {
        test("throws on conversion exception", async () => {
            const vis = VIS.instance;
            const sourceVersion = VisVersion.v3_7a;
            const targetVersion = VisVersion.v3_8a;

            const sourcePath = await GmodPath.parseAsync(
                "244.1i/H101.111/H401",
                sourceVersion,
            );

            expect(sourcePath).not.toBeUndefined();

            // This is expected to throw due to uncovered MES case
            await expect(
                vis.convertPath(sourceVersion, sourcePath!, targetVersion),
            ).rejects.toThrow();
        });

        test("converts root node correctly", async () => {
            const vis = VIS.instance;
            const sourceVersion = VisVersion.v3_4a;
            const targetVersion = VisVersion.v3_6a;

            const sourceGmod = await vis.getGmod(sourceVersion);
            const rootNode = sourceGmod.rootNode;

            expect(rootNode.code).toBe("VE");

            const targetNode = await vis.convertNode(
                sourceVersion,
                rootNode,
                targetVersion,
            );

            expect(targetNode).not.toBeUndefined();
            expect(targetNode?.code).toBe("VE");
        });
    });

    describe("convertLocalId", () => {
        const convertLocalIdData: [string, string][] = [
            [
                "/dnv-v2/vis-3-4a/411.1/C101/sec/411.1/C101.64i/S201/meta/cnt-condensate",
                "/dnv-v2/vis-3-5a/411.1/C101/sec/411.1/C101.64/S201/meta/cnt-condensate",
            ],
            [
                "/dnv-v2/vis-3-4a/411.1/C101.64i-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running",
                "/dnv-v2/vis-3-9a/411.1/C101.64-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running",
            ],
        ];

        test.each(convertLocalIdData)(
            "converts LocalId %s to %s",
            async (sourceLocalIdStr, targetLocalIdStr) => {
                const vis = VIS.instance;

                const sourceLocalId =
                    await LocalIdBuilder.parseAsync(sourceLocalIdStr);
                const targetLocalId =
                    await LocalIdBuilder.parseAsync(targetLocalIdStr);

                expect(sourceLocalId).toBeDefined();
                expect(targetLocalId).toBeDefined();

                const convertedLocalId = await vis.convertLocalIdBuilder(
                    sourceLocalId,
                    targetLocalId.visVersion!,
                );

                expect(convertedLocalId).toBeDefined();
                expect(convertedLocalId!.toString()).toBe(targetLocalIdStr);
            },
        );
    });

    describe("convertGmodPathWithLocation", () => {
        test("converts path with location 691.811i-A/H101.11-1 -> 691.83111i-A/H101.11-1", async () => {
            const vis = VIS.instance;
            const sourceVersion = VisVersion.v3_7a;
            const targetVersion = VisVersion.v3_9a;

            const sourcePath = await GmodPath.parseAsync(
                "691.811i-A/H101.11-1",
                sourceVersion,
            );
            const expectedPath = await GmodPath.parseAsync(
                "691.83111i-A/H101.11-1",
                targetVersion,
            );

            expect(sourcePath).toBeDefined();
            expect(expectedPath).toBeDefined();

            const convertedPath = await vis.convertPath(
                sourceVersion,
                sourcePath!,
                targetVersion,
            );

            expect(convertedPath).toBeDefined();
            expect(convertedPath!.toString()).toBe(expectedPath!.toString());
        });
    });

    describe("validGmodPathToLatest", () => {
        test.each(GmodPaths.Valid)(
            "converts valid path %p to latest",
            async ({ path: testPath, visVersion: visVersionStr }) => {
                const vis = VIS.instance;
                const sourceVersion = VisVersions.parse(visVersionStr);

                const sourcePath = await GmodPath.tryParseAsync(
                    testPath,
                    sourceVersion,
                );
                expect(sourcePath).toBeDefined();

                const targetPath = await vis.convertPath(
                    sourceVersion,
                    sourcePath!,
                    VIS.latestVisVersion,
                );
                expect(targetPath).toBeDefined();
            },
        );
    });
});
