export enum VisVersion {
    v3_4a = "3-4a",
    v3_5a = "3-5a",
    v3_6a = "3-6a",
    v3_7a = "3-7a",
    v3_8a = "3-8a",
    v3_9a = "3-9a",
    v3_10a = "3-10a",
}

export const allVisVersions: VisVersion[] = [
    VisVersion.v3_4a,
    VisVersion.v3_5a,
    VisVersion.v3_6a,
    VisVersion.v3_7a,
    VisVersion.v3_8a,
    VisVersion.v3_9a,
    VisVersion.v3_10a,
];

const versionToIndex = new Map<VisVersion, number>(
    allVisVersions.map((v, i) => [v, i]),
);
const stringToVersion = new Map<string, VisVersion>(
    allVisVersions.map((v) => [v as string, v]),
);

export class VisVersionExtension {
    public static toVersionString(
        version: VisVersion,
        builder?: string[],
    ): string {
        if (builder) {
            builder.push(version);
        }
        return version;
    }

    public static toString(version: VisVersion, builder?: string[]): string {
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
}

export class VisVersions {
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
}
