/**
 * GmodVersioning Data Transfer Objects (DTOs)
 *
 * These types represent the JSON structure of versioning data files
 * (gmod-vis-versioning-*.json.gz)
 */

/**
 * DTO for GMOD node conversion rules
 */
export interface GmodNodeConversionDto {
    operations: string[];
    source: string;
    target?: string | null;
    oldAssignment?: string | null;
    newAssignment?: string | null;
    deleteAssignment?: boolean | null;
}

/**
 * DTO for a single version's conversion data
 */
export interface GmodVersioningDto {
    visRelease: string;
    items: Record<string, GmodNodeConversionDto>;
}

/**
 * Conversion operation types
 */
export enum ConversionType {
    ChangeCode = "changeCode",
    Merge = "merge",
    Move = "move",
    AssignmentChange = "assignmentChange",
    AssignmentDelete = "assignmentDelete",
}

/**
 * Parsed node conversion data
 */
export interface GmodNodeConversion {
    operations: Set<ConversionType>;
    source: string;
    target?: string;
    oldAssignment?: string;
    newAssignment?: string;
    deleteAssignment?: boolean;
}
