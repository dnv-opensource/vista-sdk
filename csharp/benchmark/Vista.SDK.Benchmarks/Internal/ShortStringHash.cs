using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Vista.SDK.Benchmarks.Internal;

[Config(typeof(Config))]
public class ShortStringHash
{
    [Params("400", "H346.11112")]
    public string Input { get; set; }

    [Benchmark(Baseline = true)]
    public uint Bcl() => (uint)Input.GetHashCode();

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
        var input = inputStr.AsSpan();

        var length = input.Length * sizeof(char);
        var code = MemoryMarshal.CreateReadOnlySpan(
            ref Unsafe.As<char, byte>(ref MemoryMarshal.GetReference(input)),
            length
        );

        uint hash = 0x01000193;
        for (int i = 0; i < code.Length; i += 2)
        {
            hash = THasher.Hash(hash, code[i]);
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
        public static uint Hash(uint hash, byte ch) => SDK.Internal.NodeMap.Hashing.LarssonHash(hash, ch);
    }

    readonly struct Crc32IntrinsicHasher : IHasher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint Hash(uint hash, byte ch) => SDK.Internal.NodeMap.Hashing.Crc32(hash, ch);
    }

    readonly struct FnvHasher : IHasher
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static uint Hash(uint hash, byte ch) => SDK.Internal.NodeMap.Hashing.Fnv(hash, ch);
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
