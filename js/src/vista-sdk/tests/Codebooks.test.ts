import { VisVersion, CodebookName } from "../lib";
import { VIS } from "../lib/VIS";

describe("Codebooks", () => {
    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const codebooksPromise = vis.getCodebooks(version);

    test("Codebooks load", async () => {
        const codebooks = await codebooksPromise;
        expect(codebooks).toBeTruthy();
        expect(codebooks.getCodebook(CodebookName.Position)).toBeTruthy();
    });

    test("Codebooks equality", async () => {
        const codebooks = await codebooksPromise;
        const codebook = codebooks.getCodebook(CodebookName.Position);
        expect(codebook.hasStandardValue("centre")).toBe(true);
    });
});
