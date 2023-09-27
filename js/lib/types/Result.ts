export type Result<T, E> = Ok<T> | Err<E>;

export class Ok<T> {
    public value;
    constructor(value: T) {
        this.value = value;
    }
}

export class Err<E> {
    value: E;
    constructor(value: E) {
        this.value = value;
    }
}
