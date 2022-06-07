export enum CodebookName {
    Position = 1,
    Quantity = 2,
    Calculation = 3,
    State = 4,
    Content = 5,
    Command = 6,
    Type = 7,
    Detail = 8,
}

export class CodebookNames {
    public static get names(): CodebookName[] {
        return Object.keys(CodebookName)
            .filter((v) => !isNaN(+v))
            .map((v) => +v as CodebookName);
    }

    public static fromString(name?: string): CodebookName {
        switch (name?.toLocaleLowerCase()) {
            case "position":
            case "positions":
                return CodebookName.Position;
            case "calculation":
            case "calculations":
                return CodebookName.Calculation;
            case "quantity":
            case "quantities":
                return CodebookName.Quantity;
            case "state":
            case "states":
                return CodebookName.State;
            case "content":
            case "contents":
                return CodebookName.Content;
            case "command":
            case "commands":
                return CodebookName.Command;
            case "type":
            case "types":
                return CodebookName.Type;
            case "detail":
                return CodebookName.Detail;
            default:
                throw new Error(`Unknown or invalid metadata tag: ${name}`);
        }
    }

    public static fromPrefix(prefix?: string): CodebookName {
        switch (prefix) {
            case "pos":
                return CodebookName.Position;
            case "calc":
                return CodebookName.Calculation;
            case "qty":
                return CodebookName.Quantity;
            case "state":
                return CodebookName.State;
            case "cnt":
                return CodebookName.Content;
            case "cmd":
                return CodebookName.Command;
            case "type":
                return CodebookName.Type;
            case "detail":
                return CodebookName.Detail;
            default:
                throw new Error(`Unknown or invalid prefix: ${prefix}`);
        }
    }

    public static toString(name: CodebookName): string {
        switch (name) {
            case CodebookName.Position:
                return "position";
            case CodebookName.Calculation:
                return "calculation";
            case CodebookName.Quantity:
                return "quantity";
            case CodebookName.State:
                return "state";
            case CodebookName.Content:
                return "content";
            case CodebookName.Command:
                return "command";
            case CodebookName.Type:
                return "type";
            case CodebookName.Detail:
                return "detail";
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${name}`
                );
        }
    }

    public static toPrefix(name: CodebookName): string {
        switch (name) {
            case CodebookName.Position:
                return "pos";
            case CodebookName.Calculation:
                return "calc";
            case CodebookName.Quantity:
                return "qty";
            case CodebookName.State:
                return "state";
            case CodebookName.Content:
                return "cnt";
            case CodebookName.Command:
                return "cmd";
            case CodebookName.Type:
                return "type";
            case CodebookName.Detail:
                return "detail";
            default:
                throw new Error(
                    "Unknown or invalid metadata tag name: " + name
                );
        }
    }
}
