using System.Collections;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
#if NET6_0_OR_GREATER
using System.Runtime.Intrinsics.X86;
#endif

namespace Vista.SDK.Internal;

internal sealed class ChdDictionary<TValue>
{
    internal readonly (string Key, TValue Value)[] _table;
    internal readonly int[] _seeds;

    public ChdDictionary(IReadOnlyList<(string Key, TValue Value)> items)
    {
        ulong size = 1;
        while (size < (ulong)items.Count)
            size *= 2;

        size *= 2;

        var h = new List<(int Index, uint Hash)>[size];

        for (int i = 0; i < h.Length; i++)
            h[i] = new List<(int Index, uint Hash)>();

        for (int i = 0; i < items.Count; i++)
        {
            var k = items[i].Key.AsSpan();
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

        var table = new (string Key, TValue Value)[size];

        var free = new List<int>();
        for (int i = 0; i < indices.Length; i++)
        {
            if (indices[i] == 0)
            {
                free.Add(i);
            }
            else
            {
                --indices[i];
                table[i] = items[indices[i]];
            }
        }

        for (int i = 0; index < h.Length && h[index].Count > 0; i++)
        {
            var k = h[index++][0];
            var dst = free[i];
            indices[dst] = k.Index - 1;
            table[dst] = items[k.Index - 1];

            seeds[k.Hash & (size - 1)] = 0 - (dst + 1);
        }

        _table = table;
        _seeds = seeds;
    }

    public TValue this[ReadOnlySpan<char> key]
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            if (!TryGetValue(key, out var value))
                ThrowHelper.ThrowKeyNotFoundException(key);

            return value;
        }
    }

    public Enumerator GetEnumerator() => new Enumerator(_table);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool TryGetValue(ReadOnlySpan<char> key, [MaybeNullWhen(false)] out TValue value)
    {
        if (key.IsEmpty)
        {
            value = default;
            return false;
        }

        var hash = Hash(key);
        var size = _table.Length;
        var index = hash & (size - 1);
        var seed = _seeds[index];

        if (seed < 0)
        {
            ref readonly var kvp = ref _table[0 - seed - 1];
            if (!key.SequenceEqual(kvp.Key.AsSpan()))
            {
                value = default;
                return false;
            }

            value = kvp.Value;
            return true;
        }
        else
        {
            index = Hashing.Seed((uint)seed, hash, (ulong)size);
            ref readonly var kvp = ref _table[index];
            if (!key.SequenceEqual(kvp.Key.AsSpan()))
            {
                value = default;
                return false;
            }
            value = kvp.Value;
            return true;
        }
    }

    public struct Enumerator : IEnumerator<KeyValuePair<string, TValue>>
    {
        private readonly (string Key, TValue Value)[] _table;
        private int _index;

        internal Enumerator((string Key, TValue Value)[] table)
        {
            _table = table;
            _index = -1;
        }

        public bool MoveNext()
        {
            do
            {
                _index++;
            } while ((uint)_index < (uint)_table.Length && _table[_index].Key is null);

            if ((uint)_index < (uint)_table.Length)
            {
                return true;
            }

            _index = _table.Length;
            return false;
        }

        public readonly KeyValuePair<string, TValue> Current
        {
            get
            {
                if ((uint)_index >= (uint)_table.Length)
                    ThrowHelper.ThrowInvalidOperationException();

                var (key, value) = _table[_index];
                return new KeyValuePair<string, TValue>(key, value);
            }
        }

        object IEnumerator.Current => Current;

        public void Reset() => _index = -1;

        public void Dispose() { }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    internal static uint Hash(ReadOnlySpan<char> key)
    {
        Debug.Assert(sizeof(char) == 2);
        var length = key.Length * sizeof(char);
        ref var curr = ref Unsafe.As<char, byte>(ref MemoryMarshal.GetReference(key));

        uint hash = 0x811C9DC5;
        while (length > 0)
        {
#if NET6_0_OR_GREATER
            if (Sse42.IsSupported)
                hash = Hashing.Crc32(hash, curr);
            else
                hash = Hashing.Fnv(hash, curr);
#else
            hash = Hashing.Fnv(hash, curr);
#endif

            curr = ref Unsafe.Add(ref curr, 2);
            length -= 2;
        }

        return hash;
    }

    internal static class Hashing
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint LarssonHash(uint hash, byte ch) => 37 * hash + ch;

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint Fnv(uint hash, byte ch) => (ch ^ hash) * 0x01000193;

#if NET6_0_OR_GREATER
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        internal static uint Crc32(uint hash, byte ch) => Sse42.Crc32(hash, ch);
#endif

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

    internal static class ThrowHelper
    {
        [DoesNotReturn]
        public static void ThrowKeyNotFoundException(ReadOnlySpan<char> key) =>
            throw new KeyNotFoundException($"No value associated to key: {key.ToString()}");

        [DoesNotReturn]
        public static void ThrowInvalidOperationException() => throw new InvalidOperationException();
    }
}

// internal struct AHash
// {
//     private const int ROT = 23;
//     private const ulong MULTIPLE = 6364136223846793005UL;

//     private static readonly ulong[][] _fixedSeeds = new ulong[][]
//     {
//         new ulong[] { 0x243f_6a88_85a3_08d3, 0x1319_8a2e_0370_7344, 0xa409_3822_299f_31d0, 0x082e_fa98_ec4e_6c89, },
//         new ulong[] { 0x4528_21e6_38d0_1377, 0xbe54_66cf_34e9_0c6c, 0xc0ac_29b7_c97c_50dd, 0x3f84_d5b5_b547_0917, }
//     };
//     private ulong _buffer;
//     private ulong _pad;
//     private ulong[] _extra_keys;

//     public AHash(ulong seed)
//     {
//         var fixedSeeds = _fixedSeeds;
//         var a = fixedSeeds[0];
//         var b = fixedSeeds[1];
//         var c = seed;

//         var k0 = a[0];
//         var k1 = a[1];
//         var k2 = a[2];
//         var k3 = a[3];

//         _buffer = k0;
//         _pad = k1;
//         _extra_keys =  [k2, k3];
//     }

//     public ulong Hash(ReadOnlySpan<char> inputStr)
//     {
//         Debug.Assert(sizeof(char) == 2);
//         var length = inputStr.Length * sizeof(char);
//         var input = MemoryMarshal.CreateReadOnlySpan(
//             ref Unsafe.As<char, byte>(ref MemoryMarshal.GetReference(inputStr)),
//             length
//         );

//         _buffer = (_buffer + ((ulong)length)) * MULTIPLE;

//         if (input.Length > 8)
//         {

//             Span<ulong> value = stackalloc ulong[2];
//             value[0] = Unsafe.ReadUnaligned<ulong>(ref MemoryMarshal.GetReference(input));
//             value[1] = Unsafe.ReadUnaligned<ulong>(ref MemoryMarshal.GetReference(input[^8..]));
//             LargeUpdate(value);
//         }
//         else
//         {
//             Span<ulong> value = stackalloc ulong[2];
//             ReadSmall(input, value);
//             LargeUpdate(value);
//         }

//         var rot = (int)(_buffer & 63);
//         return BitOperations.RotateLeft(FoldedMultiply(_buffer, _pad), rot);
//     }

//     private void LargeUpdate(Span<ulong> block)
//     {
//         var s = block[0] ^ _extra_keys[0];
//         var by = block[1] ^ _extra_keys[1];
//         var combined = FoldedMultiply(s, by);
//         _buffer = BitOperations.RotateLeft((_buffer + _pad) ^ combined, ROT);
//     }

//     private static ulong FoldedMultiply(ulong s, ulong by)
//     {
//         var result = ((UInt128)s) * ((UInt128)by);
//         return ((ulong)(result & 0xffff_ffff_ffff_ffff)) ^ ((ulong)(result >> 64));
//     }

//     private static void ReadSmall(ReadOnlySpan<byte> data, Span<ulong> result)
//     {
//         Debug.Assert(data.Length <= 8);

//         if (data.Length >= 4)
//         {
//             result[0] = Unsafe.ReadUnaligned<uint>(ref MemoryMarshal.GetReference(data));
//             result[1] = Unsafe.ReadUnaligned<uint>(ref MemoryMarshal.GetReference(data[^4..]));
//         }
//         else
//         {
//             result[0] = Unsafe.ReadUnaligned<ushort>(ref MemoryMarshal.GetReference(data));
//             result[1] = data[^1];
//         }
//     }
// }
