export interface GmodNodeMetadata {
    readonly category: string;
    readonly type: string;
    readonly name: string;
    readonly commonName?: string;
    readonly definition?: string;
    readonly commonDefinition?: string;
    readonly installSubstructure?: boolean;
    readonly normalAssignmentNames: Map<string, string>;
}
