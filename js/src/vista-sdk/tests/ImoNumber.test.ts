import { ImoNumber, LocalId, Pmod, VIS, VisVersion } from "../lib";
import * as testData from "../../../../testdata/ImoNumbers.json";

describe("ImoNumbers", () => {
    it.each(testData.imoNumbers)('validate', ({ value, success, output }) => {
        const parsedImo = ImoNumber.tryParse(value);
        if (success)
            expect(parsedImo).toBeDefined();
        else
            expect(parsedImo).toBeUndefined();

        if (output)
            expect(parsedImo!.toString()).toBe(output);
    });
});
