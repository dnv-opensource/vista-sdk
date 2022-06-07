export interface GmodNodeMetadata {
  category: string;
  type: string;
  name: string;
  commonName?: string;
  definition?: string;
  commonDefinition?: string;
  installSubstructure?: boolean;
  normalAssignmentNames: Map<string, string>;
}
