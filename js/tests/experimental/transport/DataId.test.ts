import { DataId } from "../../../lib/experimental";

describe("DataId", () => {
    const parseEqualityData = [
        "/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
        "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
        "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
        "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
        "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul",
    ];

    it.each(parseEqualityData)("Parse equality - %s", async (dataIdStr) => {
        const d1 = await DataId.parseAsync(dataIdStr);
        const d2 = await DataId.parseAsync(dataIdStr);

        expect(d1.toString()).toBe(d2.toString());
        expect(d1.toString()).toBe(dataIdStr);
    });

    it("Parse inequality", async () => {
        const d1 = await DataId.parseAsync(
            "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul",
        );
        const d2 = await DataId.parseAsync(
            "/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
        );
        const d3 = await DataId.parseAsync("some short id");

        expect(d1.toString()).not.toBe(d2.toString());
        expect(d1.toString()).not.toBe(d3.toString());
        expect(d2.toString()).not.toBe(d3.toString());

        // d3 is a short id
        expect(d3.isShortId).toBe(true);
        expect(d1.isShortId).toBe(false);
    });

    it("Equality", async () => {
        const d0 = await DataId.parseAsync(
            "/dnv-v2/vis-3-7a/621.11i/H122/meta/qty-volume/cnt-low.sulphur.heavy.fuel.oil",
        );
        const d1 = await DataId.parseAsync(
            "/dnv-v2/vis-3-7a/621.11i/H122/meta/qty-volume/cnt-low.sulphur.heavy.fuel.oil",
        );
        const d2 = await DataId.parseAsync(
            "/dnv-v2/vis-3-7a/621.11i-1/H122/meta/qty-volume/cnt-low.sulphur.heavy.fuel.oil",
        );
        const d3 = await DataId.parseAsync(
            "/dnv-v2/vis-3-7a/621.11i-S/H122/meta/qty-volume/cnt-low.sulphur.heavy.fuel.oil",
        );
        const d4 = await DataId.parseAsync(
            "/dnv-v2/vis-3-7a/621.11i-1S/H122/meta/qty-volume/cnt-low.sulphur.heavy.fuel.oil",
        );
        // PMS LocalId
        const d5 = await DataId.parseAsync(
            "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul",
        );
        const d6 = await DataId.parseAsync(
            "/dnv-v2-experimental/vis-3-6a/632.32i-1S/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul",
        );

        // Collect all unique strings in a Map
        const dict = new Map<string, string>();
        dict.set(d1.toString(), d1.toString());
        dict.set(d2.toString(), d2.toString());
        dict.set(d3.toString(), d3.toString());
        dict.set(d4.toString(), d4.toString());
        dict.set(d5.toString(), d5.toString());
        dict.set(d6.toString(), d6.toString());

        expect(dict.size).toBe(6);

        // d0 and d1 are parsed from the same string - should be equal
        expect(d0.toString()).toBe(d1.toString());

        // All others should be distinct
        expect(d1.toString()).not.toBe(d2.toString());
        expect(d1.toString()).not.toBe(d3.toString());
        expect(d1.toString()).not.toBe(d4.toString());
        expect(d1.toString()).not.toBe(d5.toString());
        expect(d1.toString()).not.toBe(d6.toString());

        expect(d2.toString()).not.toBe(d3.toString());
        expect(d2.toString()).not.toBe(d4.toString());
        expect(d2.toString()).not.toBe(d5.toString());
        expect(d2.toString()).not.toBe(d6.toString());

        expect(d3.toString()).not.toBe(d4.toString());
        expect(d3.toString()).not.toBe(d5.toString());
        expect(d3.toString()).not.toBe(d6.toString());

        expect(d4.toString()).not.toBe(d5.toString());
        expect(d4.toString()).not.toBe(d6.toString());

        expect(d5.toString()).not.toBe(d6.toString());

        // Type checks
        expect(d1.isLocalId).toBe(true);
        expect(d5.isPMSLocalId).toBe(true);
    });
});
