using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Intrinsics.X86;

namespace Vista.SDK.Internal;

internal sealed class NodeMap
{
    // internal const int Size = 30293;
    // private const ulong FastModMultiplier = ulong.MaxValue / Size + 1;

    internal readonly GmodNode[] _table;
    internal readonly int[] _seeds;

    // public NodeMap(VisVersion version, GmodDto dto)
    // {
    //     _table = new GmodNode[Size];
    //     _seeds = [];

    //     foreach (var node in dto.Items)
    //     {
    //         var hash = Hash(node.Code);
    //         // var index = FastMod(hash);
    //         var index = hash % Size;
    //         Debug.Assert(_table[index] is null, $"{node.Code} collided with {_table[index]?.Code}");
    //         _table[index] = new GmodNode(version, node);
    //     }
    // }

    public NodeMap(VisVersion version, GmodDto dto)
    {
        ulong size = 1;
        while (size < (ulong)dto.Items.Length)
            size *= 2;

        var h = new List<(int Index, uint Hash)>[size];

        for (int i = 0; i < h.Length; i++)
            h[i] = new List<(int Index, uint Hash)>();

        for (int i = 0; i < dto.Items.Length; i++)
        {
            var k = dto.Items[i].Code.AsSpan();
            var hash = Hash(k);
            h[hash & (size - 1)].Add((i + 1, hash));
        }

        Array.Sort(h, (i, j) => j.Count - i.Count);

        var indices = new int[size];
        var seeds = new int[size];

        int index;
        for (index = 0; index < h.Length && h[index].Count > 1; ++index)
        {
            var subKeys = h[index];

            uint seed = 0;
            var entries = new Dictionary<uint, int>();

            retry:
            {
                ++seed;

                foreach (var k in subKeys)
                {
                    var hash = Hashing.Seed(seed, k.Hash, size);

                    if (!entries.ContainsKey(hash) && indices[hash] == 0)
                    {
                        entries.Add(hash, k.Index);
                        continue;
                    }

                    entries.Clear();
                    goto retry;
                }
            }

            foreach (var entry in entries)
                indices[entry.Key] = entry.Value;

            seeds[subKeys[0].Hash & (size - 1)] = (int)seed;
        }

        var free = new List<int>();
        for (int i = 0; i < indices.Length; i++)
        {
            if (indices[i] == 0)
                free.Add(i);
            else
                --indices[i];
        }

        for (int i = 0; index < h.Length && h[index].Count > 0; i++)
        {
            var k = h[index++][0];
            var dst = free[i];
            indices[dst] = k.Index - 1;
            seeds[k.Hash & (size - 1)] = 0 - (dst + 1);
        }

        var values = new GmodNode[size];
        for (int i = 0; i < indices.Length; i++)
        {
            var idx = indices[i];
            values[i] = new GmodNode(version, dto.Items[idx]);
        }

        _table = values;
        _seeds = seeds;
    }

    public bool TryGetValue(ReadOnlySpan<char> code, [MaybeNullWhen(false)] out GmodNode node)
    {
        var hash = Hash(code);
        var size = _table.Length;
        var index = hash & (size - 1);
        var seed = _seeds[index];

        if (seed < 0)
        {
            node = _table[0 - seed - 1];
            return true;
        }

        index = Hashing.Seed((uint)seed, hash, (ulong)size);
        node = _table[index];
        return true;
    }

    internal static uint Hash(ReadOnlySpan<char> codeStr)
    {
        Debug.Assert(sizeof(char) == 2);
        var length = codeStr.Length * sizeof(char);
        var code = MemoryMarshal.CreateReadOnlySpan(
            ref Unsafe.As<char, byte>(ref MemoryMarshal.GetReference(codeStr)),
            length
        );

        uint hash = 0x01000193;
        for (int i = 0; i < code.Length; i += 2)
        {
            // if (Sse42.IsSupported)
            //     hash = Hashing.Crc32(hash, code[i]);
            // else
            //     hash = Hashing.LarssonHash(hash, code[i]);

            hash = Hashing.LarssonHash(hash, code[i]);
        }

        return hash;
    }

    internal static class Hashing
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint LarssonHash(uint hash, byte ch) => 37 * hash + ch;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint Fnv(uint hash, byte ch) => ((hash * 0x01000193) ^ ch) & 0xffffffff;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint Crc32(uint hash, byte ch) => Sse42.Crc32(hash, ch);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint Seed(uint seed, uint hash, ulong size)
        {
            var x = seed + hash;
            x ^= x >> 12;
            x ^= x << 25;
            x ^= x >> 27;

            return (uint)((x * 0x2545F4914F6CDD1DUL) & (size - 1));
        }
    }

    // [MethodImpl(MethodImplOptions.AggressiveInlining)]
    // private static uint FastMod(uint value)
    // {
    //     // We use modified Daniel Lemire's fastmod algorithm (https://github.com/dotnet/runtime/pull/406),
    //     // which allows to avoid the long multiplication if the divisor is less than 2**31.
    //     Debug.Assert(Size <= int.MaxValue);

    //     // This is equivalent of (uint)Math.BigMul(multiplier * value, divisor, out _). This version
    //     // is faster than BigMul currently because we only need the high bits.
    //     uint highbits = (uint)(((((FastModMultiplier * value) >> 32) + 1) * Size) >> 32);

    //     Debug.Assert(highbits == value % Size);
    //     return highbits;
    // }
}
