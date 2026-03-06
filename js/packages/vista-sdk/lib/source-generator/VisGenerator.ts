export class VisGenerator {
    public static assembleVisVersionFile(versions: string[]) {
        return `${this.generateVisVersionEnum(versions)}

${this.generateVisVersionValues(versions)}

${this.generateLookupMaps()}

${this.generateVisVersionExtensionClass()}

${this.generateVisVersionsClass()}
`;
    }

    private static generateVisVersionEnum(versions: string[]) {
        const members = versions
            .map((v) => `    ${this.toVersionEnumKeyString(v)} = "${v}",`)
            .join("\n");
        return `export enum VisVersion {\n${members}\n}`;
    }

    private static generateVisVersionValues(versions: string[]) {
        const members = versions
            .map((v) => `    VisVersion.${this.toVersionEnumKeyString(v)},`)
            .join("\n");
        return `export const allVisVersions: VisVersion[] = [\n${members}\n];`;
    }

    private static generateLookupMaps() {
        return `const versionToIndex = new Map<VisVersion, number>(
    allVisVersions.map((v, i) => [v, i]),
);
const stringToVersion = new Map<string, VisVersion>(
    allVisVersions.map((v) => [v as string, v]),
);`;
    }

    private static generateVisVersionExtensionClass() {
        return `export class VisVersionExtension {
    public static toVersionString(
        version: VisVersion,
        builder?: string[],
    ): string {
        if (builder) {
            builder.push(version);
        }
        return version;
    }

    public static toString(
        version: VisVersion,
        builder?: string[],
    ): string {
        return this.toVersionString(version, builder);
    }

    public static isValid(version: VisVersion): boolean {
        return versionToIndex.has(version);
    }

    public static compare(a: VisVersion, b: VisVersion): number {
        const ai = versionToIndex.get(a);
        const bi = versionToIndex.get(b);
        if (ai === undefined || bi === undefined) {
            throw new Error("Invalid VisVersion");
        }
        return ai - bi;
    }

    public static lessThan(a: VisVersion, b: VisVersion): boolean {
        return this.compare(a, b) < 0;
    }

    public static lessThanOrEqual(a: VisVersion, b: VisVersion): boolean {
        return this.compare(a, b) <= 0;
    }

    public static greaterThan(a: VisVersion, b: VisVersion): boolean {
        return this.compare(a, b) > 0;
    }

    public static greaterThanOrEqual(a: VisVersion, b: VisVersion): boolean {
        return this.compare(a, b) >= 0;
    }

    public static equals(a: VisVersion, b: VisVersion): boolean {
        return a === b;
    }

    public static increment(version: VisVersion): VisVersion | undefined {
        const index = versionToIndex.get(version);
        if (index === undefined || index >= allVisVersions.length - 1) {
            return undefined;
        }
        return allVisVersions[index + 1];
    }
}`;
    }

    private static generateVisVersionsClass() {
        return `export class VisVersions {
    public static get all(): VisVersion[] {
        return allVisVersions;
    }

    public static parse(version: string): VisVersion {
        const v = this.tryParse(version);
        if (v === undefined) {
            throw new Error("Couldnt parse version string: " + version);
        }
        return v;
    }

    public static tryParse(version: string): VisVersion | undefined {
        return stringToVersion.get(version);
    }
}`;
    }

    private static toVersionEnumKeyString(v: string) {
        return `v${v.replace("-", "_")}`;
    }
}
