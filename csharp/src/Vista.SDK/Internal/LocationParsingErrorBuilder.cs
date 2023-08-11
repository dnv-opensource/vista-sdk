namespace Vista.SDK.Internal;

internal sealed record LocationParsingErrorBuilder
{
    private readonly List<(LocationValidationResult name, string message)> _errors;

    internal static readonly LocationParsingErrorBuilder Empty = new();

    internal LocationParsingErrorBuilder() =>
        _errors = new List<(LocationValidationResult, string)>();

    internal LocationParsingErrorBuilder AddError(
        LocationValidationResult validationResult,
        string message
    )
    {
        _errors.Add((validationResult, message));
        return this;
    }

    internal bool HasError => _errors.Count > 0;

    internal static LocationParsingErrorBuilder Create() => new();

    internal ParsingErrors Build() =>
        _errors.Count == 0
            ? ParsingErrors.Empty
            : new ParsingErrors(_errors.Select((t, m) => (t.name.ToString(), t.message)).ToArray());
}
