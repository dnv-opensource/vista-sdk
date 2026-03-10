import { ImoNumber } from "../lib";
import { TestData } from "./fixtures";

describe("ImoNumbers", () => {
    it.each(TestData.ImoNumbers.imoNumbers)(
        "validate",
        ({ value, success, output }) => {
            const parsedImo = ImoNumber.tryParse(value);
            if (success) expect(parsedImo).toBeDefined();
            else expect(parsedImo).toBeUndefined();

            if (output) expect(parsedImo!.toString()).toBe(output);
        },
    );
});
