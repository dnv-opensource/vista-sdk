export type Error<T> = { type: T; message: string };

export abstract class ErrorBuilder<T> {
    private _errors: Error<T>[];
    abstract readonly predefinedMessages: Map<T, string>;

    constructor(errors?: Error<T>[]) {
        this._errors = errors ?? [];
    }

    public get errors() {
        return this._errors;
    }

    public push(error: T | Error<T>) {
        if (typeof error === "object" && !!error && "message" in error) {
            this._errors.push(error);
            return;
        }

        const message = this.predefinedMessages.get(error);
        if (!message)
            throw new Error("Couldnt find predefined message for: " + error);
        this._errors.push({ type: error, message });
    }
}
