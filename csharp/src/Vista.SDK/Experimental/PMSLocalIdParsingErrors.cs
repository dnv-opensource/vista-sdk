namespace Vista.SDK.Experimental;

internal sealed record PMSLocalIdParsingErrorBuilder
{
    private readonly List<(PMSLocalIdParsingState type, string message)> _errors;
    private static Dictionary<PMSLocalIdParsingState, string> _predefinedErrorMessages =>
        SetPredefinedMessages();

    internal static readonly PMSLocalIdParsingErrorBuilder Empty = new();

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

    internal bool HasError => _errors.Count > 0;

    internal static PMSLocalIdParsingErrorBuilder Create() => new();

    internal ParsingErrors Build() =>
        _errors.Count == 0
            ? ParsingErrors.Empty
            : new ParsingErrors(_errors.Select((t, m) => (t.type.ToString(), t.message)).ToArray());

    private static Dictionary<PMSLocalIdParsingState, string> SetPredefinedMessages()
    {
        var parsingMap = new Dictionary<PMSLocalIdParsingState, string>
        {
            { PMSLocalIdParsingState.NamingRule, "Missing or invalid naming rule" },
            { PMSLocalIdParsingState.VisVersion, "Missing or invalid vis version" },
            {
                PMSLocalIdParsingState.PrimaryItem,
                "Invalid or missing Primary item. Local IDs require atleast primary item and 1 metadata tag."
            },
            { PMSLocalIdParsingState.SecondaryItem, "Invalid secondary item" },
            { PMSLocalIdParsingState.ItemDescription, "Missing or invalid /meta prefix" },
            { PMSLocalIdParsingState.MetaQuantity, "Invalid metadata tag: Quantity" },
            { PMSLocalIdParsingState.MetaContent, "Invalid metadata tag: Content" },
            { PMSLocalIdParsingState.MetaCommand, "Invalid metadata tag: Command" },
            { PMSLocalIdParsingState.MetaPosition, "Invalid metadata tag: Position" },
            { PMSLocalIdParsingState.MetaCalculation, "Invalid metadata tag: Calculation" },
            { PMSLocalIdParsingState.MetaState, "Invalid metadata tag: State" },
            { PMSLocalIdParsingState.MetaType, "Invalid metadata tag: Type" },
            { PMSLocalIdParsingState.MetaDetail, "Invalid metadata tag: Detail" },
            { PMSLocalIdParsingState.EmptyState, "Missing primary path or metadata" }
        };
        return parsingMap;
    }
}
