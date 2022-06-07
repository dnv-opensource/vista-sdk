import { CodebookName, CodebookNames, MetadataTag } from ".";
import { CodebookDto } from "./types/CodebookDto";
import { isNullOrWhiteSpace, tryParseInt } from "./util/util";

export class Codebook {
    public name: CodebookName;
    private readonly _groupMap: Map<string, string>;
    private readonly _standardValues: CodebookStandardValues;
    private readonly _groups: CodebookGroups;

    private static readonly tagAlphabet =
        "abcdefghijklmnopqrstuvwxyz0123456789.";
    private static readonly positionTagAlphabet = this.tagAlphabet + "-";

    public constructor(dto: CodebookDto) {
        this.name = CodebookNames.fromString(dto.name);

        this._groupMap = new Map<string, string>();

        const data = Object.entries(dto.values).flatMap(([key, values]) =>
            values
                .map((v) => ({ group: key, value: v.trim() }))
                .filter((v) => v.value !== "<number>")
        );

        for (const t of data) {
            this._groupMap.set(t.value, t.group);
        }

        const valueSet = new Set<string>(data.map((t) => t.value));
        const groupSet = new Set<string>(data.map((t) => t.group));

        this._standardValues = new CodebookStandardValues(this.name, valueSet);
        this._groups = new CodebookGroups(this.name, groupSet);
    }

    public get groups() {
        return this._groups;
    }

    public get standardValues() {
        return this._standardValues;
    }

    public hasGroup(group: string): boolean {
        return this._groups.contains(group);
    }
    public hasStandardValue(value: string): boolean {
        return this._standardValues.contains(value);
    }

    public tryCreateTag(value?: string): MetadataTag | undefined {
        if (!value || isNullOrWhiteSpace(value)) return;

        let isCustom = false;
        if (this.name === CodebookName.Position) {
            if (this.validatePosition(value) < 100) return;
            if (
                this.validatePosition(value) === PositionValidationResult.Custom
            ) {
                isCustom = true;
            }
        } else {
            if (![...value].every((c) => Codebook.tagAlphabet.includes(c)))
                return;
            if (
                this.name !== CodebookName.Detail &&
                !this._standardValues.contains(value)
            ) {
                isCustom = true;
            }
        }
        return new MetadataTag(this.name, value, isCustom);
    }

    public createTag(value?: string): MetadataTag {
        const tag = this.tryCreateTag(value);
        if (!tag)
            throw new Error(
                `Invalid value for metadata tag: codebook=${this.name}, value=${value}`
            );
        return tag;
    }

    public validatePosition(position: string): PositionValidationResult {
        if (isNullOrWhiteSpace(position))
            return PositionValidationResult.Invalid;

        if (position.trim().length != position.length)
            return PositionValidationResult.Invalid;

        if (
            ![...position].every((c) =>
                Codebook.positionTagAlphabet.includes(c)
            )
        )
            return PositionValidationResult.Invalid;

        if (this._standardValues.contains(position))
            return PositionValidationResult.Valid;

        if (typeof tryParseInt(position) === "number")
            return PositionValidationResult.Valid;

        if (!position.includes("-")) return PositionValidationResult.Custom;

        const positions = position.split("-");
        const validations: PositionValidationResult[] = [];
        for (const positionStr of positions) {
            validations.push(this.validatePosition(positionStr));
        }

        if (!!validations.find((v) => v < 100)) return Math.max(...validations);

        const numberNotAtEnd = !!positions.find(
            (pValue, pIndex) =>
                tryParseInt(pValue) && pIndex < positions.length - 1
        );
        const positionsWithoutNumber = positions.filter((p) => !tryParseInt(p));
        const alphabeticallySorted = [...positionsWithoutNumber].sort();
        const notAlphabeticallySorted =
            JSON.stringify(positionsWithoutNumber) !==
            JSON.stringify(alphabeticallySorted);

        if (numberNotAtEnd || notAlphabeticallySorted)
            return PositionValidationResult.InvalidOrder;

        if (validations.every((v) => v === PositionValidationResult.Valid)) {
            const groups = positions.map((p) =>
                tryParseInt(p) ? "<number>" : this._groupMap.get(p) ?? ""
            );

            const groupsSet = new Set<string>(groups);
            if (
                !groups.includes("DEFAULT_GROUP") &&
                groupsSet.size !== groups.length
            )
                return PositionValidationResult.InvalidGrouping;
        }

        return Math.max(...validations);
    }
}

export enum PositionValidationResult {
    Invalid = 0,
    InvalidOrder = 1,
    InvalidGrouping = 2,
    Valid = 100,
    Custom = 101,
}

class CodebookStandardValues {
    private readonly _name: CodebookName;
    private readonly _standardValues: Set<string>;

    public constructor(name: CodebookName, standardValues: Set<string>) {
        this._name = name;
        this._standardValues = standardValues;
    }

    public get values() {
        return this._standardValues;
    }

    public contains(tagValue: string): boolean {
        if (this._name === CodebookName.Position && tryParseInt(tagValue))
            return true;
        return this._standardValues.has(tagValue);
    }
}

class CodebookGroups {
    private readonly _name: CodebookName;
    private readonly _groups: Set<string>;

    public constructor(name: CodebookName, groups: Set<string>) {
        this._name = name;
        this._groups = groups;
    }

    public get values() {
        return this._groups;
    }

    public get count() {
        return this._groups.size;
    }

    public contains(group: string): boolean {
        return this._groups.has(group);
    }
}
