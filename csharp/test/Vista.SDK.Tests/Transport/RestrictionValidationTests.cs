using FluentAssertions;
using Vista.SDK.Transport;
using Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Tests.Transport;

public class RestrictionValidationTests
{
    private static Format FormatOf(string type) => new Format { Type = type, Restriction = null };

    private static void AssertResult(ValidateResult result, bool expectedOk)
    {
        if (expectedOk)
            result.Should().BeOfType<ValidateResult.Ok>();
        else
            result.Should().BeOfType<ValidateResult.Invalid>();
    }

    [Theory]
    [InlineData(5, "12345", true)] // length == MaxLength is valid
    [InlineData(5, "1234", true)] // below the limit is valid
    [InlineData(5, "123456", false)] // above the limit is invalid
    [InlineData(0, "", true)] // empty string with MaxLength 0
    public void Test_MaxLength_Boundary(int maxLength, string value, bool expectedOk)
    {
        var restriction = new Restriction { MaxLength = (uint)maxLength };

        var result = restriction.ValidateValue(value, FormatOf("String"));

        AssertResult(result, expectedOk);
    }

    [Theory]
    [InlineData(3, "123", true)] // length == MinLength is valid
    [InlineData(3, "1234", true)] // above the limit is valid
    [InlineData(3, "12", false)] // below the limit is invalid
    public void Test_MinLength_Boundary(int minLength, string value, bool expectedOk)
    {
        var restriction = new Restriction { MinLength = (uint)minLength };

        var result = restriction.ValidateValue(value, FormatOf("String"));

        AssertResult(result, expectedOk);
    }

    [Theory]
    [InlineData(3, "123", true)] // exact digit count is valid
    [InlineData(3, "12", false)] // too few digits is invalid
    [InlineData(3, "1234", false)] // too many digits is invalid
    [InlineData(3, "100", true)] // trailing zeros in the integer part count
    [InlineData(1, "0", true)] // zero counts as a single digit
    public void Test_TotalDigits_Integer(int totalDigits, string value, bool expectedOk)
    {
        var restriction = new Restriction { TotalDigits = (uint)totalDigits };

        var result = restriction.ValidateValue(value, FormatOf("Integer"));

        AssertResult(result, expectedOk);
    }

    [Theory]
    [InlineData(3, "12.3", true)] // 3 significant digits
    [InlineData(3, "1.23", true)] // 3 significant digits
    [InlineData(3, "0.045", false)] // only 2 significant digits
    [InlineData(2, "0.045", true)] // leading zeros do not count
    [InlineData(3, "12.34", false)] // 4 significant digits is invalid
    [InlineData(3, "12.30", true)] // trailing fractional zeros do not count
    public void Test_TotalDigits_Decimal(int totalDigits, string value, bool expectedOk)
    {
        var restriction = new Restriction { TotalDigits = (uint)totalDigits };

        var result = restriction.ValidateValue(value, FormatOf("Decimal"));

        AssertResult(result, expectedOk);
    }
}
