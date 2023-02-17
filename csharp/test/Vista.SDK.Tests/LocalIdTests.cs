using FluentAssertions;
using Vista.SDK.Experimental;
using Vista.SDK.Internal;
using Vista.SDK.Mqtt;

namespace Vista.SDK.Tests;

public class LocalIdTests
{
    public sealed record class Input(
        string PrimaryItem,
        string? SecondaryItem = null,
        string? Quantity = null,
        string? Content = null,
        string? Position = null,
        bool Verbose = false
    );

    public static IEnumerable<object[]> Valid_Test_Data =>
        new object[][]
        {
            new object[] { new Input("411.1/C101.31-2"), "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta", },
            new object[]
            {
                new Input("411.1/C101.31-2", null, "temperature", "exhaust.gas", "inlet"),
                "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            },
            new object[]
            {
                new Input(
                    "411.1/C101.63/S206",
                    null,
                    "temperature",
                    "exhaust.gas",
                    "inlet",
                    Verbose: true
                ),
                "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            },
            new object[]
            {
                new Input(
                    "411.1/C101.63/S206",
                    "411.1/C101.31-5",
                    "temperature",
                    "exhaust.gas",
                    "inlet",
                    Verbose: true
                ),
                "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            },
        };

    public static IEnumerable<object[]> Valid_Mqtt_Test_Data =>
        new object[][]
        {
            new object[]
            {
                new Input("411.1/C101.31-2", null, "temperature", "exhaust.gas", "inlet"),
                "dnv-v2/vis-3-4a/411.1_C101.31-2/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            },
            new object[]
            {
                new Input("411.1/C101.63/S206", null, "temperature", "exhaust.gas", "inlet"),
                "dnv-v2/vis-3-4a/411.1_C101.63_S206/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            },
            new object[]
            {
                new Input(
                    "411.1/C101.63/S206",
                    "411.1/C101.31-5",
                    "temperature",
                    "exhaust.gas",
                    "inlet"
                ),
                "dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            },
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
    public void Test_LocalId_Build_Valid(Input input, string expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localId = LocalIdBuilder
            .Create(visVersion)
            .WithPrimaryItem(primaryItem)
            .TryWithSecondaryItem(secondaryItem)
            .WithVerboseMode(input.Verbose)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Quantity, input.Quantity))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, input.Content))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, input.Position));

        var localIdStr = localId.ToString();

        Assert.Equal(expectedOutput, localIdStr);
    }

    [Fact]
    public void Test_LocalId_Build_AllWithout()
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath("411.1/C101.31-2");
        var secondaryItem = gmod.ParsePath("411.1/C101.31-5");

        var localId = LocalIdBuilder
            .Create(visVersion)
            .WithPrimaryItem(primaryItem)
            .TryWithSecondaryItem(secondaryItem)
            .WithVerboseMode(true)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Quantity, "quantity"))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, "content"))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, "position"))
            .TryWithMetadataTag(codebooks.CreateTag(CodebookName.State, "state"))
            .TryWithMetadataTag(codebooks.CreateTag(CodebookName.Content, "content"))
            .TryWithMetadataTag(codebooks.CreateTag(CodebookName.Calculation, "calculate"));

        Assert.True(localId.IsValid);

        var allWithout = localId
            .WithoutPrimaryItem()
            .WithoutSecondaryItem()
            .WithoutQuantity()
            .WithoutPosition()
            .WithoutState()
            .WithoutContent()
            .WithoutCalculcation();

        Assert.True(allWithout.IsEmpty);
    }

    [Theory]
    [MemberData(nameof(Valid_Mqtt_Test_Data))]
    public void Test_Mqtt_LocalId_Build_Valid(Input input, string expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localIdBuilder = LocalIdBuilder
            .Create(visVersion)
            .TryWithPrimaryItem(primaryItem)
            .TryWithSecondaryItem(secondaryItem)
            .WithVerboseMode(input.Verbose)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Quantity, input.Quantity))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, input.Content))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, input.Position));
        var mqttLocalId = localIdBuilder.BuildMqtt();

        var localIdStr = mqttLocalId.ToString();

        Assert.Equal(expectedOutput, localIdStr);
    }

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
#pragma warning disable xUnit1026 // Theory methods should use all of their parameters
    public void Test_LocalId_Equality(Input input, string _)
#pragma warning restore xUnit1026 // Theory methods should use all of their parameters
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localId = LocalIdBuilder
            .Create(visVersion)
            .WithPrimaryItem(primaryItem)
            .TryWithSecondaryItem(secondaryItem)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Quantity, input.Quantity))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, input.Content))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, input.Position));

        var otherLocalId = localId;
        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.Same(localId, otherLocalId);

        otherLocalId = localId with { };
        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.NotSame(localId, otherLocalId);

        otherLocalId = otherLocalId.WithMetadataTag(
            codebooks.CreateTag(CodebookName.Position, "eqtestvalue")
        );
        Assert.NotEqual(localId, otherLocalId);
        Assert.True(localId != otherLocalId);
        Assert.NotSame(localId, otherLocalId);

        otherLocalId = localId
            .TryWithPrimaryItem(localId.PrimaryItem is null ? null : localId.PrimaryItem with { })
            .TryWithMetadataTag(
                codebooks.TryCreateTag(CodebookName.Position, localId.Position?.Value)
            );

        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.NotSame(localId, otherLocalId);
    }

    [Theory]
    [InlineData("/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking")]
    [InlineData("/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage")]
    [InlineData("/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened")]
    [InlineData("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet")]
    [InlineData(
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"
    )]
    [InlineData(
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"
    )]
    [InlineData("/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low")]
    public void Test_Parsing(string localIdStr)
    {
        var parsed = LocalIdBuilder.TryParse(localIdStr, out var localId);

        Assert.True(parsed);
        Assert.Equal(localIdStr, localId!.ToString());
    }

    [Fact]
    public void Test()
    {
        var localIdAsString =
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet";

        var localId = LocalIdBuilder.Parse(localIdAsString);
    }

    [Fact(Skip = "LocalIds have a lot of location errors")]
    public async Task SmokeTest_Parsing()
    {
        var file = File.OpenRead("testdata/LocalIds.txt");

        var reader = new StreamReader(file);

        var errored =
            new List<(string LocalIdStr, LocalIdBuilder? LocalId, Exception? Exception, LocalIdParsingErrorBuilder? ErrorBuilder)>();

        string? localIdStr;
        while ((localIdStr = await reader.ReadLineAsync()) is not null)
        {
            try
            {
                // Quick fix to skip invalid metadata tags "qty-content"
                if (localIdStr.Contains("qty-content"))
                    continue;
                if (!LocalIdBuilder.TryParse(localIdStr, out var errorBuilder, out var localId))
                    errored.Add((localIdStr, localId, null, errorBuilder));
                else if (localId.IsEmpty || !localId.IsValid)
                    errored.Add((localIdStr, localId, null, errorBuilder));
                //else // Readd when regen test-data using proper SDK
                //    Assert.Equal(localIdStr, localId.ToString());
            }
            catch (Exception ex)
            {
                // Quick fix to skip invalid location e.g. primaryItem 511.11-1SO
                if (ex.Message.Contains("location"))
                    continue;
                errored.Add((localIdStr, null, ex, null));
            }
        }
        if (errored.Select(e => e.ErrorBuilder?.ErrorMessages).Count() > 0)
        {
            // TODO - gmod path parsing now fails because we actually validate locations properly
            // might have to skip the smoketests while we fix the source data
            Console.Write("");
        }
        Assert.Empty(errored.Select(e => e.ErrorBuilder?.ErrorMessages).ToList());
        Assert.Empty(errored);
    }

    [Fact(Skip = "Experimental")]
    public void NewParser()
    {
        var input = "/dnv-v2/vis-3-4a/511.11-1SO/C101.67/S208/meta/qty-pressure/cnt-air/state-low";
        var parser = new LocalIdParser(input);

        var id = parser.Parse();
    }

    [Theory]
    [MemberData(
        nameof(VistaSDKTestData.AddInvalidLocalIdsData),
        MemberType = typeof(VistaSDKTestData)
    )]
    public void Test_Parsing_Validation(string localIdStr, string[] expectedErrorMessages)
    {
        var parsed = LocalIdBuilder.TryParse(
            localIdStr,
            out LocalIdParsingErrorBuilder errorBuilder,
            out _
        );

        var actualErrorMessages = errorBuilder.ErrorMessages.Select(e => e.message).ToArray();
        actualErrorMessages.Should().Equal(expectedErrorMessages);

        Assert.False(parsed);
        Assert.NotNull(errorBuilder);
    }
}
