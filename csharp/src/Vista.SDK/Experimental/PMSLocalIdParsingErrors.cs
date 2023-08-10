namespace Vista.SDK.Experimental;

public sealed record PMSLocalIdParsingErrorBuilder
{
    private readonly List<(PMSLocalIdParsingState type, string message)> _errors;
    private static Dictionary<PMSLocalIdParsingState, string> _predefinedErrorMessages =>
        SetPredefinedMessages();

    public static readonly PMSLocalIdParsingErrorBuilder Empty = new();

    internal PMSLocalIdParsingErrorBuilder() =>
        _errors = new List<(PMSLocalIdParsingState, string)>();

    internal PMSLocalIdParsingErrorBuilder AddError(PMSLocalIdParsingState state)
    {
        if (!_predefinedErrorMessages.TryGetValue(state, out var predefinedMessage))
            throw new Exception("Couldn't find predefined message for: " + state.ToString());

        _errors.Add((state, predefinedMessage));
        return this;
    }

    internal PMSLocalIdParsingErrorBuilder AddError(PMSLocalIdParsingState state, string? message)
    {
        if (string.IsNullOrWhiteSpace(message))
            return AddError(state);

        _errors.Add((state, message!));
        return this;
    }

    public bool HasError => _errors.Count > 0;

    internal static PMSLocalIdParsingErrorBuilder Create() => new();

    internal IReadOnlyCollection<(PMSLocalIdParsingState type, string message)> ErrorMessages =>
        _errors;

    private static Dictionary<PMSLocalIdParsingState, string> SetPredefinedMessages()
    {
        var parsingMap = new Dictionary<PMSLocalIdParsingState, string>();
        parsingMap.Add(PMSLocalIdParsingState.NamingRule, "Missing or invalid naming rule");
        parsingMap.Add(PMSLocalIdParsingState.VisVersion, "Missing or invalid vis version");
        parsingMap.Add(
            PMSLocalIdParsingState.PrimaryItem,
            "Invalid or missing Primary item. Local IDs require atleast primary item and 1 metadata tag."
        );
        parsingMap.Add(PMSLocalIdParsingState.SecondaryItem, "Invalid secondary item");
        parsingMap.Add(PMSLocalIdParsingState.ItemDescription, "Missing or invalid /meta prefix");
        parsingMap.Add(PMSLocalIdParsingState.MetaQuantity, "Invalid metadata tag: Quantity");
        parsingMap.Add(PMSLocalIdParsingState.MetaContent, "Invalid metadata tag: Content");
        parsingMap.Add(PMSLocalIdParsingState.MetaCommand, "Invalid metadata tag: Command");
        parsingMap.Add(PMSLocalIdParsingState.MetaPosition, "Invalid metadata tag: Position");
        parsingMap.Add(PMSLocalIdParsingState.MetaCalculation, "Invalid metadata tag: Calculation");
        parsingMap.Add(PMSLocalIdParsingState.MetaState, "Invalid metadata tag: State");
        parsingMap.Add(PMSLocalIdParsingState.MetaType, "Invalid metadata tag: Type");
        parsingMap.Add(PMSLocalIdParsingState.MetaDetail, "Invalid metadata tag: Detail");
        parsingMap.Add(PMSLocalIdParsingState.EmptyState, "Missing primary path or metadata");
        return parsingMap;
    }
}
