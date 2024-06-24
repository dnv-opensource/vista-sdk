namespace Vista.SDK;

public class VistaException(string Message) : Exception($"VistaException - {Message}");

public sealed class ValidationException(string Message) : VistaException($"Validation failed - Message='{Message}'")
{
    public ValidationException(ValidateResult.Invalid result)
        : this($"[{string.Join(", ", result.Messages)}]") { }
}
