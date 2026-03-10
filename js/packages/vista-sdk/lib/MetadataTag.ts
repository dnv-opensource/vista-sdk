import { CodebookName, CodebookNames } from "./CodebookName";

export class MetadataTag {
    public readonly name: CodebookName;
    public readonly value: string;
    public isCustom: boolean;

    public get prefix() {
        return this.isCustom ? "~" : "-";
    }

    public constructor(name: CodebookName, value: string, isCustom = false) {
        this.name = name;
        this.value = value;
        this.isCustom = isCustom;
    }

    public equals(other?: MetadataTag): boolean {
        if (!other) return false;
        if (this.name !== other.name)
            throw new Error(`Cant compare ${this.name} with ${other.name}`);
        return this.value === other.value;
    }

    public toString(): string {
        return this.value;
    }

    public append(builder: string[]) {
        if (!!builder) {
            const prefix = CodebookNames.toPrefix(this.name);
            builder.push(prefix);
            builder.push(this.prefix);
            builder.push(this.value);
            builder.push("/");
        }
    }
}
