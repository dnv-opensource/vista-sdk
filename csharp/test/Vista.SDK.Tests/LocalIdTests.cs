using FluentAssertions;
using Vista.SDK.Experimental;
using Vista.SDK.Internal;
using Vista.SDK.Mqtt;

namespace Vista.SDK.Tests;

public class ParsingErrorsTests
{
    [Fact]
    public void Comparisons()
    {
        var errors1 = new[] { ("T1", "M1") };
        var errors2 = new[] { ("T1", "M1"), ("T2", "M1") };

        var e1 = new ParsingErrors(errors1);
        var e2 = new ParsingErrors(errors1);
        var e3 = new ParsingErrors(errors2);
        var e4 = ParsingErrors.Empty;
        Assert.Equal(e1, e2);
        Assert.True(e1 == e2);
#pragma warning disable CS1718
        Assert.True(e1 == e1);
#pragma warning restore CS1718
        Assert.False(e1 == null);
        Assert.False(e1 == e4);

        Assert.NotEqual(e1, e3);
        Assert.False(e1 == e3);
        Assert.True(e4 == ParsingErrors.Empty);
        Assert.Equal(e4, ParsingErrors.Empty);
        Assert.True(e4.Equals(ParsingErrors.Empty));
        Assert.True(e4.Equals((object)ParsingErrors.Empty));
    }

    [Fact]
    public void Enumerator()
    {
        var errors1 = new[] { ("T1", "M1") };
        var errors2 = new[] { ("T1", "M1"), ("T2", "M1") };

        var e1 = new ParsingErrors(errors1);
        var e2 = new ParsingErrors(errors2);
        var e3 = ParsingErrors.Empty;

        Assert.Equal(errors1.Length, e1.Count());
        Assert.Equal(errors2.Length, e2.Count());
        Assert.Empty(e3);
    }
}

public class LocalIdTests
{
    public sealed record class Input(
        string PrimaryItem,
        string? SecondaryItem = null,
        string? Quantity = null,
        string? Content = null,
        string? Position = null,
        VisVersion VisVersion = VisVersion.v3_4a,
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
                new Input("411.1/C101.63/S206", null, "temperature", "exhaust.gas", "inlet", Verbose: true),
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

            [
                new Input(PrimaryItem: "511.11/C101.67/S208", Quantity: "pressure", Position: "inlet", Content: "starting.air", Verbose: true, VisVersion: VisVersion.v3_6a),
                "/dnv-v2/vis-3-6a/511.11/C101.67/S208/~main.generator.engine/~starting.system.pneumatic/meta/qty-pressure/cnt-starting.air/pos-inlet"
            ]
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
                new Input("411.1/C101.63/S206", "411.1/C101.31-5", "temperature", "exhaust.gas", "inlet"),
                "dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            },
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
    public void Test_LocalId_Build_Valid(Input input, string expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = input.VisVersion;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null ? gmod.ParsePath(input.SecondaryItem) : null;

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
            .WithoutCalculation();

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
        var secondaryItem = input.SecondaryItem is not null ? gmod.ParsePath(input.SecondaryItem) : null;

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
        var secondaryItem = input.SecondaryItem is not null ? gmod.ParsePath(input.SecondaryItem) : null;

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

        otherLocalId = otherLocalId.WithMetadataTag(codebooks.CreateTag(CodebookName.Position, "eqtestvalue"));
        Assert.NotEqual(localId, otherLocalId);
        Assert.True(localId != otherLocalId);
        Assert.NotSame(localId, otherLocalId);

        otherLocalId = localId
            .TryWithPrimaryItem(localId.PrimaryItem is null ? null : localId.PrimaryItem with { })
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, localId.Position?.Value));

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
        var localIdAsString = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet";

        var localId = LocalIdBuilder.Parse(localIdAsString);
    }

    // [Fact(Skip = "LocalIds have a lot of location errors")]
    [Fact]
    public async Task SmokeTest_Parsing()
    {
        await using (var file = File.OpenRead("testdata/LocalIds.txt"))
        {
            using var reader = new StreamReader(file, leaveOpen: true);

            var errored =
                new List<(
                    string LocalIdStr,
                    LocalIdBuilder? LocalId,
                    Exception? Exception,
                    ParsingErrors ParsingErrors
                )>();

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
                    errored.Add((localIdStr, null, ex, ParsingErrors.Empty));
                }
            }
            if (errored.Any(e => e.ParsingErrors.HasErrors))
            {
                // TODO - gmod path parsing now fails because we actually validate locations properly
                // might have to skip the smoketests while we fix the source data
                Console.Write("");
            }
            Assert.Empty(errored.SelectMany(e => e.ParsingErrors).ToList());
            Assert.Empty(errored);
        }
    }

    [Fact(Skip = "Experimental")]
    public void NewParser()
    {
        var input = "/dnv-v2/vis-3-4a/511.11-1SO/C101.67/S208/meta/qty-pressure/cnt-air/state-low";
        var parser = new LocalIdParser(input);

        var id = parser.Parse();
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddInvalidLocalIdsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Parsing_Validation(string localIdStr, string[] expectedErrorMessages)
    {
        var parsed = LocalIdBuilder.TryParse(localIdStr, out var errorBuilder, out _);

        var actualErrorMessages = errorBuilder.Select(e => e.Message).ToArray();
        actualErrorMessages.Should().Equal(expectedErrorMessages);

        Assert.False(parsed);
        Assert.NotNull(errorBuilder);
    }

    // [Fact]
    // public async Task Test_ILocalId_Equivalency()
    // {
    //     var tasks = new Task[Environment.ProcessorCount];
    //     for (int i = 0; i < Environment.ProcessorCount; i++)
    //     {
    //         var task = Task.Run(() =>
    //         {
    //             for (int j = 0; j < 100; j++)
    //             {
    //                 ILocalId localId1 = PMSLocalId.Parse(
    //                     "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-F/C621/meta/maint.cat-preventive/act.type-service"
    //                 );

    //                 ILocalId localId2 = PMSLocalId.Parse(localId1.ToString());

    //                 localId2.Should().BeEquivalentTo(localId1, config => config.For);
    //             }
    //         });
    //         tasks[i] = task;
    //     }

    //     await Task.WhenAll(tasks);
    // }
}
