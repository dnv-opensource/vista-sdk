import GmodPaths from "../../testdata/GmodPaths.json";
import IndividualizableSets from "../../testdata/IndividualizableSets.json";
import { GmodPath, VisVersion, VisVersions } from "../lib";
import { GmodIndividualizableSet } from "../lib/GmodPath";
import { getVIS, getVISMap } from "./Fixture";

describe("GmodPath", () => {
    const visVersion = VisVersion.v3_4a;

    beforeAll(() => {
        return getVISMap();
    });

    it.each(GmodPaths.Valid)(
        "Valid path %p",
        ({ path: testPath, visVersion }) => {
            const { gmod, locations } = getVIS(VisVersions.parse(visVersion));
            const path = gmod.parsePath(testPath, locations);
            expect(path).toBeTruthy();
            const result = path!.toString();
            expect(result).toEqual(testPath);
        }
    );

    it.each(GmodPaths.Invalid)(
        "Invalid path %p",
        ({ path: testPath, visVersion }) => {
            const { gmod, locations } = getVIS(VisVersions.parse(visVersion));

            const path = GmodPath.tryParse(testPath, locations, gmod);

            expect(path).toBeFalsy();
        }
    );

    it.each([
        [
            "411.1/C101.72/I101",
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101",
        ],
        [
            "612.21-1/C701.13/S93",
            "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93",
        ],
    ])("Full string parsing %p", (shortPathStr, expectedFullPathStr) => {
        const { gmod, locations } = getVIS(visVersion);
        const path = GmodPath.parse(shortPathStr, locations, gmod);
        const fullString = path.toFullPathString();
        expect(fullString).toEqual(expectedFullPathStr);

        const parsedPath = GmodPath.tryParseFromFullPath(
            fullString,
            gmod,
            locations
        );
        expect(parsedPath).toBeTruthy();
        expect(path.equals(parsedPath)).toBe(true);
        expect(path.toFullPathString()).toEqual(fullString);
        expect(parsedPath!.toFullPathString()).toEqual(fullString);
        expect(path.toString()).toEqual(shortPathStr);
        expect(parsedPath!.toString()).toEqual(shortPathStr);
    });

    it("GmodPath withLocation", () => {
        const { gmod, locations } = getVIS(visVersion);

        let path = gmod.parseFromFullPath(
            "VE/400a/410/411/411i/411.1/CS1/C101/C101.3/C101.3i/C101.31/C101.311-1",
            locations
        );
        const newLocation = locations.parse("2");

        // Test setting location on the path node
        let sets = path.individualizableSets;
        let set: GmodIndividualizableSet | undefined = sets[sets.length - 1];
        expect(set).toBeDefined();
        set.location = newLocation;
        path = set.build();
        expect(path.node.location?.toString()).toEqual(newLocation.toString());
        expect(path.getNode("C101.31").toString()).toEqual("C101.31-2");
        expect(path.getNode("C101.3i").toString()).toEqual("C101.3i-2");

        // Test setting location on a parent in the path
        sets = path.individualizableSets;
        set = sets.find((s) => s.codes.includes("411.1"));
        expect(set).toBeDefined();
        set!.location = newLocation;
        path = set!.build();
        expect(path.getNode("411.1")?.toString()).toEqual("411.1-2");
        expect(path.getNode("411")?.toString()).not.toEqual("411-2");

        sets = path.individualizableSets;
        set = sets.find((s) => s.codes.includes("411"));
        expect(set).toBeUndefined();

        // Test setting location on the path node
        path = path.withoutLocation();
        sets = path.individualizableSets;
        set = sets.find((s) => s.codes.includes("C101.3i"));
        expect(set).toBeDefined();
        set!.location = newLocation;
        path = set!.build();
        expect(path.getNode("C101.31").toString()).toEqual("C101.31-2");
    });

    it("path individualizes", () => {
        const { gmod, locations } = getVIS(visVersion);

        const path = gmod.parsePath("411.1/C101.62/S205", locations);
        const sets = path.individualizableSets;
        expect(sets.length).toBe(2);
    });

    it("toFullPathString", () => {
        const { gmod, locations } = getVIS(visVersion);

        let path = gmod.parsePath("511.11-1/C101.663i-1/C663", locations);

        expect(path.toFullPathString()).toEqual(
            "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663"
        );

        path = gmod.parsePath("846/G203.32-2/S110.2-1/E31", locations);
        expect(path.toFullPathString()).toEqual(
            "VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31"
        );
    });

    it.each(IndividualizableSets)("Individualizable sets %s", (testCase) => {
        const version = VisVersions.parse(testCase.visVersion);
        const { gmod, locations } = getVIS(version);

        if (!testCase.expected) {
            const parsed = testCase.isFullPath
                ? gmod.tryParseFromFullPath(testCase.path, locations)
                : gmod.tryParsePath(testCase.path, locations);
            expect(parsed).toBeFalsy();
            return;
        }

        const path = testCase.isFullPath
            ? gmod.parseFromFullPath(testCase.path, locations)
            : gmod.parsePath(testCase.path, locations);
        const sets = path.individualizableSets;
        expect(sets.length).toBe(testCase.expected.length);
        for (let i = 0; i < testCase.expected.length; i++) {
            expect(sets[i].codes).toEqual(testCase.expected[i]);
        }
    });

    it.each(IndividualizableSets)(
        "Individualizable sets fullpath %s",
        (testCase) => {
            const version = VisVersions.parse(testCase.visVersion);
            const { gmod, locations } = getVIS(version);

            if (testCase.isFullPath) return;

            if (!testCase.expected) {
                expect(gmod.tryParsePath(testCase.path, locations)).toBeFalsy();
                return;
            }

            const path = gmod.parseFromFullPath(
                gmod.parsePath(testCase.path, locations).toFullPathString(),
                locations
            );
            const sets = path.individualizableSets;
            expect(sets.length).toBe(testCase.expected.length);
            for (let i = 0; i < testCase.expected.length; i++) {
                expect(sets[i].codes).toEqual(testCase.expected[i]);
            }
        }
    );

    it.each(GmodPaths.Valid)(
        "Individualizable sets valid paths %p",
        ({ path: testPath, visVersion }) => {
            const { gmod, locations } = getVIS(VisVersions.parse(visVersion));

            const path = GmodPath.parse(testPath, locations, gmod);
            const sets = path.individualizableSets;

            const uniqueCodes = new Set<string>();
            for (const set of sets) {
                for (const node of set.nodes) {
                    expect(uniqueCodes.has(node.code)).toBe(false);
                    uniqueCodes.add(node.code);
                }
            }
        }
    );

    it.each(GmodPaths.Valid)(
        "Individualizable sets valid paths full path %p",
        ({ path: testPath, visVersion }) => {
            const { gmod, locations } = getVIS(VisVersions.parse(visVersion));

            const path = GmodPath.parseFromFullPath(
                GmodPath.parse(testPath, locations, gmod).toFullPathString(),
                gmod,
                locations
            );
            const sets = path.individualizableSets;

            const uniqueCodes = new Set<string>();
            for (const set of sets) {
                for (const node of set.nodes) {
                    expect(uniqueCodes.has(node.code)).toBe(false);
                    uniqueCodes.add(node.code);
                }
            }
        }
    );
});
