using Spectre.Console;
using Spectre.Console.Cli;
using System.Diagnostics.CodeAnalysis;
using System.Text.Json;
using Vista.SDK;
using Vista.SDK.Transport.Json;
using Vista.SDK.Transport.Json.DataChannel;

var cts = new CancellationTokenSource();
Console.CancelKeyPress += (s, e) =>
{
    AnsiConsole.WriteLine("Canceling...");
    cts.Cancel();
    e.Cancel = true;
};

var app = new CommandApp();
app.Configure(
    config =>
    {
        config.AddCommand<GenerateTimeseries>("generate-timeseries");

        config.Settings.Registrar.RegisterInstance(cts.Token);
    }
);

app.Run(args);

public sealed class GenerateTimeseries : AsyncCommand<GenerateTimeseries.Settings>
{
    private readonly CancellationToken _cancellationToken;

    public sealed class Settings : CommandSettings
    {
        [CommandArgument(0, "[Type]")]
        public string? Type { get; set; }

        [CommandArgument(0, "[DataChannelListPath]")]
        public string? DataChannelListPath { get; set; }
    }

    public GenerateTimeseries(CancellationToken cancellationToken) =>
        _cancellationToken = cancellationToken;

    public override ValidationResult Validate(
        [NotNull] CommandContext context,
        [NotNull] Settings settings
    )
    {
        if (settings.Type != "tabular" && settings.Type != "event")
            return ValidationResult.Error("Invalid timeseries type: " + settings.Type);

        if (
            string.IsNullOrWhiteSpace(settings.DataChannelListPath)
            || !File.Exists(settings.DataChannelListPath)
        )
            return ValidationResult.Error(
                "DataChannelList does not exist at: " + settings.DataChannelListPath
            );

        return base.Validate(context, settings);
    }

    public override async Task<int> ExecuteAsync(CommandContext context, Settings settings)
    {
        await using var dataChannelListStream = new FileStream(
            settings.DataChannelListPath!,
            FileMode.Open,
            FileAccess.Read,
            FileShare.Read
        );

        var dataChannelListDto = await Serializer.DeserializeDataChannelListAsync(
            dataChannelListStream,
            _cancellationToken
        );

        if (dataChannelListDto is null)
            throw new Exception(
                "Couldnt load DataChannelList from " + settings.DataChannelListPath
            );

        var generateBasedOn = new GenerateBasedOn[]
        {
            new GenerateBasedOn("1021.31-4S/H203", null, Command: "shut.down"),
            new GenerateBasedOn("511.15-1/E32", null, Quantity: "frequency"),
        };

        var dataChannelList = dataChannelListDto.ToDomainModel();

        // var localIds = dataChannelList.Package.DataChannelList.DataChannel
        //     .Select(dc => dc.DataChannelId.LocalId)
        //     .Where(l => l)
        //     .ToArray();

        // var generateFor = localIds.Where(l => )


        return 0;
    }

    readonly record struct GenerateBasedOn(
        string? PrimaryItem,
        string? SecondaryItem = null,
        string? Quantity = null,
        string? State = null,
        string? Content = null,
        string? Position = null,
        string? Calculation = null,
        string? Command = null,
        string? Type = null
    );
}
