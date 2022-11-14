export interface GmodDto {
    visRelease: string;
    items: GmodNodeDto[];
    relations: [string, string][];
}

export interface GmodNodeDto {
    id?: string;
    category: string;
    type: string;
    code: string;
    name: string;
    commonName?: string | null;
    definition?: string | null;
    commonDefinition?: string | null;
    installSubstructure?: boolean | null;
    normalAssignmentNames?: { [key: string]: string };
}
