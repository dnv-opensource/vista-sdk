using System.Collections;
using System.Runtime.CompilerServices;

namespace Vista.SDK;

public sealed class Codebooks : IEnumerable<(CodebookName Name, Codebook Codebook)>
{
    public VisVersion VisVersion { get; }

    private readonly Codebook[] _codebooks;

    internal Codebooks(VisVersion version, CodebooksDto dto)
    {
        VisVersion = version;

        _codebooks = new Codebook[Enum.GetValues(typeof(CodebookName)).Length];

        foreach (var typeDto in dto.Items)
        {
            var type = new Codebook(typeDto);
            _codebooks[(int)type.Name - 1] = type;
        }

        var detailsCodebook = new Codebook(new CodebookDto("detail", new Dictionary<string, string[]>()));
        _codebooks[(int)detailsCodebook.Name - 1] = detailsCodebook;
    }

    public Codebook this[CodebookName name]
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            var index = (int)name - 1;
            if (index >= _codebooks.Length)
                throw new ArgumentException("Invalid codebook name: " + name);

            return _codebooks[index];
        }
    }

    public MetadataTag? TryCreateTag(CodebookName name, string? value) => this[name].TryCreateTag(value);

    public MetadataTag CreateTag(CodebookName name, string value) => this[name].CreateTag(value);

    public Codebook GetCodebook(CodebookName name) => this[name];

    public Enumerator GetEnumerator() => new Enumerator(_codebooks);

    IEnumerator<(CodebookName Name, Codebook Codebook)> IEnumerable<(
        CodebookName Name,
        Codebook Codebook
    )>.GetEnumerator() => new Enumerator(_codebooks);

    IEnumerator IEnumerable.GetEnumerator() => new Enumerator(_codebooks);

    public struct Enumerator : IEnumerator<(CodebookName Name, Codebook Codebook)>
    {
        private readonly Codebook[] _table;
        private int _index;

        internal Enumerator(Codebook[] table)
        {
            _table = table;
            _index = -1;
        }

        public bool MoveNext()
        {
            _index++;
            if ((uint)_index < (uint)_table.Length)
            {
                return true;
            }

            _index = _table.Length;
            return false;
        }

        public readonly (CodebookName Name, Codebook Codebook) Current
        {
            get
            {
                if ((uint)_index >= (uint)_table.Length)
                    throw new InvalidOperationException();

                var codebook = _table[_index];
                return (codebook.Name, codebook);
            }
        }

        object IEnumerator.Current => Current;

        public void Reset() => _index = -1;

        public void Dispose() { }
    }
}
