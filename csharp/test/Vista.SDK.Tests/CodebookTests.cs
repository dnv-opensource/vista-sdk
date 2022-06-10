namespace Vista.SDK.Tests;

public class CodebookTests
{
    [Theory]
    [MemberData(
        nameof(VistaSDKTestData.AddValidPositionData),
        MemberType = typeof(VistaSDKTestData)
    )]
    public void Test_Position_Validation(string input, string expectedOutput)
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var codebookType = codebooks[CodebookName.Position];
        var validPosition = codebookType.ValidatePosition(input);
        var parsedExpectedOutput = PositionValidationResults.FromString(expectedOutput);

        Assert.Equal(parsedExpectedOutput, validPosition);
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddPositionsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Positions(string invalidStandardValue, string validStandardValue)
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var positions = codebooks[CodebookName.Position];

        Assert.False(positions.HasStandardValue(invalidStandardValue));
        Assert.True(positions.HasStandardValue(validStandardValue));
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddStatesData), MemberType = typeof(VistaSDKTestData))]
    public void Test_States(
        string invalidGroup,
        string validValue,
        string validGroup,
        string secondValidValue
    )
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var states = codebooks[CodebookName.State];
        Assert.NotNull(states);

        Assert.False(states.HasGroup(invalidGroup));

        Assert.True(states.HasStandardValue(validValue));

        Assert.True(states.HasGroup(validGroup));

        Assert.True(states.HasStandardValue(secondValidValue));
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddTagData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Create_Tag(
        string firstTag,
        string secondTag,
        string thirdTag,
        char thirdTagPrefix,
        string customTag,
        char customTagPrefix,
        string firstInvalidTag,
        string secondInvalidTag
    )
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var codebookType = codebooks[CodebookName.Position];

        var metadataTag1 = codebookType.CreateTag(firstTag);
        Assert.Equal(firstTag, metadataTag1);
        Assert.False(metadataTag1.IsCustom);
        Assert.Same(firstTag, metadataTag1.Value);

        var metadataTag2 = codebookType.CreateTag(secondTag);
        Assert.Equal(secondTag, metadataTag2);
        Assert.False(metadataTag2.IsCustom);

        var metadataTag3 = codebookType.CreateTag(thirdTag);
        Assert.Equal(thirdTag, metadataTag3);
        Assert.False(metadataTag3.IsCustom);
        Assert.Equal(thirdTagPrefix, metadataTag3.Prefix);

        var metadataTag4 = codebookType.CreateTag(customTag);
        Assert.Equal(customTag, metadataTag4);
        Assert.True(metadataTag4.IsCustom);
        Assert.Equal(customTagPrefix, metadataTag4.Prefix);

        Assert.Throws<ArgumentException>(() => codebookType.CreateTag(firstInvalidTag));
        Assert.Null(codebookType.TryCreateTag(firstInvalidTag));

        Assert.Throws<ArgumentException>(() => codebookType.CreateTag(secondInvalidTag));
        Assert.Null(codebookType.TryCreateTag(secondInvalidTag));
    }

    [Fact]
    public void Test_Get_Groups()
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var groups = codebooks[CodebookName.Position].Groups;
        Assert.True(groups.Count > 1);
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddDetailTagData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Detail_Tag(
        string validCustomTag,
        string firstInvalidCustomTag,
        string secondInvalidCustomTag
    )
    {
        var (_, vis) = VISTests.GetVis();
        var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

        var codebook = codebooks[CodebookName.Detail];
        Assert.NotNull(codebook);

        Assert.NotNull(codebook.TryCreateTag(validCustomTag));
        Assert.Null(codebook.TryCreateTag(firstInvalidCustomTag));
        Assert.Null(codebook.TryCreateTag(secondInvalidCustomTag));

        Assert.Throws<ArgumentException>(() => codebook.CreateTag(firstInvalidCustomTag));
        Assert.Throws<ArgumentException>(() => codebook.CreateTag(secondInvalidCustomTag));
    }
}
