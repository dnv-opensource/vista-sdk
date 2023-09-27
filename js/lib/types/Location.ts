import { Location } from "../Location";

export interface RelativeLocation {
    code: string;
    name: string;
    definition?: string;
    location: Location;
}

export enum LocationValidationResult {
    Invalid,
    InvalidCode,
    InvalidOrder,
    NullOrWhiteSpace,
    Valid,
}
