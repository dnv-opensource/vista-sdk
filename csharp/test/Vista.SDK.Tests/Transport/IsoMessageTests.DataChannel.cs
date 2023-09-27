using FluentAssertions;
using Vista.SDK.Transport;
using Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Tests.Transport;

public partial class IsoMessageTests
{
    public static DataChannelListPackage TestDataChannelListPackage =>
        new DataChannelListPackage(
            new Package(
                new Header(
                    ShipId.Parse("IMO1234567"),
                    new ConfigurationReference(
                        "DataChannelList.xml",
                        "1.0",
                        DateTimeOffset.Parse("2016-01-01T00:00:00Z")
                    ),
                    new VersionInformation("some_naming_rule", "2.0", "http://somewhere.net"),
                    "Author1",
                    DateTimeOffset.Parse("2015-12-01T00:00:00Z"),
                    new Dictionary<string, object>() { ["nr:CustomHeaderElement"] = "Vender specific headers" }
                ),
                new DataChannelList(
                    new DataChannel[]
                    {
                        new DataChannel(
                            new SDK.Transport.DataChannel.DataChannelId(
                                LocalId.Parse(
                                    "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
                                ),
                                "0010",
                                new NameObject(
                                    "Naming_Rule",
                                    new Dictionary<string, object>()
                                    {
                                        ["nr:CustomNameObject"] = "Vender specific NameObject"
                                    }
                                )
                            ),
                            new Property(
                                new DataChannelType("Inst", UpdateCycle: "1", CalculationPeriod: null),
                                new Format(
                                    "Decimal",
                                    new Restriction(
                                        Enumeration: null,
                                        FractionDigits: "1",
                                        Length: null,
                                        MaxExclusive: null,
                                        MaxInclusive: "200.0",
                                        MaxLength: null,
                                        MinExclusive: null,
                                        MinInclusive: "-150.0",
                                        MinLength: null,
                                        Pattern: null,
                                        TotalDigits: null,
                                        WhiteSpace: null
                                    )
                                ),
                                new Vista.SDK.Transport.DataChannel.Range("150.0", "0.0"),
                                new Unit("°C", "Temperature", new Dictionary<string, object>()),
                                "OPC_QUALITY",
                                AlertPriority: null,
                                "M/E #1 Air Cooler CFW OUT Temp",
                                " Location: ECR, Manufacturer: AAA Company, Type: TYPE-AAA ",
                                new Dictionary<string, object>()
                                {
                                    ["nr:CustomPropertyElement"] = "Vender specific Property"
                                }
                            )
                        )
                    }
                )
            )
        );

    [Fact]
    public void Test_DataChannelList()
    {
        var message = TestDataChannelListPackage;

        Assert.NotNull(message);
    }

    [Fact]
    public void Test_DataChannelList_Json()
    {
        var message = TestDataChannelListPackage;

        var dto = SDK.Transport.Json.DataChannel.Extensions.ToJsonDto(message);
        var message2 = SDK.Transport.Json.DataChannel.Extensions.ToDomainModel(dto);

        message.Should().BeEquivalentTo(message2);
    }
}
