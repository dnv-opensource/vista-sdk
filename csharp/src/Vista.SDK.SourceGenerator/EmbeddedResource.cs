// Copyright @kzu
// License MIT
// copied from https://github.com/devlooped/ThisAssembly/blob/main/src/EmbeddedResource.cs

using System.IO.Compression;
using System.Reflection;
using System.Text.Json;

namespace Vista.SDK;

internal static class EmbeddedResource
{
    internal static List<string> GetVisVersions()
    {
        var assembly = Assembly.GetExecutingAssembly();

        var manifestResourceNames = GetResourceNames(assembly).Where(x => x.EndsWith(".gz")).ToArray();

        if (!manifestResourceNames.Any())
            throw new InvalidOperationException(
                $"Did not find required resources in assembly '{assembly.GetName().Name}'."
            );

        var visVersions = new List<string>();

        foreach (var manifestResourceName in manifestResourceNames)
        {
            if (manifestResourceName.Contains("gmod") && !manifestResourceName.Contains("versioning"))
            {
                var stream = GetDecompressedStream(assembly, manifestResourceName);
                var gmod =
                    JsonSerializer.Deserialize<GmodDto>(stream)
                    ?? throw new InvalidOperationException($"Could not deserialize Gmod {nameof(stream)}");
                visVersions.Add(gmod.VisVersion);
            }
        }
        return visVersions;
    }

    internal static GmodDto? GetGmod(string visVersion)
    {
        var assembly = Assembly.GetExecutingAssembly();

        var gmodResourceName = GetResourceNames(assembly)
            .Where(x => x.Contains("gmod") && x.EndsWith(".gz") && !x.Contains("versioning"))
            .Where(x => x.Contains(visVersion))
            .SingleOrDefault();

        if (gmodResourceName is null)
            return null;

        using var stream = GetDecompressedStream(assembly, gmodResourceName);

        return JsonSerializer.Deserialize<GmodDto>(stream);
    }

    internal static CodebooksDto? GetCodebooks(string visVersion)
    {
        var assembly = Assembly.GetExecutingAssembly();

        var codebooksResourceName = GetResourceNames(assembly)
            .Where(x => x.Contains("codebooks") && x.EndsWith(".gz"))
            .Where(x => x.Contains(visVersion))
            .SingleOrDefault();

        if (codebooksResourceName is null)
            return null;

        using var stream = GetDecompressedStream(assembly, codebooksResourceName);

        return JsonSerializer.Deserialize<CodebooksDto>(stream);
    }

    internal static UnmanagedMemoryStream GetStream(Assembly assembly, string resourceName) =>
        (UnmanagedMemoryStream)assembly.GetManifestResourceStream(resourceName)!;

    internal static string[] GetResourceNames(Assembly assembly) => assembly.GetManifestResourceNames().ToArray();

    internal static GZipStream GetDecompressedStream(Assembly assembly, string resourceName)
    {
        var stream = GetStream(assembly, resourceName);

        if (stream == null)
            throw new InvalidOperationException(
                $"Did not find required resource '{resourceName}' in assembly '{assembly.FullName}'."
            );

        return new GZipStream(stream, CompressionMode.Decompress, leaveOpen: false);
    }

    internal static Dictionary<string, GmodVersioningDto>? GetGmodVersioning()
    {
        var assembly = Assembly.GetExecutingAssembly();
        var baseName = assembly.GetName().Name;
        var gmodVersioningResourceNames = assembly
            .GetManifestResourceNames()
            .Where(x => x.Contains("gmod-vis-versioning") && x.EndsWith(".gz"))
            .ToList();

        if (gmodVersioningResourceNames is null)
            return null;

        var dtos = new Dictionary<string, GmodVersioningDto>();

        foreach (var resourceName in gmodVersioningResourceNames)
        {
            using var stream = GetDecompressedStream(assembly, resourceName);
            var gmodVersioning = JsonSerializer.Deserialize<GmodVersioningDto>(stream);
            if (gmodVersioning is null)
                continue;
            dtos.Add(gmodVersioning.VisVersion, gmodVersioning);
        }

        return dtos;
    }

    internal static LocationsDto? GetLocations(string visVersion)
    {
        var assembly = Assembly.GetExecutingAssembly();

        var locationsResourceName = GetResourceNames(assembly)
            .Where(x => x.Contains("locations") && x.EndsWith(".gz"))
            .Where(x => x.Contains(visVersion))
            .SingleOrDefault();

        if (locationsResourceName is null)
            return null;

        using var stream = GetDecompressedStream(assembly, locationsResourceName);

        return JsonSerializer.Deserialize<LocationsDto>(stream);
    }

    internal static DataChannelTypeNamesDto? GetDataChannelTypeNames(string version)
    {
        var assembly = Assembly.GetExecutingAssembly();

        var dataResourceName = GetResourceNames(assembly)
            .Where(x => x.Contains("data-channel-type-names") && x.EndsWith(".gz"))
            .Where(x => x.Contains("iso19848"))
            .Where(x => x.Contains(version))
            .SingleOrDefault();

        if (dataResourceName is null)
            return null;

        using var stream = GetDecompressedStream(assembly, dataResourceName);

        return JsonSerializer.Deserialize<DataChannelTypeNamesDto>(stream);
    }

    internal static FormatDataTypesDto? GetFormatDataTypes(string version)
    {
        var assembly = Assembly.GetExecutingAssembly();

        var dataResourceName = GetResourceNames(assembly)
            .Where(x => x.Contains("format-data-types") && x.EndsWith(".gz"))
            .Where(x => x.Contains("iso19848"))
            .Where(x => x.Contains(version))
            .SingleOrDefault();

        if (dataResourceName is null)
            return null;

        using var stream = GetDecompressedStream(assembly, dataResourceName);

        return JsonSerializer.Deserialize<FormatDataTypesDto>(stream);
    }
}
