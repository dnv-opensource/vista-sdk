namespace Vista.SDK;

public interface IVIS
{
    Gmod GetGmod(VisVersion visVersion);

    // Codebooks GetCodebooks(VisVersion visversion);

    // Locations GetLocations(VisVersion visversion);

    // IReadOnlyDictionary<VisVersion, Codebooks> GetCodebooksMap(IEnumerable<VisVersion> visVersions);

    IReadOnlyDictionary<VisVersion, Gmod> GetGmodsMap(IEnumerable<VisVersion> visVersions);

    // IReadOnlyDictionary<VisVersion, Locations> GetLocationsMap(IEnumerable<VisVersion> visVersions);

    IEnumerable<VisVersion> GetVisVersions();

    GmodNode? ConvertNode(VisVersion sourceVersion, GmodNode sourceNode, VisVersion targetVersion);

    GmodPath? ConvertPath(VisVersion sourceVersion, GmodPath sourcePath, VisVersion targetVersion);
}

public sealed class VIS : IVIS
{
    private readonly uniffi.bindings.Vis _internal;

    public static readonly VIS Instance = new VIS();

    public VIS()
    {
        _internal = uniffi.bindings.Vis.Instance();
    }

    public Gmod GetGmod(VisVersion visVersion)
    {
        return Gmod.FromBindings(_internal.GetGmod(version: visVersion));
    }

    public IReadOnlyDictionary<VisVersion, Gmod> GetGmodsMap(IEnumerable<VisVersion> visVersions)
    {
        throw new NotImplementedException();
    }

    public IEnumerable<VisVersion> GetVisVersions()
    {
        throw new NotImplementedException();
    }

    public GmodNode? ConvertNode(VisVersion sourceVersion, GmodNode sourceNode, VisVersion targetVersion)
    {
        throw new NotImplementedException();
    }

    public GmodPath? ConvertPath(VisVersion sourceVersion, GmodPath sourcePath, VisVersion targetVersion)
    {
        throw new NotImplementedException();
    }
}
