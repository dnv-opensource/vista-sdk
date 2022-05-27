using Vista.SDK;

namespace Vista.SDK.Tests;

public class CodebookTests
{
    public static IEnumerable<object[]> Valid_Position_Test_data =>
        new object[][]
        {
            new object[] { "phase.w.u", PositionValidationResult.Valid },
            new object[] { "outside-phase.w.u", PositionValidationResult.Valid },
            new object[] { "outside-phase.w.u-1", PositionValidationResult.Valid },
            new object[] { "outside-phase.w.u-10", PositionValidationResult.Valid },
            new object[] { "10-outside-phase.w.u", PositionValidationResult.InvalidOrder },
            new object[] { "10-#", PositionValidationResult.Invalid },
            new object[] { "#", PositionValidationResult.Invalid },
            new object[] { "outside!", PositionValidationResult.Invalid },
            new object[] { "1-centre", PositionValidationResult.InvalidOrder },
            new object[] { "phas!.w.u-outside-10", PositionValidationResult.Invalid },
            new object[] { "phase.w.u-outsid!-10", PositionValidationResult.Invalid },
            new object[] { "outside-phased.w.u-10", PositionValidationResult.Custom },
            new object[] { "outsidee", PositionValidationResult.Custom },
            new object[] { "port-starboard", PositionValidationResult.InvalidGrouping },
            new object[] { "starboard-port", PositionValidationResult.InvalidOrder },
            new object[] { "port-starboard-1", PositionValidationResult.InvalidGrouping },
            new object[] { "starboard-port-1", PositionValidationResult.InvalidOrder },
        };

    [Theory]
    [MemberData(nameof(Valid_Position_Test_data))]
    public void Test_Position_Validation(string input, PositionValidationResult expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var codebookType = codebooks[CodebookName.Position];
        var validPosition = codebookType.ValidatePosition(input);

        Assert.Equal(expectedOutput, validPosition);
    }

    [Fact]
    public void Test_Positions()
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var positions = codebooks[CodebookName.Position];

        Assert.False(positions.HasStandardValue("<number>"));
        Assert.True(positions.HasStandardValue("1"));
    }

    [Fact]
    public void Test_States()
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var states = codebooks[CodebookName.State];
        Assert.NotNull(states);

        Assert.True(states.HasGroup("Clogged"));

        Assert.True(states.HasStandardValue("clogged"));
    }

    [Fact]
    public void Test_Create_Tag()
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var codebookType = codebooks[CodebookName.Position];

        var metadataTag1 = codebookType.CreateTag("1");
        Assert.Equal("1", metadataTag1);
        Assert.False(metadataTag1.IsCustom);
        Assert.Same("1", metadataTag1.Value);

        var metadataTag2 = codebookType.CreateTag("centre");
        Assert.Equal("centre", metadataTag2);
        Assert.False(metadataTag2.IsCustom);

        var metadataTag3 = codebookType.CreateTag("centre-1");
        Assert.Equal("centre-1", metadataTag3);
        Assert.False(metadataTag3.IsCustom);
        Assert.Equal('-', metadataTag3.Prefix);

        var metadataTag4 = codebookType.CreateTag("somethingcustom");
        Assert.Equal("somethingcustom", metadataTag4);
        Assert.True(metadataTag4.IsCustom);
        Assert.Equal('~', metadataTag4.Prefix);

        Assert.Throws<ArgumentException>(() => codebookType.CreateTag("1-centre"));
        Assert.Null(codebookType.TryCreateTag("1-centre"));

        Assert.Throws<ArgumentException>(() => codebookType.CreateTag("centre!"));
        Assert.Null(codebookType.TryCreateTag("centre!"));
    }

    [Fact]
    public void Test_Get_Groups()
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var groups = codebooks[CodebookName.Position].Groups;
        Assert.True(groups.Count > 1);
    }

    [Fact]
    public void Test_Detail_Tag()
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var codebook = codebooks[CodebookName.Detail];
        Assert.NotNull(codebook);

        Assert.NotNull(codebook.TryCreateTag("something"));
        Assert.Null(codebook.TryCreateTag("something!"));
        Assert.Null(codebook.TryCreateTag("something<"));

        Assert.Throws<ArgumentException>(() => codebook.CreateTag("something!"));
        Assert.Throws<ArgumentException>(() => codebook.CreateTag("something<"));
    }
}
