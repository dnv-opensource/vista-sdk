namespace Vista.SDK;

public enum CodebookName
{
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
    Detail = 11
}

public static class CodebookNames
{
    public static CodebookName FromPrefix(string? prefix) =>
        prefix switch
        {
            "pos" => CodebookName.Position,
            "qty" => CodebookName.Quantity,
            "calc" => CodebookName.Calculation,
            "state" => CodebookName.State,
            "cnt" => CodebookName.Content,
            "cmd" => CodebookName.Command,
            "type" => CodebookName.Type,
            "funct.svc" => CodebookName.FunctionalServices,
            "maint.cat" => CodebookName.MaintenanceCategory,
            "act.type" => CodebookName.ActivityType,
            "detail" => CodebookName.Detail,
            null => throw new ArgumentException(nameof(prefix)),
            _ => throw new ArgumentException("unknown prefix: " + prefix, nameof(prefix)),
        };
}
