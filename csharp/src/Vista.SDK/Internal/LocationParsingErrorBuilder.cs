namespace Vista.SDK.Internal;

public sealed record LocationParsingErrorBuilder
{
    private readonly List<(LocationValidationResult name, string message)> _errors;

    public static readonly LocationParsingErrorBuilder Empty = new LocationParsingErrorBuilder();

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

    public bool HasError => _errors.Count > 0;

    internal static LocationParsingErrorBuilder Create() => new LocationParsingErrorBuilder();

    internal IReadOnlyCollection<(LocationValidationResult name, string message)> ErrorMessages =>
        _errors;
}
