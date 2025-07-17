import { CodebookName, VisVersion, VisVersions } from "../lib";
import { Codebook, PositionValidationResult } from "../lib/Codebook";
import { getVIS, getVISMap } from "./Fixture";

beforeAll(() => {
    return getVISMap();
});
const version = VisVersion.v3_4a;

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

it.each(VisVersions.all)("Build and create %s", (version) => {
    const codebooks = getVIS(version).codebooks;
    for (const item of Object.values(CodebookName)) {
        if (typeof item === "string") continue;
        if (
            version < VisVersion.v3_6a &&
            (item === CodebookName.FunctionalServices ||
                item === CodebookName.MaintenanceCategory ||
                item === CodebookName.ActivityType)
        ) {
            continue; // Skip these codebooks for versions before v3.6a
        }
        const codebook = codebooks.getCodebook(item);
        expect(codebook).toBeTruthy();
        expect(codebook.name).toEqual(item);
        if (codebook.name !== CodebookName.Detail) {
            expect(codebook.standardValues).toBeTruthy();
            expect(codebook.groups).toBeTruthy();
            expect(codebook.standardValues.values.size).toBeGreaterThan(0);
        }
        const created = codebook.tryCreateTag("1");
        expect(created).toBeTruthy();
    }
});

it.each(VisVersions.all)("Position validation %s", (version) => {
    const codebooks = getVIS(version).codebooks;

    const codebookType = codebooks.getCodebook(CodebookName.Position);

    testPositionData.forEach(({ input, output }) => {
        const validPosition = codebookType.validatePosition(input);

        expect(validPosition).toEqual(output);
    });
});

it("Positions", () => {
    const codebooks = getVIS(version).codebooks;
    const positions = codebooks.getCodebook(CodebookName.Position);

    expect(positions.hasStandardValue("<number>")).toBe(false);
    expect(positions.hasStandardValue("1")).toBe(true);
});

it("States", () => {
    const codebooks = getVIS(version).codebooks;
    const states = codebooks.getCodebook(CodebookName.State);

    expect(states).not.toBe(undefined);

    expect(states.hasGroup("Clogged")).toBe(false);
    expect(states.hasStandardValue("clogged")).toBe(true);
});

it("Create tag", () => {
    const codebooks = getVIS(version).codebooks;
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

it("Detail tag", () => {
    const codebooks = getVIS(version).codebooks;
    const codebook = codebooks.getCodebook(CodebookName.Detail);

    expect(codebook).toBeTruthy();
    expect(codebook.tryCreateTag("something")).toBeTruthy();
    expect(codebook.tryCreateTag("something!")).toBeFalsy();
    expect(codebook.tryCreateTag("something<")).toBeFalsy();

    expect(() => codebook.createTag("something!")).toThrow();
    expect(() => codebook.createTag("something<")).toThrow();
});

const testCases: [CodebookName, string, boolean][] = [
    [CodebookName.Position, "aft", true],
    [CodebookName.Quantity, "test", true],
    [CodebookName.Position, "Aft", true],
];
it.each(testCases)("IsValid", (name, value, valid) => {
    const codebooks = getVIS(version).codebooks;
    const codebook = codebooks.getCodebook(name);

    const isValid = Codebook.isValidTag(name, value, codebook);
    expect(isValid).toEqual(valid);
});
