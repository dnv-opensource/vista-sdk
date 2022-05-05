using Microsoft.Extensions.Caching.Memory;

namespace Vista.SDK;

public interface IVIS
{
    ValueTask<GmodDto> GetGmodDto(
        VisVersion visVersion,
        CancellationToken cancellationToken = default
    );

    ValueTask<Gmod> GetGmod(VisVersion visVersion, CancellationToken cancellationToken = default);

    ValueTask<CodebooksDto> GetCodebooksDto(
        VisVersion visVersion,
        CancellationToken cancellationToken = default
    );

    ValueTask<Codebooks> GetCodebooks(
        VisVersion visversion,
        CancellationToken cancellationToken = default
    );

    ValueTask<IReadOnlyDictionary<VisVersion, Codebooks>> GetCodebooksMap(
        IEnumerable<VisVersion> visVersions,
        CancellationToken cancellationToken = default
    );

    ValueTask<IReadOnlyDictionary<VisVersion, Gmod>> GetGmodsMap(
        IEnumerable<VisVersion> visVersions,
        CancellationToken cancellationToken = default
    );

    IEnumerable<VisVersion> GetVisVersions();

    //GmodNode Convert(VisVersion sourceVersion, GmodNode sourceNode, VisVersion targetVersion);

    //GmodPath Convert(VisVersion sourceVersion, GmodPath sourcePath, VisVersion targetVersion);
}

public static partial class VisVersionExtensions { }

public sealed class VIS : IVIS
{
    private readonly MemoryCache _gmodDtoCache;
    private readonly MemoryCache _gmodCache;
    private readonly MemoryCache _codebooksDtoCache;
    private readonly MemoryCache _codebooksCache;

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
    }

    public ValueTask<GmodDto> GetGmodDto(
        VisVersion visVersion,
        CancellationToken cancellationToken = default
    )
    {
        if (_gmodDtoCache.TryGetValue(visVersion, out GmodDto gmod))
            return new ValueTask<GmodDto>(gmod);

        return Get(visVersion);

        async ValueTask<GmodDto> Get(VisVersion visVersion)
        {
            return await _gmodDtoCache.GetOrCreate(
                visVersion,
                async entry =>
                {
                    entry.Size = 1;
                    entry.SlidingExpiration = TimeSpan.FromHours(1);

                    var dto = await EmbeddedResource.GetGmod(
                        visVersion.ToVersionString(),
                        cancellationToken
                    );
                    if (dto is null)
                        throw new Exception("Invalid state");

                    return dto;
                }
            );
        }
    }

    public ValueTask<Gmod> GetGmod(
        VisVersion visVersion,
        CancellationToken cancellationToken = default
    )
    {
        if (!visVersion.IsValid())
            throw new ArgumentException("Invalid VIS version: " + visVersion);

        if (_gmodCache.TryGetValue(visVersion, out Gmod gmod))
            return new ValueTask<Gmod>(gmod);

        return Get(visVersion);

        async ValueTask<Gmod> Get(VisVersion visVersion)
        {
            return await _gmodCache.GetOrCreateAsync(
                visVersion,
                async entry =>
                {
                    entry.Size = 1;
                    entry.SlidingExpiration = TimeSpan.FromHours(1);

                    var dto = await GetGmodDto(visVersion);

                    return new Gmod(visVersion, dto);
                }
            );
        }
    }

    public async ValueTask<IReadOnlyDictionary<VisVersion, Gmod>> GetGmodsMap(
        IEnumerable<VisVersion> visVersions,
        CancellationToken cancellationToken = default
    )
    {
        var invalidVisVersions = visVersions.Where(v => !v.IsValid());
        if (invalidVisVersions.Any())
            throw new ArgumentException(
                "Invalid VIS versions provided: " + string.Join(", ", invalidVisVersions)
            );

        var versions = new HashSet<VisVersion>(visVersions);

        var tasks = versions
            .Select(v => (Version: v, Task: GetGmod(v, cancellationToken).AsTask()))
            .ToArray();
        await Task.WhenAll(tasks.Select(t => t.Task));

        return tasks.ToDictionary(t => t.Version, t => t.Task.Result);
    }

    public ValueTask<CodebooksDto> GetCodebooksDto(
        VisVersion visVersion,
        CancellationToken cancellationToken = default
    )
    {
        if (_codebooksDtoCache.TryGetValue(visVersion, out CodebooksDto codebooks))
            return new ValueTask<CodebooksDto>(codebooks);

        return Get(visVersion);

        async ValueTask<CodebooksDto> Get(VisVersion visVersion)
        {
            return await _codebooksDtoCache.GetOrCreateAsync(
                visVersion,
                async entry =>
                {
                    entry.Size = 1;
                    entry.SlidingExpiration = TimeSpan.FromHours(1);

                    var dto = await EmbeddedResource.GetCodebooks(
                        visVersion.ToVersionString(),
                        cancellationToken
                    );
                    if (dto is null)
                        throw new Exception("Invalid state");

                    return dto;
                }
            );
        }
    }

    public ValueTask<Codebooks> GetCodebooks(
        VisVersion visversion,
        CancellationToken cancellationToken = default
    )
    {
        if (_codebooksCache.TryGetValue(visversion, out Codebooks codebooks))
            return new ValueTask<Codebooks>(codebooks);

        return Get(visversion);

        async ValueTask<Codebooks> Get(VisVersion visVersion)
        {
            return await _codebooksCache.GetOrCreateAsync(
                visversion,
                async entry =>
                {
                    entry.Size = 1;
                    entry.SlidingExpiration = TimeSpan.FromHours(1);

                    var dto = await GetCodebooksDto(visVersion);

                    return new Codebooks(visVersion, dto);
                }
            );
        }
    }

    public async ValueTask<IReadOnlyDictionary<VisVersion, Codebooks>> GetCodebooksMap(
        IEnumerable<VisVersion> visVersions,
        CancellationToken cancellationToken = default
    )
    {
        var invalidVisVersions = visVersions.Where(v => !v.IsValid());
        if (invalidVisVersions.Any())
            throw new ArgumentException(
                "Invalid VIS versions provided: " + string.Join(", ", invalidVisVersions)
            );

        var versions = new HashSet<VisVersion>(visVersions);

        var tasks = versions
            .Select(v => (Version: v, Task: GetCodebooks(v, cancellationToken).AsTask()))
            .ToArray();
        await Task.WhenAll(tasks.Select(t => t.Task));

        return tasks.ToDictionary(t => t.Version, t => t.Task.Result);
    }

    public IEnumerable<VisVersion> GetVisVersions()
    {
        return (VisVersion[])Enum.GetValues(typeof(VisVersion));
    }
}
