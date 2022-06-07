using Microsoft.Extensions.Caching.Memory;

namespace Vista.SDK;

public interface IVIS
{
    GmodDto GetGmodDto(VisVersion visVersion);

    Gmod GetGmod(VisVersion visVersion);

    CodebooksDto GetCodebooksDto(VisVersion visVersion);

    Codebooks GetCodebooks(VisVersion visversion);

    IReadOnlyDictionary<VisVersion, Codebooks> GetCodebooksMap(IEnumerable<VisVersion> visVersions);

    IReadOnlyDictionary<VisVersion, Gmod> GetGmodsMap(IEnumerable<VisVersion> visVersions);

    IEnumerable<VisVersion> GetVisVersions();

    GmodVersioningDto GetGmodVersioningDto();

    GmodVersioning GetGmodVersioning();
}

/// <summary>
/// Extensions for working with VIS versions
/// </summary>
public static partial class VisVersionExtensions { }

public sealed class VIS : IVIS
{
    private readonly MemoryCache _gmodDtoCache;
    private readonly MemoryCache _gmodCache;
    private readonly MemoryCache _codebooksDtoCache;
    private readonly MemoryCache _codebooksCache;
    private readonly MemoryCache _gmodVersioningDtoCache;
    private readonly MemoryCache _gmodVersioningCache;
    private const string _versioning = "versioning";

    public static readonly VIS Instance = new VIS();

    public VIS()
    {
        _gmodDtoCache = new MemoryCache(
            new MemoryCacheOptions
            {
                SizeLimit = 10,
                ExpirationScanFrequency = TimeSpan.FromHours(1),
            }
        );
        _gmodCache = new MemoryCache(
            new MemoryCacheOptions
            {
                SizeLimit = 10,
                ExpirationScanFrequency = TimeSpan.FromHours(1),
            }
        );
        _codebooksDtoCache = new MemoryCache(
            new MemoryCacheOptions
            {
                SizeLimit = 10,
                ExpirationScanFrequency = TimeSpan.FromHours(1),
            }
        );
        _codebooksCache = new MemoryCache(
            new MemoryCacheOptions
            {
                SizeLimit = 10,
                ExpirationScanFrequency = TimeSpan.FromHours(1),
            }
        );
        _gmodVersioningDtoCache = new MemoryCache(
            new MemoryCacheOptions
            {
                SizeLimit = 10,
                ExpirationScanFrequency = TimeSpan.FromHours(1),
            }
        );
        _gmodVersioningCache = new MemoryCache(
            new MemoryCacheOptions
            {
                SizeLimit = 10,
                ExpirationScanFrequency = TimeSpan.FromHours(1),
            }
        );
    }

    public GmodDto GetGmodDto(VisVersion visVersion)
    {
        return _gmodDtoCache.GetOrCreate(
            visVersion,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = EmbeddedResource.GetGmod(visVersion.ToVersionString());
                if (dto is null)
                    throw new Exception("Invalid state");

                return dto;
            }
        );
    }

    public Gmod GetGmod(VisVersion visVersion)
    {
        if (!visVersion.IsValid())
            throw new ArgumentException("Invalid VIS version: " + visVersion);

        return _gmodCache.GetOrCreate(
            visVersion,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = GetGmodDto(visVersion);

                return new Gmod(visVersion, dto);
            }
        );
    }

    public IReadOnlyDictionary<VisVersion, Gmod> GetGmodsMap(IEnumerable<VisVersion> visVersions)
    {
        var invalidVisVersions = visVersions.Where(v => !v.IsValid());
        if (invalidVisVersions.Any())
            throw new ArgumentException(
                "Invalid VIS versions provided: " + string.Join(", ", invalidVisVersions)
            );

        var versions = new HashSet<VisVersion>(visVersions);

        var gmods = versions.Select(v => (Version: v, Gmod: GetGmod(v))).ToArray();

        return gmods.ToDictionary(t => t.Version, t => t.Gmod);
    }

    public GmodVersioningDto GetGmodVersioningDto()
    {
        return _gmodVersioningDtoCache.GetOrCreate(
            _versioning,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = EmbeddedResource.GetGmodVersioning();

                if (dto is null)
                    throw new ArgumentException("Invalid state");

                return dto;
            }
        );
    }

    public GmodVersioning GetGmodVersioning()
    {
        return _gmodVersioningCache.GetOrCreate(
            _versioning,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = GetGmodVersioningDto();

                return new GmodVersioning(
                    dto,
                    (VisVersion targetVisVersion) => GetGmod(targetVisVersion)
                );
            }
        );
    }

    public CodebooksDto GetCodebooksDto(VisVersion visVersion)
    {
        return _codebooksDtoCache.GetOrCreate(
            visVersion,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = EmbeddedResource.GetCodebooks(visVersion.ToVersionString());
                if (dto is null)
                    throw new Exception("Invalid state");

                return dto;
            }
        );
    }

    public Codebooks GetCodebooks(VisVersion visVersion)
    {
        return _codebooksCache.GetOrCreate(
            visVersion,
            entry =>
            {
                entry.Size = 1;
                entry.SlidingExpiration = TimeSpan.FromHours(1);

                var dto = GetCodebooksDto(visVersion);

                return new Codebooks(visVersion, dto);
            }
        );
    }

    public IReadOnlyDictionary<VisVersion, Codebooks> GetCodebooksMap(
        IEnumerable<VisVersion> visVersions
    )
    {
        var invalidVisVersions = visVersions.Where(v => !v.IsValid());
        if (invalidVisVersions.Any())
            throw new ArgumentException(
                "Invalid VIS versions provided: " + string.Join(", ", invalidVisVersions)
            );

        var versions = new HashSet<VisVersion>(visVersions);

        var codebooks = versions.Select(v => (Version: v, Codebooks: GetCodebooks(v))).ToArray();

        return codebooks.ToDictionary(t => t.Version, t => t.Codebooks);
    }

    public IEnumerable<VisVersion> GetVisVersions()
    {
        return (VisVersion[])Enum.GetValues(typeof(VisVersion));
    }
}
