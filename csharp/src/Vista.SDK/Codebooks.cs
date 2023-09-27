using System.Collections;

namespace Vista.SDK;

public sealed class Codebooks : IEnumerable<(CodebookName Name, Codebook Codebook)>
{
    public VisVersion VisVersion { get; }

    private readonly Dictionary<CodebookName, Codebook> _codebooks;

    internal Codebooks(VisVersion version, CodebooksDto dto)
    {
        VisVersion = version;

        _codebooks = new Dictionary<CodebookName, Codebook>(dto.Items.Length);

        foreach (var typeDto in dto.Items)
        {
            var type = new Codebook(typeDto);
            _codebooks.Add(type.Name, type);
        }

        var detailsCodebook = new Codebook(new CodebookDto("detail", new Dictionary<string, string[]>()));
        _codebooks.Add(detailsCodebook.Name, detailsCodebook);
    }

    public Codebook this[CodebookName name]
    {
        get
        {
            if (!_codebooks.TryGetValue(name, out var codebook))
                throw new ArgumentException("Invalid codebook name: " + name);

            return codebook;
        }
    }

    public MetadataTag? TryCreateTag(CodebookName name, string? value) =>
        _codebooks.GetValueOrDefault(name)?.TryCreateTag(value);

    public MetadataTag CreateTag(CodebookName name, string value) => _codebooks[name].CreateTag(value);

    public Codebook GetCodebook(CodebookName name)
    {
        if (!_codebooks.TryGetValue(name, out var codebook))
            throw new ArgumentException("Invalid codebook name: " + name);

        return codebook;
    }

    public Enumerator GetEnumerator() => new Enumerator(this);

    IEnumerator<(CodebookName Name, Codebook Codebook)> IEnumerable<(
        CodebookName Name,
        Codebook Codebook
    )>.GetEnumerator() => new Enumerator(this);

    IEnumerator IEnumerable.GetEnumerator() => new Enumerator(this);

    public struct Enumerator : IEnumerator<(CodebookName Name, Codebook Codebook)>
    {
        private Dictionary<CodebookName, Codebook>.Enumerator _inner;

        public Enumerator(Codebooks parent)
        {
            _inner = parent._codebooks.GetEnumerator();
        }

        public (CodebookName Name, Codebook Codebook) Current => (_inner.Current.Key, _inner.Current.Value);

        object IEnumerator.Current => Current;

        public void Dispose() => _inner.Dispose();

        public bool MoveNext() => _inner.MoveNext();

        public void Reset() { }
    }
}
