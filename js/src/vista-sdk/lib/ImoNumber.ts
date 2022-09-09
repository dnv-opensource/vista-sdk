import { tryParseInt } from "./util/util";

export class ImoNumber {
    private readonly _value: number;

    public constructor(value: number | string, skipVerification = false) {
        if (typeof value === "string") {
            let parsedValue = skipVerification
                ? tryParseInt(/\+d/.exec(value)?.[0]) ?? 0
                : ImoNumber.parse(value).value;
            this._value = parsedValue;
        } else {
            if (!skipVerification && !ImoNumber.isValid(value))
                throw new Error("Invalid IMO number");
            this._value = value;
        }
    }

    public static parse(value: string): ImoNumber {
        const imo = this.tryParse(value);
        if (!imo) throw new Error("Invalid IMO number");
        return imo;
    }

    public static tryParse(value: string): ImoNumber | undefined {
        if (!value.startsWith("IMO")) return;

        const num = tryParseInt(value.substring(3));
        if (!num || !this.isValid(num)) return;

        return new ImoNumber(num);
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
        return this._value.toString();
    }
}
