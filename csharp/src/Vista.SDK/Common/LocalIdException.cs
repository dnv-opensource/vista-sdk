using System.Diagnostics.CodeAnalysis;

namespace Vista.SDK.Common;

public sealed class LocalIdException : Exception
{
    public LocalIdException(LocalIdError localIdError) : base("LocalId error")
        => LocalIdError = localIdError;

    public LocalIdError LocalIdError { get; }

    [DoesNotReturn]
    public static void Throw(LocalIdError error) => throw new LocalIdException(error);
}
