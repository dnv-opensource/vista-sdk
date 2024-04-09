namespace Vista.SDK;

public static class VisVersionExtensions
{
    private static readonly uniffi.bindings.VisVersionExtensions _extensions = new();

    public static string ToVersionString(this VisVersion version) => _extensions.ToVersionString(version);
}

public static class VisVersions
{
    private static readonly uniffi.bindings.VisVersions _visVersions = new();

    public static VisVersion Parse(string input) => _visVersions.Parse(input);

    public static VisVersion[] All => _visVersions.All().ToArray();
}
