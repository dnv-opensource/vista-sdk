﻿using Vista.SDK;

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

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
    public async Task Test_LocalId_Build_Valid(Input input, string expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = await vis.GetGmod(visVersion);
        var codebooks = await vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localId = LocalId
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
    [MemberData(nameof(Valid_Test_Data))]
#pragma warning disable xUnit1026 // Theory methods should use all of their parameters
    public async Task Test_LocalId_Equality(Input input, string _)
#pragma warning restore xUnit1026 // Theory methods should use all of their parameters
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = await vis.GetGmod(visVersion);
        var codebooks = await vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localId = LocalId
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
    [InlineData("/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet")]
    [InlineData(
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"
    )]
    [InlineData(
        "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"
    )]
    public async Task Test_Parsing(string localIdStr)
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_4a;

        var gmod = await vis.GetGmod(visVersion);
        var codebooks = await vis.GetCodebooks(visVersion);

        var parsed = LocalId.TryParse(localIdStr, gmod, codebooks, out var localId);
        Assert.True(parsed);
        Assert.Equal(localIdStr, localId!.ToString());
    }
}
