using FluentAssertions;
using Vista.SDK.Transport;
using Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Tests.Transport;

public partial class IsoMessageTests
{
    public static DataChannelListPackage ValidFullyCustomDataChannelList =>
        new DataChannelListPackage
        {
            Package = new Package
            {
                Header = new Header
                {
                    ShipId = ShipId.Parse("IMO1234567"),
                    DataChannelListId = new ConfigurationReference
                    {
                        Id = "DataChannelList.xml",
                        Version = "1.0",
                        TimeStamp = DateTimeOffset.Parse("2016-01-01T00:00:00Z")
                    },
                    VersionInformation = new VersionInformation
                    {
                        NamingRule = "some_naming_rule",
                        NamingSchemeVersion = "2.0",
                        ReferenceUrl = "http://somewhere.net"
                    },
                    Author = "Author1",
                    DateCreated = DateTimeOffset.Parse("2015-12-01T00:00:00Z"),
                    CustomHeaders = new() { ["nr:CustomHeaderElement"] = "Vender specific headers" }
                },
                DataChannelList = new DataChannelList()
                {
                    new DataChannel
                    {
                        DataChannelId = new SDK.Transport.DataChannel.DataChannelId
                        {
                            LocalId = LocalId.Parse(
                                "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air"
                            ),
                            ShortId = "0010",
                            NameObject = new NameObject
                            {
                                NamingRule = "Naming_Rule",
                                CustomNameObjects = new Dictionary<string, object>()
                                {
                                    ["nr:CustomNameObject"] = "Vender specific NameObject"
                                }
                            }
                        },
                        Property = new Property
                        {
                            DataChannelType = new DataChannelType { Type = "Inst", UpdateCycle = 1, },
                            Format = new Format
                            {
                                Type = "Decimal",
                                Restriction = new Restriction
                                {
                                    FractionDigits = 1,
                                    MaxInclusive = 200.0,
                                    MinInclusive = -150.0,
                                }
                            },
                            Range = new SDK.Transport.DataChannel.Range { Low = 0.0, High = 150.0 },
                            Unit = new Unit { UnitSymbol = "°C", QuantityName = "Temperature", },
                            QualityCoding = "OPC_QUALITY",
                            AlertPriority = null,
                            Name = "M/E #1 Air Cooler CFW OUT Temp",
                            Remarks = " Location: ECR, Manufacturer: AAA Company, Type: TYPE-AAA ",
                            CustomProperties = new Dictionary<string, object>()
                            {
                                ["nr:CustomPropertyElement"] = "Vender specific Property"
                            }
                        }
                    },
                    new DataChannel
                    {
                        DataChannelId = new SDK.Transport.DataChannel.DataChannelId
                        {
                            LocalId = LocalId.Parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power"),
                            ShortId = "0020",
                        },
                        Property = new Property
                        {
                            DataChannelType = new DataChannelType { Type = "Alert", },
                            Format = new Format
                            {
                                Type = "String",
                                Restriction = new Restriction { MaxLength = 100, MinLength = 0 }
                            },
                            Range = null,
                            Unit = null,
                            AlertPriority = "Warning",
                        }
                    }
                }
            }
        };
    public static DataChannelListPackage ValidDataChannelList =>
        new DataChannelListPackage
        {
            Package = new Package
            {
                Header = new Header
                {
                    Author = "some-author",
                    DataChannelListId = new ConfigurationReference
                    {
                        Id = "some-id",
                        TimeStamp = DateTimeOffset.Parse("2016-01-01T00:00:00Z")
                    },
                    ShipId = ShipId.Parse("IMO1234567"),
                },
                DataChannelList = new DataChannelList()
                {
                    new DataChannel
                    {
                        DataChannelId = new SDK.Transport.DataChannel.DataChannelId
                        {
                            LocalId = LocalId.Parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power"),
                            ShortId = "0010",
                        },
                        Property = new Property
                        {
                            DataChannelType = new DataChannelType { Type = "Inst", },
                            Format = new Format { Type = "String", Restriction = null },
                            Range = null,
                            Unit = null,
                            AlertPriority = null,
                        }
                    }
                },
            }
        };

    [Fact]
    public void Test_DataChannelList()
    {
        var message = ValidDataChannelList;

        Assert.NotNull(message);
    }

    [Fact]
    public void Test_LocalId_Lookup()
    {
        var message = ValidDataChannelList;
        var dataChannel = message.Package.DataChannelList[0];
        var localId = dataChannel.DataChannelId.LocalId;
        var lookup = message.Package.DataChannelList[localId];
        Assert.True(message.Package.DataChannelList.TryGetByLocalId(localId, out var lookup2));
        dataChannel.Should().BeEquivalentTo(lookup);
        dataChannel.Should().BeEquivalentTo(lookup2);
    }

    [Fact]
    public void Test_ShortId_Lookup()
    {
        var message = ValidDataChannelList;
        var dataChannel = message.Package.DataChannelList[0];
        var shortId = dataChannel.DataChannelId.ShortId;
        Assert.NotNull(shortId);
        var lookup = message.Package.DataChannelList[shortId];
        Assert.True(message.Package.DataChannelList.TryGetByShortId(shortId, out var lookup2));
        dataChannel.Should().BeEquivalentTo(lookup);
        dataChannel.Should().BeEquivalentTo(lookup2);
    }

    [Fact]
    public void Test_DataChannelList_Enumerator()
    {
        var message = ValidDataChannelList;

        var expectedLength = message.DataChannelList.DataChannels.Count;
        var actualLength = message.DataChannelList.Count;
        var counter = 0;
        Assert.Equal(expectedLength, actualLength);
        foreach (var dc in ValidDataChannelList.DataChannelList)
        {
            Assert.NotNull(dc);
            counter++;
        }
        Assert.Equal(expectedLength, counter);
    }

    [Fact]
    public void Test_DataChannelList_Json()
    {
        var message = ValidDataChannelList;

        var dto = SDK.Transport.Json.DataChannel.Extensions.ToJsonDto(message);
        var message2 = SDK.Transport.Json.DataChannel.Extensions.ToDomainModel(dto);

        message.Should().BeEquivalentTo(message2);
    }
}
