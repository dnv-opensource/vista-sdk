namespace Vista.SDK.Internal;

public sealed record LocalIdParsingErrorBuilder
{
    private readonly List<(ParsingState type, string message)> _errors;
    private static Dictionary<ParsingState, string> _predefinedErrorMessages =>
        SetPredefinedMessages();

    public static readonly LocalIdParsingErrorBuilder Empty = new LocalIdParsingErrorBuilder();

    internal LocalIdParsingErrorBuilder() => _errors = new List<(ParsingState, string)>();

    internal LocalIdParsingErrorBuilder AddError(ParsingState state)
    {
        if (!_predefinedErrorMessages.TryGetValue(state, out var predefinedMessage))
            throw new Exception("Couldn't find predefined message for: " + state.ToString());

        _errors.Add((state, predefinedMessage));
        return this;
    }

    internal LocalIdParsingErrorBuilder AddError(ParsingState state, string? message)
    {
        if (string.IsNullOrWhiteSpace(message))
            return AddError(state);

        _errors.Add((state, message!));
        return this;
    }

    public bool HasError => _errors.Count > 0;

    internal static LocalIdParsingErrorBuilder Create() => new LocalIdParsingErrorBuilder();

    internal IReadOnlyCollection<(ParsingState type, string message)> ErrorMessages => _errors;

    private static Dictionary<ParsingState, string> SetPredefinedMessages()
    {
        var parsingMap = new Dictionary<ParsingState, string>();
        parsingMap.Add(ParsingState.NamingRule, "Missing or invalid naming rule");
        parsingMap.Add(ParsingState.VisVersion, "Missing or invalid vis version");
        parsingMap.Add(ParsingState.PrimaryItem, "Missing or invalid primary item");
        parsingMap.Add(ParsingState.SecondaryItem, "Invalid secondary item");
        parsingMap.Add(ParsingState.ItemDescription, "Missing or invalid /meta prefix");
        parsingMap.Add(ParsingState.MetaQuantity, "Invalid metadata tag: Quantity");
        parsingMap.Add(ParsingState.MetaContent, "Invalid metadata tag: Content");
        parsingMap.Add(ParsingState.MetaCommand, "Invalid metadata tag: Command");
        parsingMap.Add(ParsingState.MetaPosition, "Invalid metadata tag: Position");
        parsingMap.Add(ParsingState.MetaCalculation, "Invalid metadata tag: Calculation");
        parsingMap.Add(ParsingState.MetaState, "Invalid metadata tag: State");
        parsingMap.Add(ParsingState.MetaType, "Invalid metadata tag: Type");
        parsingMap.Add(ParsingState.MetaDetail, "Invalid metadata tag: Detail");
        parsingMap.Add(ParsingState.EmptyState, "Missing primary path or metadata");
        return parsingMap;
    }
}
