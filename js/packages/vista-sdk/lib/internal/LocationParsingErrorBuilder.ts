import { LocationValidationResult } from "../types/Location";
import { Error, ErrorBuilder } from "./ErrorBuilder";

export class LocationParsingErrorBuilder extends ErrorBuilder<LocationValidationResult> {
    readonly predefinedMessages: Map<LocationValidationResult, string>;

    public static create(): LocationParsingErrorBuilder {
        return new LocationParsingErrorBuilder();
    }

    constructor(errors?: Error<LocationValidationResult>[]) {
        super(errors);
        this.predefinedMessages = new Map<LocationValidationResult, string>();
    }
    public get hasError(): boolean {
        return this.errors.length > 0;
    }
}
