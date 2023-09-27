import { LocationBuilder, VIS, VisVersion, VisVersions } from "../lib";
import * as testData from "../../testdata/Locations.json";
import { LocationParsingErrorBuilder } from "../lib/internal/LocationParsingErrorBuilder";

describe("Location", () => {
    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const locationPromise = vis.getLocations(version);

    test.each(VisVersions.all)(
        "Location loads for VIS version %s",
        async (version) => {
            const location = await vis.getLocations(version);

            expect(location).toBeTruthy();
            expect(location.groups).toBeTruthy();
        }
    );

    test.each(testData.locations.map((l) => [l]))(
        "Location parsing - %s",
        async ({ value, success, output, expectedErrorMessages }) => {
            const locations = await locationPromise;

            const errorBuilder = new LocationParsingErrorBuilder();
            const createdLocation = locations.tryParse(value, errorBuilder);
            if (!success) {
                expect(createdLocation).toBeUndefined();
                if (expectedErrorMessages.length > 0) {
                    expect(errorBuilder).toBeDefined();
                    expect(errorBuilder.hasError).toBe(true);
                    const actualErrors = errorBuilder.errors.map(
                        (e) => e.message
                    );
                    expect(actualErrors).toEqual(expectedErrorMessages);
                }
            } else {
                expect(errorBuilder.hasError).toBe(false);
                expect(createdLocation).toBeTruthy();
                expect(createdLocation!.toString()).toBe(output);
            }
        }
    );

    test("Location parsing throws", async () => {
        const locations = await locationPromise;

        expect(() => locations.parse(null!)).toThrowError();
        expect(() => locations.parse(undefined!)).toThrowError();
    });

    test("Location builder", async () => {
        const locations = await locationPromise;

        const locationStr = "11FIPU";
        var location = locations.parse(locationStr);

        var builder = LocationBuilder.create(locations);

        builder = builder
            .withNumber(11)
            .withSide("P")
            .withTransverse("I")
            .withLongitudinal("F")
            .withValue("U");

        expect(builder.toString()).toEqual("11FIPU");
        expect(builder.number).toEqual(11);
        expect(builder.side).toEqual("P");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");

        expect(() => (builder = builder.withValue("X"))).toThrowError();
        expect(() => (builder = builder.withNumber(-1))).toThrowError();
        expect(() => (builder = builder.withNumber(0))).toThrowError();
        expect(() => (builder = builder.withSide("A"))).toThrowError();
        expect(() => (builder = builder.withValue("a"))).toThrowError();

        expect(builder.build()).toEqual(location);

        builder = LocationBuilder.create(locations).withLocation(
            builder.build()
        );

        expect(builder.toString()).toEqual("11FIPU");
        expect(builder.number).toEqual(11);
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
});
