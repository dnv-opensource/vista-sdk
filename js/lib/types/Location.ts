export interface RelativeLocation {
    code: string;
    name: string;
    definition?: string;
}

export enum LocationValidationResult {
    Invalid,
    InvalidCode,
    InvalidOrder,
    NullOrWhiteSpace,
    Valid,
}
