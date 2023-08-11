using System.Collections;
using System.Text;

namespace Vista.SDK;

public sealed class ParsingErrors
    : IEnumerable<(string Type, string Message)>,
        IEquatable<ParsingErrors>
{
    public static readonly ParsingErrors Empty = new(Array.Empty<(string Type, string Message)>());

    private readonly (string Type, string Message)[] _errors = Array.Empty<(
        string Type,
        string Message
    )>();

    public bool HasErrors => _errors.Length > 0;

    public bool HasErrorType(string type) => _errors.Any(e => e.Type == type);

    internal ParsingErrors((string Type, string Message)[] errors)
    {
        _errors = errors;
    }

    public override string ToString()
    {
        if (_errors.Length == 0)
            return "Success";

        var builder = new StringBuilder("Parsing errors:\n", 32);
        foreach (var (type, message) in _errors)
        {
            builder.Append('\t');
            builder.Append(type);
            builder.Append(" - ");
            builder.Append(message);
            builder.Append('\n');
        }

        return builder.ToString();
    }

    public bool Equals(ParsingErrors? other)
    {
        if (other is null)
            return false;
        return _errors.SequenceEqual(other._errors);
    }

    public override bool Equals(object? obj) => Equals(obj as ParsingErrors);

    public static bool operator ==(ParsingErrors? left, ParsingErrors? right)
    {
        if (!ReferenceEquals(left, right))
        {
            if (left is not null)
                return left.Equals(right);
            return false;
        }
        return true;
    }

    public static bool operator !=(ParsingErrors? left, ParsingErrors? right) => !(left == right);

    public override int GetHashCode() => _errors.GetHashCode();

    public IEnumerator<(string Type, string Message)> GetEnumerator() => new Enumerator(_errors);

    IEnumerator IEnumerable.GetEnumerator() => new Enumerator(_errors);

    public struct Enumerator : IEnumerator<(string Type, string Message)>
    {
        private readonly (string Type, string Message)[] _data;
        private int _index;

        public Enumerator((string Type, string Message)[] data)
        {
            _data = data;
            _index = 0;
        }

        public (string Type, string Message) Current { readonly get; private set; }

        readonly object IEnumerator.Current => this.Current;

        public readonly void Dispose() { }

        public bool MoveNext()
        {
            var data = _data;

            if ((uint)_index < (uint)data.Length)
            {
                Current = data[_index];
                _index++;
                return true;
            }

            _index = data.Length + 1;
            Current = default;
            return false;
        }

        public void Reset()
        {
            _index = 0;
            Current = default;
        }
    }
}
