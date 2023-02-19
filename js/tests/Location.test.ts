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
});
