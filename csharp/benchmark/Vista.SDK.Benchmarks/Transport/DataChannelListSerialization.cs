using Avro.IO;
using Avro.Specific;
using BenchmarkDotNet.Analysers;
using BenchmarkDotNet.Columns;
using BenchmarkDotNet.Engines;
using BenchmarkDotNet.Exporters;
using BenchmarkDotNet.Exporters.Csv;
using BenchmarkDotNet.Mathematics;
using BenchmarkDotNet.Reports;
using BenchmarkDotNet.Running;
using BenchmarkDotNet.Toolchains;
using BenchmarkDotNet.Toolchains.InProcess;
using BenchmarkDotNet.Toolchains.InProcess.Emit;
using BenchmarkDotNet.Toolchains.InProcess.NoEmit;
using BenchmarkDotNet.Validators;
using ICSharpCode.SharpZipLib.BZip2;
using Microsoft.Diagnostics.Tracing.Parsers.IIS_Trace;
using System.Globalization;
using System.IO.Compression;
using System.Text.Json;
using Vista.SDK.Transport.Avro.DataChannel;
using Vista.SDK.Transport.Json;
using Vista.SDK.Transport.Json.DataChannel;
using DataChannelListJsonPackage = Vista.SDK.Transport.Json.DataChannel.DataChannelListPackage;
using DataChannelListAvroPackage = Vista.SDK.Transport.Avro.DataChannel.DataChannelListPackage;
using RunMode = BenchmarkDotNet.Diagnosers.RunMode;

namespace Vista.SDK.Benchmarks.Transport;

[ConfigSource]
public class DataChannelListSerialization
{
    private MemoryStream _memoryStream;
    private BrotliStream _brotliStream;
    private MemoryStream _compressionStream;

    private DataChannelListJsonPackage _jsonPackage;
    private DataChannelListAvroPackage _avroPackage;
    private BinaryEncoder _avroEncoder;
    private SpecificWriter<DataChannelListAvroPackage> _avroWriter;

    private static readonly Lazy<DataChannelListSerialization> _bench =
        new(
            () =>
            {
                var bench = new DataChannelListSerialization();
                bench.Setup();
                return bench;
            }
        );
    private static readonly Dictionary<string, long> _payloadSizes = new();

    public class ConfigSourceAttribute : Attribute, IConfigSource
    {
        public IConfig Config { get; }

        public ConfigSourceAttribute()
        {
            var config = ManualConfig.CreateEmpty();

            // config.AddJob(
            //     Job.Default
            //         .WithId("DataChannelList serialization")
            //         .WithToolchain(new InProcessEmitToolchain(TimeSpan.FromHours(1), true))
            //         .WithCustomBuildConfiguration("Debug")
            // );
            // config.WithOption(ConfigOptions.DisableOptimizationsValidator, true);

            config.AddJob(Job.ShortRun.WithId("DataChannelList serialization"));

            config.AddColumn(new CategoriesColumn());
            config.AddExporter(CsvMeasurementsExporter.Default);
            config.AddExporter(RPlotExporter.Default);
            // config.AddDiagnoser(MemoryDiagnoser.Default);
            // config.WithOrderer(
            //     new DefaultOrderer(SummaryOrderPolicy.Method, MethodOrderPolicy.Alphabetical)
            // );
            config.AddLogicalGroupRules(BenchmarkLogicalGroupRule.ByCategory);

            config.SummaryStyle = SummaryStyle.Default
                .WithRatioStyle(RatioStyle.Percentage)
                .WithSizeUnit(SizeUnit.KB);

            // AddColumn(new RankColumn(NumeralSystem.Arabic));
            // AddColumn(BaselineRatioColumn.RatioMean);
            // AddColumn(new SizeRatioColumn());
            config.AddDiagnoser(new PayloadSizeDiagnoser());

            config.AddColumnProvider(
                new SimpleColumnProvider(
                    TargetMethodColumn.Method,
                    new CategoriesColumn(),
                    new ParamColumn("CompressionLevel", 0),
                    StatisticColumn.Mean,
                    new MetricColumn(new PayloadSizeMetric())
                )
            );

            config.KeepBenchmarkFiles(true);

            Config = config;
        }
    }

    [GlobalSetup]
    public void Setup()
    {
        var text = File.ReadAllText("schemas/json/DataChannelList.sample.compact.json");
        _jsonPackage = Serializer.DeserializeDataChannelList(text);
        var domainModel = _jsonPackage!.ToDomainModel();
        _avroPackage = domainModel.ToAvroDto();

        _memoryStream = new MemoryStream();
        _compressionStream = new MemoryStream();
        _avroWriter = new SpecificWriter<DataChannelListAvroPackage>(
            DataChannelListAvroPackage._SCHEMA
        );
        _avroEncoder = new BinaryEncoder(_memoryStream);

        Json();
        _payloadSizes[nameof(Json)] = _memoryStream.Length;

        Avro();
        _payloadSizes[nameof(Avro)] = _memoryStream.Length;

        Json_Brotli();
        _payloadSizes[nameof(Json_Brotli)] = _compressionStream.Length;

        Avro_Brotli();
        _payloadSizes[nameof(Avro_Brotli)] = _compressionStream.Length;

        foreach (var compressionLevelArgs in GetCompressionLevels())
        {
            var compressionLevel = (int)compressionLevelArgs[0];

            Json_Bzip2(compressionLevel);
            _payloadSizes[$"{nameof(Json_Bzip2)}-{compressionLevel}"] = _compressionStream.Length;

            Avro_Bzip2(compressionLevel);
            _payloadSizes[$"{nameof(Avro_Bzip2)}-{compressionLevel}"] = _compressionStream.Length;
        }
    }

    [GlobalCleanup]
    public void Cleanup()
    {
        _memoryStream.Dispose();
    }

    [Benchmark(Description = "Json")]
    [BenchmarkCategory("Uncompressed")]
    public void Json()
    {
        _memoryStream.SetLength(0);
        _jsonPackage.Serialize(_memoryStream);
    }

    [Benchmark(Description = "Json")]
    [ArgumentsSource(nameof(GetCompressionLevels))]
    [BenchmarkCategory("Bzip2")]
    public void Json_Bzip2(int CompressionLevel)
    {
        _memoryStream.SetLength(0);
        _compressionStream.SetLength(0);
        _jsonPackage.Serialize(_memoryStream);
        _memoryStream.Position = 0;
        BZip2.Compress(_memoryStream, _compressionStream, false, CompressionLevel);
    }

    [Benchmark(Description = "Json")]
    [BenchmarkCategory("Brotli")]
    public void Json_Brotli()
    {
        _memoryStream.SetLength(0);
        _compressionStream.SetLength(0);
        _jsonPackage.Serialize(_memoryStream);
        _memoryStream.Position = 0;
        _brotliStream = new BrotliStream(_compressionStream, CompressionLevel.SmallestSize, true);
        _memoryStream.CopyTo(_brotliStream);
        _brotliStream.Flush();
    }

    [Benchmark(Description = "Avro")]
    [BenchmarkCategory("Uncompressed")]
    public void Avro()
    {
        _memoryStream.SetLength(0);
        _avroWriter.Write(_avroPackage, _avroEncoder);
    }

    [Benchmark(Description = "Avro")]
    [ArgumentsSource(nameof(GetCompressionLevels))]
    [BenchmarkCategory("Bzip2")]
    public void Avro_Bzip2(int CompressionLevel)
    {
        _memoryStream.SetLength(0);
        _compressionStream.SetLength(0);
        _avroWriter.Write(_avroPackage, _avroEncoder);
        _memoryStream.Position = 0;
        BZip2.Compress(_memoryStream, _compressionStream, false, CompressionLevel);
    }

    [Benchmark(Description = "Avro")]
    [BenchmarkCategory("Brotli")]
    public void Avro_Brotli()
    {
        _memoryStream.SetLength(0);
        _compressionStream.SetLength(0);
        _avroWriter.Write(_avroPackage, _avroEncoder);
        _memoryStream.Position = 0;
        _brotliStream = new BrotliStream(_compressionStream, CompressionLevel.SmallestSize, true);
        _memoryStream.CopyTo(_brotliStream);
        _brotliStream.Flush();
    }

    public static IEnumerable<object[]> GetCompressionLevels()
    {
        yield return new object[] { 5 };
        yield return new object[] { 9 };
    }

    public class PayloadSizeMetric : IMetricDescriptor
    {
        public static readonly PayloadSizeMetric Instance = new PayloadSizeMetric();
        public string Id => "payload-size-metric";
        public string DisplayName => "Payload size";
        public string Legend => "Payload size";
        public string NumberFormat => "0.##";
        public UnitType UnitType => UnitType.Size;
        public string Unit => SizeUnit.B.Name;
        public bool TheGreaterTheBetter => false;
        public int PriorityInCategory => 0;
    }

    public class PayloadSizeDiagnoser : IDiagnoser
    {
        private long _currentPayloadSize;

        public RunMode GetRunMode(BenchmarkCase benchmarkCase) => RunMode.NoOverhead;

        public void Handle(HostSignal signal, DiagnoserActionParameters parameters)
        {
            switch (signal)
            {
                case HostSignal.AfterAll:
                    _ = _bench.Value;

                    var arg = parameters.BenchmarkCase.HasParameters
                        ? (int?)parameters.BenchmarkCase.Parameters.Items.Single().Value
                        : null;
                    var name = parameters.BenchmarkCase.Descriptor.WorkloadMethod.Name;
                    _currentPayloadSize = _payloadSizes[arg is null ? name : $"{name}-{arg.Value}"];
                    break;
            }
        }

        public IEnumerable<Metric> ProcessResults(DiagnoserResults results)
        {
            yield return new Metric(PayloadSizeMetric.Instance, _currentPayloadSize);
        }

        public void DisplayResults(ILogger logger) { }

        public IEnumerable<ValidationError> Validate(ValidationParameters validationParameters) =>
            Array.Empty<ValidationError>();

        public IEnumerable<string> Ids => new[] { nameof(PayloadSizeDiagnoser) };
        public IEnumerable<IExporter> Exporters => Array.Empty<IExporter>();
        public IEnumerable<IAnalyser> Analysers => Array.Empty<IAnalyser>();
    }
}
