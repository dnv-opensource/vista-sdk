using Vista.SDK;
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

    //public static readonly Gmod TestGmod =

    //public static readonly Input TestInput = new Input("411.1/C101.31-2", null, "temperature", "exhaust.gas", "inlet");

    //public static readonly LocalId TestLocalId = LocalId
    //        .Create(visVersion)
    //        .WithPrimaryItem(primaryItem)
    //        .WithSecondaryItem(secondaryItem)
    //        .WithVerboseMode(TestInput.Verbose)
    //        .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Quantity, TestInput.Quantity))
    //        .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, TestInput.Content))
    //        .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, TestInput.Position));

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
            .WithSecondaryItem(secondaryItem)
            .WithVerboseMode(input.Verbose)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Quantity, input.Quantity))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, input.Content))
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Position, input.Position));

        var localIdStr = localId.ToString();

        Assert.Equal(expectedOutput, localIdStr);
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
            .WithPrimaryItem(primaryItem)
            .WithSecondaryItem(secondaryItem)
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
            .WithSecondaryItem(secondaryItem)
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
            .WithPrimaryItem(localId.PrimaryItem is null ? null : localId.PrimaryItem with { })
            .TryWithMetadataTag(
                codebooks.TryCreateTag(CodebookName.Position, localId.Position?.Value)
            );

        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.NotSame(localId, otherLocalId);
    }

    [Theory]
    [InlineData("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta")]
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

    [Fact]
    public async Task SmokeTest_Parsing()
    {
        var file = File.OpenRead("testdata/LocalIds.txt");

        var reader = new StreamReader(file);

        var errored =
            new List<(string LocalIdStr, LocalIdBuilder? LocalId, Exception? Exception)>();

        string? localIdStr;
        LocalIdErrorBuilder errorBuilder = LocalIdErrorBuilder.Create();
        while ((localIdStr = await reader.ReadLineAsync()) is not null)
        {
            try
            {
                if (!LocalIdBuilder.TryParse(localIdStr, out errorBuilder, out var localId))
                    errored.Add((localIdStr, localId, null));
                else if (localId.IsEmpty || !localId.IsValid)
                    errored.Add((localIdStr, localId, null));
                //else // Readd when regen test-data using proper SDK
                //    Assert.Equal(localIdStr, localId.ToString());
            }
            catch (Exception ex)
            {
                errored.Add((localIdStr, null, ex));
            }
        }
        Assert.True(errorBuilder.ErrorMessages.Count == 0);
        Assert.Empty(errored);
    }

    public static IEnumerable<object[]> Invalid_Test_Data =>
        new object[][]
        {
            new object[]
            {
                "/dnv-v2/vis-3-4a/652.31/Sf90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
                new[]
                {
                    "Invalid GmodNode in Primary item: Sf90.3",
                    "Invalid GmodPath: Last part in Primary item: Sf90.3/S61"
                }
            },
            new object[]
            {
                "/dnv-v2/vis-3-4a/652.31/S90.3/S61/se/652.1i-1P/meta/cnt-sea.water/state-opened",
                new[]
                {
                    "Invalid GmodNode in Primary item: se",
                    "Invalid GmodPath: Last part in Primary item: se/652.1i-1P",
                }
            },
            new object[]
            {
                "/dnv-v2/vis-3-4a/f652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
                new[]
                {
                    "Invalid start GmodNode in Primary item: f652.31",
                    "Invalid GmodPath in Primary item: f652.31/S90.3/S61",
                }
            },
            new object[]
            {
                "/dnv-v2/vis-3-4a/f652.31/S90.3/S61/se/652.1i-1P/meta/cnt-sea.water/state-opened",
                new[]
                {
                    "Invalid start GmodNode in Primary item: f652.31",
                    "Invalid GmodNode in Primary item: se",
                    "Invalid GmodPath: Last part in Primary item: se/652.1i-1P",
                }
            },
        };

    [Theory]
    //[InlineData("/dnv-v2/vis-3-4a/1021.1i-3AC/H121/meta/qty-temperature/cnt-cargo/cal")]
    [MemberData(nameof(Invalid_Test_Data))]
    public void Test_Faulty_Parsing(string localIdStr, string[] expectedErrorMessages)
    {
        var parsed = LocalIdBuilder.TryParse(
            localIdStr,
            out LocalIdErrorBuilder errorBuilder,
            out _
        );

        foreach (var error in errorBuilder.ErrorMessages)
        {
            Assert.Contains(error.message, expectedErrorMessages);
        }

        Assert.False(parsed);
        Assert.NotNull(errorBuilder);
        Assert.Equal(expectedErrorMessages.Count(), errorBuilder.ErrorMessages.Count);
        Assert.Throws<LocalIdException>(() => errorBuilder.ThrowOnError());
    }
}
