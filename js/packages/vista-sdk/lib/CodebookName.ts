export enum CodebookName {
    Quantity = 1,
    Content = 2,
    Calculation = 3,
    State = 4,
    Command = 5,
    Type = 6,
    FunctionalServices = 7,
    MaintenanceCategory = 8,
    ActivityType = 9,
    Position = 10,
    Detail = 11,
}

export class CodebookNames {
    public static get names(): CodebookName[] {
        return Object.keys(CodebookName)
            .filter((v) => !isNaN(+v))
            .map((v) => +v as CodebookName);
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
            case "funct.svc":
                return CodebookName.FunctionalServices;
            case "maint.cat":
                return CodebookName.MaintenanceCategory;
            case "act.type":
                return CodebookName.ActivityType;

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
            case CodebookName.FunctionalServices:
                return "functionalService";
            case CodebookName.MaintenanceCategory:
                return "maintenanceCategory";
            case CodebookName.ActivityType:
                return "activityType";
            default:
                throw new Error(
                    `Unknown or invalid metadata tag name: ${name}`,
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
            case CodebookName.FunctionalServices:
                return "funct.svc";
            case CodebookName.MaintenanceCategory:
                return "maint.cat";
            case CodebookName.ActivityType:
                return "act.type";
            default:
                throw new Error(
                    "Unknown or invalid metadata tag name: " + name,
                );
        }
    }
}
