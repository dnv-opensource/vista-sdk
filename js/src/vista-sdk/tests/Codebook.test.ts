import { VisVersion, CodebookName } from "../lib";
import { PositionValidationResult } from "../lib/Codebook";
import { VIS } from "../lib/VIS";

describe("Codebooks", () => {
    const testPositionData = [
        { input: "phase.w.u", output: PositionValidationResult.Valid },
        { input: "outside-phase.w.u", output: PositionValidationResult.Valid },
        {
            input: "outside-phase.w.u-1",
            output: PositionValidationResult.Valid,
        },
        {
            input: "outside-phase.w.u-10",
            output: PositionValidationResult.Valid,
        },
        {
            input: "10-outside-phase.w.u",
            output: PositionValidationResult.InvalidOrder,
        },
        { input: "10-#", output: PositionValidationResult.Invalid },
        { input: "#", output: PositionValidationResult.Invalid },
        { input: "outside!", output: PositionValidationResult.Invalid },
        { input: "1-centre", output: PositionValidationResult.InvalidOrder },
        {
            input: "phas!.w.u-outside-10",
            output: PositionValidationResult.Invalid,
        },
        {
            input: "phase.w.u-outsid!-10",
            output: PositionValidationResult.Invalid,
        },
        {
            input: "outside-phased.w.u-10",
            output: PositionValidationResult.Custom,
        },
        { input: "outsidee", output: PositionValidationResult.Custom },
        {
            input: "port-starboard",
            output: PositionValidationResult.InvalidGrouping,
        },
        {
            input: "starboard-port",
            output: PositionValidationResult.InvalidOrder,
        },
        {
            input: "port-starboard-1",
            output: PositionValidationResult.InvalidGrouping,
        },
        {
            input: "starboard-port-1",
            output: PositionValidationResult.InvalidOrder,
        },
    ];

    const vis = VIS.instance;
    const version = VisVersion.v3_4a;
    const codebooksPromise = vis.getCodebooks(version);

    test("Position validation", async () => {
        const codebooks = await codebooksPromise;

        const codebookType = codebooks.getCodebook(CodebookName.Position);

        testPositionData.forEach(({ input, output }) => {
            const validPosition = codebookType.validatePosition(input);

            expect(validPosition).toEqual(output);
        });
    }); // TODO

    test("Positions", async () => {
        const codebooks = await codebooksPromise;
        const positions = codebooks.getCodebook(CodebookName.Position);

        expect(positions.hasStandardValue("<number>")).toBe(false);
        expect(positions.hasStandardValue("1")).toBe(true);
    });

    test("States", async () => {
        const codebooks = await codebooksPromise;
        const states = codebooks.getCodebook(CodebookName.State);

        expect(states).not.toBe(undefined);

        expect(states.hasGroup("Clogged")).toBe(false);
        expect(states.hasStandardValue("clogged")).toBe(true);
    });

    test("Create tag", async () => {
        const codebooks = await codebooksPromise;
        const codebookType = codebooks.getCodebook(CodebookName.Position);

        const metadataTag1 = codebookType.createTag("1");
        expect(metadataTag1.value).toEqual("1");
        expect(metadataTag1.isCustom).toBe(false);
        expect(metadataTag1.name).toEqual(CodebookName.Position);

        const metadataTag2 = codebookType.createTag("centre");
        expect(metadataTag2.value).toEqual("centre");
        expect(metadataTag2.isCustom).toBe(false);

        const metadataTag3 = codebookType.createTag("centre-1");
        expect(metadataTag3.value).toEqual("centre-1");
        expect(metadataTag3.isCustom).toBe(false);
        expect(metadataTag3.prefix).toEqual("-");

        const metadataTag4 = codebookType.createTag("somethingcustom");
        expect(metadataTag4.value).toEqual("somethingcustom");
        expect(metadataTag4.isCustom).toBe(true);
        expect(metadataTag4.prefix).toEqual("~");

        expect(() => codebookType.createTag("1-centre")).toThrow();
        expect(codebookType.tryCreateTag("1-centre")).toBeFalsy();

        expect(() => codebookType.createTag("centre!")).toThrow();
        expect(codebookType.tryCreateTag("centre!")).toBeFalsy();
    });

    test("Detail tag", async () => {
        const codebooks = await vis.getCodebooks(version);
        const codebook = codebooks.getCodebook(CodebookName.Detail);

        expect(codebook).toBeTruthy();
        expect(codebook.tryCreateTag("something")).toBeTruthy();
        expect(codebook.tryCreateTag("something!")).toBeFalsy();
        expect(codebook.tryCreateTag("something<")).toBeFalsy();

        expect(() => codebook.createTag("something!")).toThrow();
        expect(() => codebook.createTag("something<")).toThrow();
    });
});
