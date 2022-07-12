namespace Vista.SDK;

public interface IUniversalIdBuilder
{
    bool IsValid { get; }

    ImoNumber? ImoNumber { get; }

    string ToString();
}
