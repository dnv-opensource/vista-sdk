namespace Vista.SDK;

public abstract record ValidateResult
{
    public record Ok() : ValidateResult;

    public record Invalid(string[] Messages) : ValidateResult;
}
