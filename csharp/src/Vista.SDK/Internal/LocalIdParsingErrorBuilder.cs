namespace Vista.SDK.Internal;

internal sealed record LocalIdParsingErrorBuilder
{
    private readonly List<(LocalIdParsingState type, string message)> _errors;
    private static Dictionary<LocalIdParsingState, string> _predefinedErrorMessages =>
        SetPredefinedMessages();

    internal static readonly LocalIdParsingErrorBuilder Empty = new();

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

    internal bool HasError => _errors.Count > 0;

    internal static LocalIdParsingErrorBuilder Create() => new();

    public ParsingErrors Build() =>
        _errors.Count == 0
            ? ParsingErrors.Empty
            : new ParsingErrors(_errors.Select((t, m) => (t.type.ToString(), t.message)).ToArray());

    private static Dictionary<LocalIdParsingState, string> SetPredefinedMessages()
    {
        var parsingMap = new Dictionary<LocalIdParsingState, string>
        {
            { LocalIdParsingState.NamingRule, "Missing or invalid naming rule" },
            { LocalIdParsingState.VisVersion, "Missing or invalid vis version" },
            {
                LocalIdParsingState.PrimaryItem,
                "Invalid or missing Primary item. Local IDs require atleast primary item and 1 metadata tag."
            },
            { LocalIdParsingState.SecondaryItem, "Invalid secondary item" },
            { LocalIdParsingState.ItemDescription, "Missing or invalid /meta prefix" },
            { LocalIdParsingState.MetaQuantity, "Invalid metadata tag: Quantity" },
            { LocalIdParsingState.MetaContent, "Invalid metadata tag: Content" },
            { LocalIdParsingState.MetaCommand, "Invalid metadata tag: Command" },
            { LocalIdParsingState.MetaPosition, "Invalid metadata tag: Position" },
            { LocalIdParsingState.MetaCalculation, "Invalid metadata tag: Calculation" },
            { LocalIdParsingState.MetaState, "Invalid metadata tag: State" },
            { LocalIdParsingState.MetaType, "Invalid metadata tag: Type" },
            { LocalIdParsingState.MetaDetail, "Invalid metadata tag: Detail" },
            { LocalIdParsingState.EmptyState, "Missing primary path or metadata" }
        };
        return parsingMap;
    }
}
