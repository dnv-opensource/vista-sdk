using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Vista.SDK.Benchmarks.Internal;

[Config(typeof(Config))]
public class ShortStringHash
{
    [Params("400", "H346.11112")]
    public string Input { get; set; }

    [Benchmark(Baseline = true)]
    public int Bcl() => Input.GetHashCode();

    [Benchmark]
    public int BclOrd() => Hashing.GetHashCodeOrdinal(Input);

    [Benchmark]
    public uint Larsson() => Hash<LarssonHasher>(Input);

    [Benchmark]
    public uint Crc32Intrinsic() => Hash<Crc32IntrinsicHasher>(Input);

    [Benchmark]
    public uint Fnv() => Hash<FnvHasher>(Input);

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static uint Hash<THasher>(string inputStr)
        where THasher : struct, IHasher
    {
        var key = inputStr.AsSpan();

        var length = key.Length * sizeof(char);
        ref var curr = ref Unsafe.As<char, byte>(ref MemoryMarshal.GetReference(key));

        uint hash = 0x811C9DC5;
        while (length > 0)
        {
            hash = THasher.Hash(hash, curr);

            curr = ref Unsafe.Add(ref curr, 2);
            length -= 2;
        }

        return hash;
    }

    interface IHasher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        static abstract uint Hash(uint hash, byte ch);
    }

    readonly struct LarssonHasher : IHasher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint Hash(uint hash, byte ch) =>
            SDK.Internal.ChdDictionary<GmodNode>.Hashing.LarssonHash(hash, ch);
    }

    readonly struct Crc32IntrinsicHasher : IHasher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint Hash(uint hash, byte ch) => SDK.Internal.ChdDictionary<GmodNode>.Hashing.Crc32(hash, ch);
    }

    readonly struct FnvHasher : IHasher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint Hash(uint hash, byte ch) => SDK.Internal.ChdDictionary<GmodNode>.Hashing.Fnv(hash, ch);
    }

    internal static class Hashing
    {
        // TODO https://github.com/dotnet/runtime/issues/77679:
        // Replace these once non-randomized implementations are available.

        // Lengths 0 to 4 are unrolled manually due to their commonality, especially
        // with the substring-based dictionary/sets that use substrings with length <= 4.

        private const uint Hash1Start = (5381 << 16) + 5381;
        private const uint Factor = 1_566_083_941;

        public static unsafe int GetHashCodeOrdinal(ReadOnlySpan<char> s)
        {
            int length = s.Length;
            fixed (char* src = &MemoryMarshal.GetReference(s))
            {
                uint hash1,
                    hash2;
                switch (length)
                {
                    case 0:
                        return (int)(Hash1Start + unchecked(Hash1Start * Factor));

                    case 1:
                        hash2 = (BitOperations.RotateLeft(Hash1Start, 5) + Hash1Start) ^ src[0];
                        return (int)(Hash1Start + (hash2 * Factor));

                    case 2:
                        hash2 = (BitOperations.RotateLeft(Hash1Start, 5) + Hash1Start) ^ src[0];
                        hash2 = (BitOperations.RotateLeft(hash2, 5) + hash2) ^ src[1];
                        return (int)(Hash1Start + (hash2 * Factor));

                    case 3:
                        hash2 = (BitOperations.RotateLeft(Hash1Start, 5) + Hash1Start) ^ src[0];
                        hash2 = (BitOperations.RotateLeft(hash2, 5) + hash2) ^ src[1];
                        hash2 = (BitOperations.RotateLeft(hash2, 5) + hash2) ^ src[2];
                        return (int)(Hash1Start + (hash2 * Factor));

                    case 4:
                        hash1 = (BitOperations.RotateLeft(Hash1Start, 5) + Hash1Start) ^ ((uint*)src)[0];
                        hash2 = (BitOperations.RotateLeft(Hash1Start, 5) + Hash1Start) ^ ((uint*)src)[1];
                        return (int)(hash1 + (hash2 * Factor));

                    default:
                        hash1 = Hash1Start;
                        hash2 = hash1;

                        uint* ptrUInt32 = (uint*)src;
                        while (length >= 4)
                        {
                            hash1 = (BitOperations.RotateLeft(hash1, 5) + hash1) ^ ptrUInt32[0];
                            hash2 = (BitOperations.RotateLeft(hash2, 5) + hash2) ^ ptrUInt32[1];
                            ptrUInt32 += 2;
                            length -= 4;
                        }

                        char* ptrChar = (char*)ptrUInt32;
                        while (length-- > 0)
                        {
                            hash2 = (BitOperations.RotateLeft(hash2, 5) + hash2) ^ *ptrChar++;
                        }

                        return (int)(hash1 + (hash2 * Factor));
                }
            }
        }
    }

    internal sealed class Config : ManualConfig
    {
        public Config()
        {
            this.SummaryStyle = SummaryStyle.Default.WithRatioStyle(RatioStyle.Trend);
            this.AddColumn(RankColumn.Arabic);
            this.Orderer = new DefaultOrderer(SummaryOrderPolicy.SlowestToFastest, MethodOrderPolicy.Declared);
            this.AddDiagnoser(MemoryDiagnoser.Default);
        }
    }
}
