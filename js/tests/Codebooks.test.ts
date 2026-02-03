import { CodebookName, VIS } from "../lib";

const visVersion = VIS.latestVisVersion;

it("Codebooks load", async () => {
    const { codebooks } = await VIS.instance.getVIS(visVersion);
    expect(codebooks).toBeTruthy();
    expect(codebooks.getCodebook(CodebookName.Position)).toBeTruthy();
});

it("Codebooks equality", async () => {
    const { codebooks } = await VIS.instance.getVIS(visVersion);
    const codebook = codebooks.getCodebook(CodebookName.Position);
    expect(codebook.hasStandardValue("centre")).toBe(true);
});
