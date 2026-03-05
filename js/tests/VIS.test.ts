import { VIS, VisVersion, VisVersionExtension, VisVersions } from "../lib";

describe("VIS", () => {
    it("VersionString", () => {
        const version = VisVersion.v3_4a;
        const versionStr = VisVersionExtension.toVersionString(version);

        expect(versionStr).toBe("3-4a");
        expect(VisVersions.parse(versionStr)).toBe(version);
    });

    it("OrderedVisVersions", () => {
        const versions = VisVersions.all;

        const index34 = versions.indexOf(VisVersion.v3_4a);
        const index310 = versions.indexOf(VisVersion.v3_10a);
        expect(index34).toBeGreaterThanOrEqual(0);
        expect(index310).toBeGreaterThanOrEqual(0);
        expect(index34).toBeLessThan(index310);
    });

    it("VisVersion parse", () => {
        for (const version of VisVersions.all) {
            const versionStr = VisVersionExtension.toVersionString(version);
            const parsed = VisVersions.parse(versionStr);
            expect(parsed).toBe(version);
        }
    });

    it("VisVersion tryParse", () => {
        expect(VisVersions.tryParse("3-4a")).toBe(VisVersion.v3_4a);
        expect(VisVersions.tryParse("3-10a")).toBe(VisVersion.v3_10a);
        expect(VisVersions.tryParse("invalid")).toBeUndefined();
        expect(VisVersions.tryParse("")).toBeUndefined();
    });

    it("VisVersionExtension comparison", () => {
        expect(
            VisVersionExtension.lessThan(VisVersion.v3_4a, VisVersion.v3_10a),
        ).toBe(true);
        expect(
            VisVersionExtension.greaterThanOrEqual(
                VisVersion.v3_10a,
                VisVersion.v3_4a,
            ),
        ).toBe(true);
        expect(
            VisVersionExtension.lessThan(VisVersion.v3_10a, VisVersion.v3_4a),
        ).toBe(false);
    });

    it("latestVisVersion is set", () => {
        expect(VIS.latestVisVersion).toBeDefined();
        expect(VIS.latestVisVersion).toBe(VisVersion.v3_10a);
    });
});
