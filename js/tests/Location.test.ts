import * as testData from "../../testdata/Locations.json";
import {
    Location,
    LocationBuilder,
    LocationGroup,
    VIS,
    VisVersion,
    VisVersions,
} from "../lib";
import { LocationParsingErrorBuilder } from "../lib/internal/LocationParsingErrorBuilder";

describe("Location", () => {
    const version = VisVersion.v3_4a;

    it.each(VisVersions.all)(
        "Location loads for VIS version %s",
        async (version) => {
            const { locations } = await VIS.instance.getVIS(version);

            expect(locations).toBeTruthy();
            expect(locations.groups).toBeTruthy();
        },
    );

    it.each(testData.locations.map((l) => [l]))(
        "Location parsing - %s",
        async ({ value, success, output, expectedErrorMessages }) => {
            const { locations } = await VIS.instance.getVIS(version);

            const errorBuilder = new LocationParsingErrorBuilder();
            const createdLocation = locations.tryParse(value, errorBuilder);
            if (!success) {
                expect(createdLocation).toBeUndefined();
                if (expectedErrorMessages.length > 0) {
                    expect(errorBuilder).toBeDefined();
                    expect(errorBuilder.hasError).toBe(true);
                    const actualErrors = errorBuilder.errors.map(
                        (e) => e.message,
                    );
                    expect(actualErrors).toEqual(expectedErrorMessages);
                }
            } else {
                expect(errorBuilder.hasError).toBe(false);
                expect(createdLocation).toBeTruthy();
                expect(createdLocation!.toString()).toBe(output);
            }
        },
    );

    it("Location parsing throws", async () => {
        const { locations } = await VIS.instance.getVIS(version);

        expect(() => locations.parse(null!)).toThrow();
        expect(() => locations.parse(undefined!)).toThrow();
    });

    it("Location builder", async () => {
        const { locations } = await VIS.instance.getVIS(version);

        const locationStr = "21FIPU";
        var location = locations.parse(locationStr);

        var builder = LocationBuilder.create(locations);

        builder = builder.withNumber(62);
        expect(builder.toString()).toEqual("62");

        builder = builder
            .withNumber(21)
            .withSide("P")
            .withTransverse("I")
            .withLongitudinal("F")
            .withValue("U");

        expect(builder.toString()).toEqual("21FIPU");
        expect(builder.number).toEqual(21);
        expect(builder.side).toEqual("P");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");

        expect(() => (builder = builder.withValue("X"))).toThrow();
        expect(() => (builder = builder.withNumber(-1))).toThrow();
        expect(() => (builder = builder.withNumber(0))).toThrow();
        expect(() => (builder = builder.withNumber(1.5))).toThrow();
        expect(() => (builder = builder.withSide("A"))).toThrow();
        expect(() => (builder = builder.withValue("a"))).toThrow();

        expect(builder.build()).toEqual(location);

        builder = LocationBuilder.create(locations).withLocation(
            builder.build(),
        );

        expect(builder.toString()).toEqual("21FIPU");
        expect(builder.number).toEqual(21);
        expect(builder.side).toEqual("P");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");

        builder = builder.withValue("S").withValue(2);

        expect(builder.toString()).toEqual("2FISU");
        expect(builder.number).toEqual(2);
        expect(builder.side).toEqual("S");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");
    });

    it("LocationGroup properties", () => {
        const values = Object.values(LocationGroup).filter(
            (v) => typeof v === "number",
        ) as number[];
        // All values unique
        expect(new Set(values).size).toBe(values.length);
        // 5 groups total
        expect(values.length).toBe(5);
        // Number is 0
        expect(LocationGroup.Number).toBe(0);
    });

    it("Location single digit builder", async () => {
        const { locations } = await VIS.instance.getVIS(version);

        // Test single digit location parsing
        const builder1 = LocationBuilder.create(locations).withLocation(
            new Location("1"),
        );
        expect(builder1.number).toBe(1);
        expect(builder1.toString()).toBe("1");

        const builder5 = LocationBuilder.create(locations).withLocation(
            new Location("5"),
        );
        expect(builder5.number).toBe(5);
        expect(builder5.toString()).toBe("5");

        const builder9 = LocationBuilder.create(locations).withLocation(
            new Location("9"),
        );
        expect(builder9.number).toBe(9);
        expect(builder9.toString()).toBe("9");

        // Test single digit with characters
        const builderMixed = LocationBuilder.create(locations).withLocation(
            new Location("1FIPU"),
        );
        expect(builderMixed.number).toBe(1);
        expect(builderMixed.side).toBe("P");
        expect(builderMixed.vertical).toBe("U");
        expect(builderMixed.transverse).toBe("I");
        expect(builderMixed.longitudinal).toBe("F");
        expect(builderMixed.toString()).toBe("1FIPU");
    });

    it("Location multi-digit number not sorted", async () => {
        const { locations } = await VIS.instance.getVIS(version);

        const builder = LocationBuilder.create(locations)
            .withNumber(10)
            .withSide("S")
            .withValue("U")
            .withLongitudinal("F");

        // Should be "10FSU" NOT "01FSU"
        expect(builder.toString()).toBe("10FSU");
        expect(builder.number).toBe(10);
    });

    it("Location equality", async () => {
        const { gmod, locations } = await VIS.instance.getVIS(version);

        const node1 = gmod.getNode("C101.663").withLocation("FIPU", locations);
        const node2 = gmod.getNode("C101.663").withLocation("FIPU", locations);

        expect(node1.location).toBeDefined();
        expect(node2.location).toBeDefined();
        expect(node1.location!.equals(node2.location!)).toBe(true);
        expect(node1.equals(node2)).toBe(true);
        expect(node1).not.toBe(node2);
    });
});
