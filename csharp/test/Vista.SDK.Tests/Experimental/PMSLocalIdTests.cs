using Vista.SDK.Tests;

namespace Vista.SDK.Experimental.Tests;

public class PMSLocalIdTests
{
    public sealed record class Input(
        string PrimaryItem,
        string? SecondaryItem = null,
        string? Content = null,
        string? MaintenanceCategory = null,
        string? ActivityType = null,
        bool Verbose = false
    );

    public static IEnumerable<object[]> Valid_Test_Data =>
        new object[][]
        {
            new object[]
            {
                new Input(
                    "632.32i-1/S110",
                    null,
                    "high.temperature.fresh.water",
                    "preventive",
                    "overhaul"
                ),
                "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-overhaul"
            },
            new object[]
            {
                new Input(
                    "632.32i-2/S110",
                    null,
                    "high.temperature.fresh.water",
                    "preventive",
                    "test"
                ),
                "/dnv-v2-experimental/vis-3-6a/632.32i-2/S110/meta/cnt-high.temperature.fresh.water/maint.cat-preventive/act.type-test"
            },
            new object[]
            {
                new Input(
                    "632.32i-2/S110",
                    null,
                    "low.temperature.fresh.water",
                    "preventive",
                    "inspection"
                ),
                "/dnv-v2-experimental/vis-3-6a/632.32i-2/S110/meta/cnt-low.temperature.fresh.water/maint.cat-preventive/act.type-inspection"
            },
            new object[]
            {
                new Input(
                    "632.32i-1/S110",
                    null,
                    "low.temperature.fresh.water",
                    "preventive",
                    "inspection"
                ),
                "/dnv-v2-experimental/vis-3-6a/632.32i-1/S110/meta/cnt-low.temperature.fresh.water/maint.cat-preventive/act.type-inspection"
            },
            new object[]
            {
                new Input("641.11i-2/C662", null, null, "preventive", "refit"),
                "/dnv-v2-experimental/vis-3-6a/641.11i-2/C662/meta/maint.cat-preventive/act.type-refit"
            },
            new object[]
            {
                new Input("411.1/C101.661i-F/C621", null, null, "preventive", "service"),
                "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-F/C621/meta/maint.cat-preventive/act.type-service"
            },
            new object[]
            {
                new Input("411.1/C101.661i-A/C621", null, null, "preventive", "service"),
                "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-A/C621/meta/maint.cat-preventive/act.type-service"
            },
            new object[]
            {
                new Input("641.11i-2/C662", null, null, "preventive", "refit"),
                "/dnv-v2-experimental/vis-3-6a/641.11i-2/C662/meta/maint.cat-preventive/act.type-refit"
            },
        };

    public static IEnumerable<object[]> Invalid_Test_Data =>
        new object[][]
        {
            new object[]
            {
                new Input("641.11i-2/C66", null, null, "temperature", "refit"),
                "/dnv-v2-experimental/vis-3-6a/641.11i-2/C662/meta/qty-temperature/act.type-refit"
            },
        };

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
    public void Test_LocalId_Build_Valid(Input input, string expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_6a;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localId = PMSLocalIdBuilder
            .Create(visVersion)
            .WithPrimaryItem(primaryItem)
            .TryWithSecondaryItem(secondaryItem)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, input.Content))
            .TryWithMetadataTag(
                codebooks.TryCreateTag(CodebookName.MaintenanceCategory, input.MaintenanceCategory)
            )
            .TryWithMetadataTag(
                codebooks.TryCreateTag(CodebookName.ActivityType, input.ActivityType)
            )
            .WithVerboseMode(input.Verbose);

        var localIdStr = localId.ToString();

        Assert.Equal(expectedOutput, localIdStr);
    }

    [Theory]
    [MemberData(nameof(Valid_Test_Data))]
#pragma warning disable xUnit1026 // Theory methods should use all of their parameters
    public void Test_LocalId_Equality(Input input, string _)
#pragma warning restore xUnit1026 // Theory methods should use all of their parameters
    {
        var (_, vis) = VISTests.GetVis();

        var visVersion = VisVersion.v3_6a;

        var gmod = vis.GetGmod(visVersion);
        var codebooks = vis.GetCodebooks(visVersion);

        var primaryItem = gmod.ParsePath(input.PrimaryItem);
        var secondaryItem = input.SecondaryItem is not null
            ? gmod.ParsePath(input.SecondaryItem)
            : null;

        var localId = PMSLocalIdBuilder
            .Create(visVersion)
            .WithPrimaryItem(primaryItem)
            .TryWithSecondaryItem(secondaryItem)
            .TryWithMetadataTag(codebooks.TryCreateTag(CodebookName.Content, input.Content))
            .TryWithMetadataTag(
                codebooks.TryCreateTag(CodebookName.MaintenanceCategory, input.MaintenanceCategory)
            )
            .TryWithMetadataTag(
                codebooks.TryCreateTag(CodebookName.ActivityType, input.ActivityType)
            )
            .WithVerboseMode(input.Verbose);

        var otherLocalId = localId;
        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.Same(localId, otherLocalId);

        otherLocalId = localId with { };
        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.NotSame(localId, otherLocalId);

        otherLocalId = otherLocalId.WithMetadataTag(
            codebooks.CreateTag(CodebookName.FunctionalServices, "eqtestvalue")
        );
        Assert.NotEqual(localId, otherLocalId);
        Assert.True(localId != otherLocalId);
        Assert.NotSame(localId, otherLocalId);

        otherLocalId = localId
            .TryWithPrimaryItem(localId.PrimaryItem)
            .TryWithMetadataTag(
                codebooks.TryCreateTag(
                    CodebookName.FunctionalServices,
                    localId.FunctionalServices?.Value
                )
            );

        Assert.Equal(localId, otherLocalId);
        Assert.True(localId == otherLocalId);
        Assert.NotSame(localId, otherLocalId);
    }

    [Theory]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-F/C621/meta/maint.cat-preventive/act.type-service"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/411.1/C101.661i-A/C621/meta/maint.cat-preventive/act.type-service"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/511.11-2/C101.663i/C663/meta/maint.cat-preventive/act.type-service/detail-turbine"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/511.15-1/E32/meta/maint.cat-preventive/act.type-check"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/621.21/S90.1/S41/~fuel.oil.piping/~pipes/meta/maint.cat-preventive/act.type-service"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/411.1/C101.64/S201/meta/maint.cat-preventive/act.type-check"
    )]
    public void Test_Parsing(string pmsLocalIdStr)
    {
        var parsed = PMSLocalIdBuilder.TryParse(pmsLocalIdStr, out var pmsLocalId);

        Assert.True(parsed);
        Assert.Equal(pmsLocalIdStr, pmsLocalId!.ToString());
    }

    [Theory]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/621.21/S90.1/S41/~fuel.oil.piping/~pipes/meta/maint.cat-preventive/act.type-service"
    )]
    [InlineData(
        "/dnv-v2-experimental/vis-3-6a/411.1/C101.64/S201/meta/maint.cat-preventive/act.type-check"
    )]
    public void Test_Build(string pmsLocalIdStr)
    {
        var parsedBuilder = PMSLocalIdBuilder.TryParse(pmsLocalIdStr, out var pmsLocalIdBuilder);
        var parsedId = PMSLocalId.Parse(pmsLocalIdStr);
        Assert.True(parsedBuilder);
        Assert.NotNull(parsedId);
        Assert.Equal(pmsLocalIdStr, pmsLocalIdBuilder!.ToString());

        var builtFromBuilder = pmsLocalIdBuilder.Build();
        Assert.NotNull(builtFromBuilder);
        Assert.NotSame(parsedId, builtFromBuilder);
        Assert.True(parsedId == builtFromBuilder);
    }
}
