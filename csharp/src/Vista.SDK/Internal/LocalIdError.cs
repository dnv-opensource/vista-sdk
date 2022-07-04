namespace Vista.SDK.Internal;

//public sealed record LocalIdError(IEnumerable<string> Errors)
//{
//    public static implicit operator LocalIdError(string error) => new LocalIdError(new[] { error });
//}
public sealed record LocalIdError(IEnumerable<(ParsingState, string)> Errors)
{
    public static implicit operator LocalIdError((ParsingState type, string message) error) =>
        new LocalIdError(new[] { (error.type, error.message) });
}

public sealed record LocalIdErrorBuilder
{
    //private readonly List<string> _errors;
    private readonly List<(ParsingState type, string message)> _errors;
    private static Dictionary<ParsingState, string> _predefinedErrorMessages =>
        SetPredefinedMessages();

    //public LocalIdErrorBuilder() => _errors = new List<string>();
    public LocalIdErrorBuilder() => _errors = new List<(ParsingState, string)>();

    public IReadOnlyCollection<(ParsingState type, string message)> ErrorMessages => _errors;

    public LocalIdErrorBuilder AddError(ParsingState state, string message)
    {
        _errors.Add((state, message));
        return this;
    }

    public LocalIdErrorBuilder AddError(ParsingState state)
    {
        if (!_predefinedErrorMessages.TryGetValue(state, out var predefinedMessage))
            throw new Exception("Couldn't find predefined message for: " + state.ToString());

        _errors.Add((state, predefinedMessage));
        return this;
    }

    public bool HasError => _errors.Count > 0;

    public LocalIdError Build() => new LocalIdError(_errors);

    public void ThrowOnError()
    {
        if (HasError)
            LocalIdException.Throw(Build());
    }

    public static LocalIdErrorBuilder Create() => new LocalIdErrorBuilder();

    private static Dictionary<ParsingState, string> SetPredefinedMessages()
    {
        var parsingMap = new Dictionary<ParsingState, string>();
        parsingMap.Add(ParsingState.NamingRule, "Missing or invalid naming rule");
        parsingMap.Add(ParsingState.VisVersion, "Missing or invalid vis version");
        parsingMap.Add(ParsingState.PrimaryItem, "Missing or invalid primary item");
        parsingMap.Add(ParsingState.SecondaryItem, "Invalid secondary item");
        parsingMap.Add(ParsingState.ItemDescription, "Could not /meta in string");
        parsingMap.Add(ParsingState.MetaQty, "Invalid metadata tag: Quantity");
        parsingMap.Add(ParsingState.MetaCnt, "Invalid metadata tag: Content");
        parsingMap.Add(ParsingState.MetaCmd, "Invalid metadata tag: Command");
        parsingMap.Add(ParsingState.MetaPos, "Invalid metadata tag: Position");
        parsingMap.Add(ParsingState.MetaCalc, "Invalid metadata tag: Calculation");
        parsingMap.Add(ParsingState.MetaState, "Invalid metadata tag: State");
        parsingMap.Add(ParsingState.MetaType, "Invalid metadata tag: Type");
        parsingMap.Add(ParsingState.MetaDetail, "Invalid metadata tag: Detail");
        parsingMap.Add(ParsingState.EmptyState, "Missing primary path or metadata");
        return parsingMap;
    }
}
