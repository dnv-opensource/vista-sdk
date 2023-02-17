using System.Diagnostics;

namespace Vista.SDK.Experimental;

internal enum ParsingStateV2
{
    NamingEntity,
    IMONumber,
    NamingRule,
    VisVersion,
    PrimaryItem,
    SecondaryItem,
    ItemDescription,
    MetaPrefix,
    MetaQuantityTag,
    MetaContentTag,
    MetaCalculationTag,
    MetaStateTag,
    MetaCommandTag,
    MetaTypeTag,
    MetaPositionTag,
    MetaDetailTag,
}

internal ref struct LocalIdParser
{
    private readonly ReadOnlySpan<char> _buffer;
    private ParsingState _state;
    private int _currentIndex;
    private int _nextSlash;

    internal LocalIdParser(ReadOnlySpan<char> buffer)
    {
        _buffer = buffer;
        _state = ParsingState.NamingRule;
        _currentIndex = 1;
        _nextSlash = 0;
    }

    public LocalIdBuilder? Parse()
    {
        VisVersion version;
        Gmod? gmod = null;

        if (_buffer.Length == 0 || (_buffer.Length == 1 && _buffer[0] == '/'))
            return null;

        while (Peek(out var segment))
        {
            var segmentStr = segment.ToString();
            switch (_state)
            {
                case ParsingState.NamingRule:
                    Debug.Assert(segment.SequenceEqual(LocalId.NamingRule.AsSpan()));
                    Consume();
                    AdvanceState();
                    break;
                case ParsingState.VisVersion:
                    VisVersions.TryParse(segment, out version);
                    gmod = VIS.Instance.GetGmod(version);
                    Consume();
                    AdvanceState();
                    break;
                case ParsingState.PrimaryItem:
                    if (gmod is null)
                        throw new Exception("Invalid state");
                    ParsePrimaryItem(gmod, segment);
                    break;
            }
        }

        return null;
    }

    private void ParsePrimaryItem(Gmod gmod, ReadOnlySpan<char> segment)
    {
        var primaryItemStart = int.MinValue;

        do
        {
            var segmentStr = segment.ToString();

            var dashIndex = segment.IndexOf('-');
            var codeSegment = dashIndex == -1 ? segment : segment.Slice(0, dashIndex);
            if (!gmod.TryGetNode(codeSegment, out var node))
            {
                // Could be a single bad node, or we've reached the end of the path

                if (segment.StartsWith("sec".AsSpan())) { }
                else if (segment.StartsWith("meta".AsSpan())) { }
                else if (segment[0] == '~') { }
                break;
            }

            if (primaryItemStart == int.MinValue)
                primaryItemStart = _currentIndex;

            Consume();
        } while (Peek(out segment));
    }

    // Segment parsing and progression below

    private bool Peek(out ReadOnlySpan<char> segment)
    {
        if (_buffer.Length == 0)
        {
            segment = default;
            return false;
        }

        var nextSlash = _buffer.Slice(_currentIndex).IndexOf('/');
        if (nextSlash == -1)
        {
            segment = default;
            return false;
        }

        _nextSlash = _currentIndex + nextSlash;
        segment = _buffer.Slice(_currentIndex, nextSlash);
        return true;
    }

    private void Consume() => _currentIndex += (_nextSlash - _currentIndex) + 1;

    private ParsingState AdvanceState() => ++_state;
}
