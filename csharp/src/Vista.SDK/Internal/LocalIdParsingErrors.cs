namespace Vista.SDK.Internal;

public sealed record LocalIdParsingErrorBuilder
{
    private readonly List<(LocalIdParsingState type, string message)> _errors;
    private static Dictionary<LocalIdParsingState, string> _predefinedErrorMessages =>
        SetPredefinedMessages();

    public static readonly LocalIdParsingErrorBuilder Empty = new();

    internal LocalIdParsingErrorBuilder() => _errors = new List<(LocalIdParsingState, string)>();

    internal LocalIdParsingErrorBuilder AddError(LocalIdParsingState state)
    {
        if (!_predefinedErrorMessages.TryGetValue(state, out var predefinedMessage))
            throw new Exception("Couldn't find predefined message for: " + state.ToString());

        _errors.Add((state, predefinedMessage));
        return this;
    }

    internal LocalIdParsingErrorBuilder AddError(LocalIdParsingState state, string? message)
    {
        if (string.IsNullOrWhiteSpace(message))
            return AddError(state);

        _errors.Add((state, message!));
        return this;
    }

    public bool HasError => _errors.Count > 0;

    internal static LocalIdParsingErrorBuilder Create() => new();

    internal IReadOnlyCollection<(LocalIdParsingState type, string message)> ErrorMessages =>
        _errors;

    private static Dictionary<LocalIdParsingState, string> SetPredefinedMessages()
    {
        var parsingMap = new Dictionary<LocalIdParsingState, string>();
        parsingMap.Add(LocalIdParsingState.NamingRule, "Missing or invalid naming rule");
        parsingMap.Add(LocalIdParsingState.VisVersion, "Missing or invalid vis version");
        parsingMap.Add(
            LocalIdParsingState.PrimaryItem,
            "Invalid or missing Primary item. Local IDs require atleast primary item and 1 metadata tag."
        );
        parsingMap.Add(LocalIdParsingState.SecondaryItem, "Invalid secondary item");
        parsingMap.Add(LocalIdParsingState.ItemDescription, "Missing or invalid /meta prefix");
        parsingMap.Add(LocalIdParsingState.MetaQuantity, "Invalid metadata tag: Quantity");
        parsingMap.Add(LocalIdParsingState.MetaContent, "Invalid metadata tag: Content");
        parsingMap.Add(LocalIdParsingState.MetaCommand, "Invalid metadata tag: Command");
        parsingMap.Add(LocalIdParsingState.MetaPosition, "Invalid metadata tag: Position");
        parsingMap.Add(LocalIdParsingState.MetaCalculation, "Invalid metadata tag: Calculation");
        parsingMap.Add(LocalIdParsingState.MetaState, "Invalid metadata tag: State");
        parsingMap.Add(LocalIdParsingState.MetaType, "Invalid metadata tag: Type");
        parsingMap.Add(LocalIdParsingState.MetaDetail, "Invalid metadata tag: Detail");
        parsingMap.Add(LocalIdParsingState.EmptyState, "Missing primary path or metadata");
        return parsingMap;
    }
}
