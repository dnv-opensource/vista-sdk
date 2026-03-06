import * as fs from "fs";
import * as path from "path";

/**
 * Root directory of the monorepo (4 levels up from this file).
 * All shared test data lives under this root.
 */
const REPO_ROOT = path.resolve(__dirname, "../../../..");

// ---------------------------------------------------------------------------
//  Test data types (mirroring C# VistaSDKTestData records)
// ---------------------------------------------------------------------------

export interface ImoNumberTestItem {
    value: string;
    success: boolean;
    output: string | null;
}

export interface ImoNumberTestData {
    imoNumbers: ImoNumberTestItem[];
}

export interface GmodPathTestItem {
    path: string;
    visVersion: string;
}

export interface GmodPathTestData {
    Valid: GmodPathTestItem[];
    Invalid: GmodPathTestItem[];
}

export interface InvalidLocalIdItem {
    input: string;
    expectedErrorMessages: string[];
}

export interface InvalidLocalIdTestData {
    InvalidLocalIds: InvalidLocalIdItem[];
}

export interface LocationTestItem {
    value: string;
    success: boolean;
    output: string | null;
    expectedErrorMessages: string[];
}

export interface LocationsTestData {
    locations: LocationTestItem[];
}

export interface IndividualizableSetItem {
    isFullPath: boolean;
    visVersion: string;
    path: string;
    expected: string[][] | null;
}

export interface PmodVersionData {
    fullPaths: string[];
    localIds: string[];
}

export interface VersioningTestItem {
    from: string;
    to: string;
    sourceFullPath: string;
    targetFullPath: string;
}

// ---------------------------------------------------------------------------
//  Data loading (inspired by C# GetData<T>)
// ---------------------------------------------------------------------------

function getTestDataPath(name: string): string {
    return path.join(REPO_ROOT, "testdata", `${name}.json`);
}

function getSchemaPath(name: string): string {
    return path.join(REPO_ROOT, "schemas", "json", name);
}

function getData<T>(testName: string): T {
    const filePath = getTestDataPath(testName);
    const json = fs.readFileSync(filePath, "utf-8");
    return JSON.parse(json) as T;
}

// ---------------------------------------------------------------------------
//  Test data accessors
// ---------------------------------------------------------------------------

export const TestData = {
    /** Path to LocalIds.txt (line-based, not JSON) */
    LocalIdsPath: path.join(REPO_ROOT, "testdata", "LocalIds.txt"),

    get ImoNumbers(): ImoNumberTestData {
        return getData<ImoNumberTestData>("ImoNumbers");
    },
    get GmodPaths(): GmodPathTestData {
        return getData<GmodPathTestData>("GmodPaths");
    },
    get InvalidLocalIds(): InvalidLocalIdTestData {
        return getData<InvalidLocalIdTestData>("InvalidLocalIds");
    },
    get Locations(): LocationsTestData {
        return getData<LocationsTestData>("Locations");
    },
    get IndividualizableSets(): IndividualizableSetItem[] {
        return getData<IndividualizableSetItem[]>("IndividualizableSets");
    },
    get PmodData(): Record<string, PmodVersionData> {
        return getData<Record<string, PmodVersionData>>("PmodData");
    },
    get VersioningTestCases(): Record<string, VersioningTestItem[]> {
        return getData<Record<string, VersioningTestItem[]>>(
            "VersioningTestCases",
        );
    },
} as const;

/** Paths to shared JSON schema files */
export const Schemas = {
    dir: path.join(REPO_ROOT, "schemas", "json"),

    DataChannelListSample: getSchemaPath("DataChannelList.sample.json"),
    DataChannelListSchema: getSchemaPath("DataChannelList.schema.json"),
    TimeSeriesDataSample: getSchemaPath("TimeSeriesData.sample.json"),
    TimeSeriesDataSchema: getSchemaPath("TimeSeriesData.schema.json"),
} as const;
