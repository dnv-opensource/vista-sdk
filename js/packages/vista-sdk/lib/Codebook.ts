import { CodebookName, CodebookNames } from "./CodebookName";
import { MetadataTag } from "./MetadataTag";
import { VIS } from "./VIS";
import { CodebookDto } from "./types/CodebookDto";
import { isNullOrWhiteSpace, tryParseInt } from "./util/util";

export class Codebook {
    public name: CodebookName;
    private readonly _groupMap: Map<string, string>;
    private readonly _standardValues: CodebookStandardValues;
    private readonly _groups: CodebookGroups;

    public constructor(dto: CodebookDto) {
        switch (dto.name) {
            case "positions":
                this.name = CodebookName.Position;
                break;
            case "calculations":
                this.name = CodebookName.Calculation;
                break;
            case "quantities":
                this.name = CodebookName.Quantity;
                break;
            case "states":
                this.name = CodebookName.State;
                break;
            case "contents":
                this.name = CodebookName.Content;
                break;
            case "commands":
                this.name = CodebookName.Command;
                break;
            case "types":
                this.name = CodebookName.Type;
                break;
            case "functional_services":
                this.name = CodebookName.FunctionalServices;
                break;
            case "maintenance_category":
                this.name = CodebookName.MaintenanceCategory;
                break;
            case "activity_type":
                this.name = CodebookName.ActivityType;
                break;
            case "detail":
                this.name = CodebookName.Detail;
                break;
            default:
                throw new Error("Invalid codebook name");
        }

        this._groupMap = new Map<string, string>();

        const data = Object.entries(dto.values).flatMap(([key, values]) =>
            values
                .map((v) => ({ group: key, value: v.trim() }))
                .filter((v) => v.value !== "<number>"),
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

    public get groupMap() {
        return this._groupMap;
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
            if (!VIS.isISOString(value)) return;
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
                `Invalid value for metadata tag: codebook=${this.name}, value=${value}`,
            );
        return tag;
    }

    public static isValidTag(
        name: CodebookName,
        value: string,
        codebook: Codebook,
    ) {
        if (!value || isNullOrWhiteSpace(value)) return false;

        if (name === CodebookName.Position) {
            if (this.validatePositionInternal(value, codebook) < 100)
                return false;
        } else {
            if (!VIS.isISOString(value)) return false;
        }
        return true;
    }

    public validatePosition(position: string) {
        return Codebook.validatePositionInternal(position, this);
    }
    private static validatePositionInternal(
        position: string,
        codebook: Codebook,
    ): PositionValidationResult {
        if (isNullOrWhiteSpace(position) || !VIS.isISOString(position))
            return PositionValidationResult.Invalid;

        if (position.trim().length != position.length)
            return PositionValidationResult.Invalid;

        if (codebook.standardValues.contains(position))
            return PositionValidationResult.Valid;

        if (typeof tryParseInt(position) === "number")
            return PositionValidationResult.Valid;

        if (!position.includes("-")) return PositionValidationResult.Custom;

        const positions = position.split("-");
        const validations: PositionValidationResult[] = [];
        for (const positionStr of positions) {
            validations.push(
                this.validatePositionInternal(positionStr, codebook),
            );
        }

        if (!!validations.find((v) => v < 100)) return Math.max(...validations);

        const numberNotAtEnd = !!positions.find(
            (pValue, pIndex) =>
                tryParseInt(pValue) && pIndex < positions.length - 1,
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
                tryParseInt(p) ? "<number>" : (codebook.groupMap.get(p) ?? ""),
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
