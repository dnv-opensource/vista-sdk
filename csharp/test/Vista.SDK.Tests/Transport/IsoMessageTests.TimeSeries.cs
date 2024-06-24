using FluentAssertions;
using Vista.SDK.Transport;
using Vista.SDK.Transport.TimeSeries;
using DataChannel = Vista.SDK.Transport.DataChannel;

namespace Vista.SDK.Tests.Transport;

public partial class IsoMessageTests
{
    public static DataChannel.DataChannelListPackage TestDataChannelListPackage => ValidFullyCustomDataChannelList;
    public static TimeSeriesDataPackage TestTimeSeriesDataPackage =>
        new TimeSeriesDataPackage
        {
            Package = new Package
            {
                Header = new Header
                {
                    ShipId = ShipId.Parse("IMO1234567"),
                    TimeSpan = new SDK.Transport.TimeSeries.TimeSpan
                    {
                        Start = DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                        End = DateTimeOffset.Parse("2016-01-03T12:00:00Z")
                    },
                    DateCreated = DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                    DateModified = DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                    Author = "Shipboard data server",
                    SystemConfiguration =
                    [
                        new ConfigurationReference {
                            Id = "SystemConfiguration.xml",
                            TimeStamp = DateTimeOffset.Parse("2016-01-03T00:00:00Z")
                        },
                        new ConfigurationReference {
                            Id = "SystemConfiguration.xml",
                            TimeStamp = DateTimeOffset.Parse("2016-01-03T00:00:00Z")
                        }
                    ],
                },
                TimeSeriesData =
                [
                    new TimeSeriesData {
                    DataConfiguration = new ConfigurationReference {
                        Id = TestDataChannelListPackage.Package.Header.DataChannelListId.Id, TimeStamp = TestDataChannelListPackage.Package.Header.DataChannelListId.TimeStamp
                    },
                    TabularData = [
                        new TabularData {
                            DataChannelIds = TestDataChannelListPackage.DataChannelList.Select(dc => DataChannelId.Parse(dc.DataChannelId.ShortId ?? dc.DataChannelId.LocalId.ToString())).ToList(),
                            DataSets = [
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                                    Value = TestDataChannelListPackage.DataChannelList.Select((dc, i) => $"{100.0 * (i+1)}").ToList(),
                                    Quality = TestDataChannelListPackage.DataChannelList.Select((dc) => "0").ToList(),
                                },
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-02T12:00:00Z"),
                                    Value = TestDataChannelListPackage.DataChannelList.Select((dc, i) => $"{105.0 * (i+1)}").ToList(),
                                    Quality = TestDataChannelListPackage.DataChannelList.Select((dc) => "0").ToList(),
                                }
                            ]
                        },
                        new TabularData {
                            DataChannelIds = [DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[0].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[0].DataChannelId.LocalId.ToString())],
                            DataSets = [
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                                    Value = ["100.0"],
                                    Quality = null
                                },
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-02T12:00:00Z"),
                                    Value = ["100.1", ],
                                    Quality = null
                                },
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                                    Value = ["100.2", ],
                                    Quality = null
                                }
                            ]
                        },
                    ],
                    EventData = new EventData() {
                        DataSet = [new EventDataSet {
                            TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                            DataChannelId = DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[1].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[1].DataChannelId.LocalId.ToString()),
                            Value = "HIGH", Quality = "0"
                        },new EventDataSet {
                            TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                            DataChannelId = DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[1].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[1].DataChannelId.LocalId.ToString()),
                            Value = "LOW", Quality = "0"
                        }, new EventDataSet {
                            TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                            DataChannelId = DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[1].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[1].DataChannelId.LocalId.ToString()),
                            Value = "AVERAGE", Quality = "0"
                        }]
                    }
                },
                    new TimeSeriesData {
                    DataConfiguration = new ConfigurationReference {
                        Id = TestDataChannelListPackage.Package.Header.DataChannelListId.Id, TimeStamp = TestDataChannelListPackage.Package.Header.DataChannelListId.TimeStamp
                    },
                    TabularData = [
                        new TabularData {
                            DataChannelIds = TestDataChannelListPackage.DataChannelList.Select(dc => DataChannelId.Parse(dc.DataChannelId.ShortId ?? dc.DataChannelId.LocalId.ToString())).ToList(),
                            DataSets = [
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                                    Value = TestDataChannelListPackage.DataChannelList.Select((dc, i) => $"{100.0 * (i+1)}").ToList(),
                                    Quality = TestDataChannelListPackage.DataChannelList.Select((dc) => "0").ToList(),
                                },
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-02T12:00:00Z"),
                                    Value = TestDataChannelListPackage.DataChannelList.Select((dc, i) => $"{105.0 * (i+1)}").ToList(),
                                    Quality = TestDataChannelListPackage.DataChannelList.Select((dc) => "0").ToList(),
                                }
                            ]
                        },
                        new TabularData {
                            DataChannelIds = [DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[0].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[0].DataChannelId.LocalId.ToString())],
                            DataSets = [
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                                    Value = ["100.0"],
                                    Quality = null
                                },
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-02T12:00:00Z"),
                                    Value = ["100.1"],
                                    Quality = null
                                },
                                new TabularDataSet {
                                    TimeStamp = DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                                    Value = ["100.2"],
                                    Quality = null
                                }
                            ]
                        },
                    ],
                    EventData = new EventData() {
                        DataSet = [new EventDataSet {
                            TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                            DataChannelId = DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[1].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[1].DataChannelId.LocalId.ToString()),
                            Value = "HIGH", Quality = "0"
                        },new EventDataSet {
                            TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                            DataChannelId = DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[1].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[1].DataChannelId.LocalId.ToString()),
                            Value = "LOW", Quality = "0"
                        }, new EventDataSet {
                            TimeStamp = DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                            DataChannelId = DataChannelId.Parse(TestDataChannelListPackage.DataChannelList[1].DataChannelId.ShortId ?? TestDataChannelListPackage.DataChannelList[1].DataChannelId.LocalId.ToString()),
                            Value = "AVERAGE", Quality = "0"
                        }]
                    }
                }
                ]
            }
        };

    [Fact]
    public void Test_TimeSeriesData()
    {
        var message = TestTimeSeriesDataPackage;

        Assert.NotNull(message);
    }

    [Fact]
    public void Test_TimeSeriesData_Json()
    {
        var message = TestTimeSeriesDataPackage;

        var dto = SDK.Transport.Json.TimeSeriesData.Extensions.ToJsonDto(message);
        var message2 = SDK.Transport.Json.TimeSeriesData.Extensions.ToDomainModel(dto);

        message.Should().BeEquivalentTo(message2);
    }

    [Fact]
    public void Test_TimeSeriesData_Validation()
    {
        var dcList = TestDataChannelListPackage;
        var message = TestTimeSeriesDataPackage;

        foreach (var tsData in message.Package.TimeSeriesData)
        {
            var result = tsData.Validate(
                dcList,
                onTabularData: (timestamp, dc, value, quality) =>
                {
                    return new ValidateResult.Ok();
                },
                onEventData: (timestamp, dc, value, quality) =>
                {
                    return new ValidateResult.Ok();
                }
            );

            result.Should().BeOfType<ValidateResult.Ok>();
        }
    }
}
