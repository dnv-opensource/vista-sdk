import { VIS, VisVersion } from "../lib";
import * as testData from "../../testdata/Locations.json";
import { LocationParsingErrorBuilder } from "../lib/internal/LocationParsingErrorBuilder";

describe("Location", () => {
    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const locationPromise = vis.getLocations(version);

    test("Location loads", async () => {
        const location = await locationPromise;

        expect(location).toBeTruthy();
    });

    test("Location validation", async () => {
        const locations = await locationPromise;

        testData.locations.forEach(
            ({ value, success, output, expectedErrorMessages }) => {
                const errorBuilder = new LocationParsingErrorBuilder();
                const createdLocation = locations.tryParse(value, errorBuilder);
                if (!success) {
                    expect(createdLocation).toBeUndefined();
                    if (
                        expectedErrorMessages !== undefined &&
                        expectedErrorMessages.length > 0
                    ) {
                        const errorMessages = expectedErrorMessages as string[];
                        errorBuilder.errors.forEach((e) => {
                            if (!errorMessages.includes(e.message))
                                console.log(e.message);
                            expect(
                                errorMessages.includes(e.message)
                            ).toBeTruthy();
                        });

                        expect(errorBuilder.errors.length).not.toEqual(0);
                        expect(errorBuilder.errors.length).toEqual(
                            expectedErrorMessages.length
                        );
                    }
                } else expect(createdLocation!).toBeDefined();

                if (output) expect(createdLocation!.toString()).toBe(output);
            }
        );
    });
});
