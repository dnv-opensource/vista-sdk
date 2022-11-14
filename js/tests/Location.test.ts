import { VIS, VisVersion } from "../lib";
import * as testData from "../../testdata/Locations.json";

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

        testData.locations.forEach(({ value, success, output }) => {
            const createdLocation = locations.tryParse(value);
            if (success) expect(createdLocation).toBeDefined();
            else expect(createdLocation).toBeUndefined();

            if (output) expect(createdLocation!.toString()).toBe(output);
        });
    });
});
