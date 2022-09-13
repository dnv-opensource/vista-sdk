import { tryParseInt } from "./util/util";

export class ImoNumber {
    private readonly _value: number;

    private constructor(value: number) {
        this._value = value;
    }

    public static create(value: string | number) {
        let num: number;
        if (typeof value === 'string') {
            const parsedValue = ImoNumber.tryParseInternal(value);
            if (parsedValue === undefined)
                throw new Error("Invalid IMO number: " + value);
            num = parsedValue;
        } else {
            num = value;
        }

        if (!this.isValid(num))
            throw new Error("Invalid IMO number: " + value);

        return new ImoNumber(num);
    }

    public static parse(value: string): ImoNumber {
        const imo = ImoNumber.tryParse(value);
        if (!imo) throw new Error("Invalid IMO number: " + value);
        return imo;
    }

    public static tryParse(value: string): ImoNumber | undefined {
        const num = ImoNumber.tryParseInternal(value);
        return num !== undefined ? new ImoNumber(num) : undefined;
    }

    private static tryParseInternal(value: string): number | undefined {
        const startsWithImo = value.startsWith("IMO");

        const num = tryParseInt(startsWithImo ? value.substring(3) : value);
        if (!num || !ImoNumber.isValid(num)) return;

        return num;
    }

    public get value() {
        return this._value;
    }

    public get isValid() {
        return ImoNumber.isValid(this._value);
    }

    public static isValid(imoNumber: number): boolean {
        // https://en.wikipedia.org/wiki/IMO_number
        // An IMO number is made of the three letters "IMO" followed by a seven-digit number.
        // This consists of a six-digit sequential unique number followed by a check digit.
        // The integrity of an IMO number can be verified using its check digit.
        // This is done by multiplying each of the first six digits by a factor
        // of 2 to 7 corresponding to their position from right to left.
        // The rightmost digit of this sum is the check digit.
        // For example, for IMO 9074729: (9×7) + (0×6) + (7×5) + (4×4) + (7×3) + (2×2) = 139
        if (imoNumber < 1000000 || imoNumber > 9999999) return false;

        const digits = imoNumber
            .toString()
            .split("")
            .reverse()
            .map((n) => +n);

        let checkDigit = 0;
        for (let i = 1; i < digits.length; i++) {
            checkDigit += (i + 1) * digits[i];
        }

        return imoNumber % 10 === checkDigit % 10;
    }

    public toString() {
        return `IMO${this._value}`;
    }
}
