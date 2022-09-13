using FluentAssertions;
using Vista.SDK.Transport;
using Vista.SDK.Transport.TimeSeries;

namespace Vista.SDK.Tests.Transport;

public partial class IsoMessageTests
{
    public static TimeSeriesDataPackage TestTimeSeriesDataPackage =>
        new TimeSeriesDataPackage(
            new Package(
                new Header(
                    ShipId.Parse("IMO1234567"),
                    new SDK.Transport.TimeSeries.TimeSpan(
                        DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                        DateTimeOffset.Parse("2016-01-03T12:00:00Z")
                    ),
                    DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                    DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                    "Shipboard data server",
                    new[]
                    {
                        new ConfigurationReference(
                            "SystemConfiguration.xml",
                            DateTimeOffset.Parse("2016-01-01T00:00:00Z")
                        ),
                        new ConfigurationReference(
                            " SystemConfiguration.xml ",
                            DateTimeOffset.Parse("2016-01-03T00:00:00Z")
                        ),
                    },
                    new Dictionary<string, object>()
                ),
                new[]
                {
                    new TimeSeriesData(
                        new ConfigurationReference(
                            "DataChannelList.xml",
                            DateTimeOffset.Parse("2016-01-01T00:00:00Z")
                        ),
                        TabularData: new[]
                        {
                            new TabularData(
                                "2",
                                "2",
                                new[] { DataChannelId.Parse("0010"), DataChannelId.Parse("0020") },
                                new[]
                                {
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                                        new[] { "100.0", "200.0" },
                                        new[] { "0", "0" }
                                    ),
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-02T12:00:00Z"),
                                        new[] { "100.5", "205.0" },
                                        new[] { "0", "0" }
                                    ),
                                }
                            ),
                            new TabularData(
                                "3",
                                "1",
                                new[] { DataChannelId.Parse("0110") },
                                new[]
                                {
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-01T12:00:00Z"),
                                        new[] { "100.0" },
                                        new[] { "0" }
                                    ),
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-02T00:00:00Z"),
                                        new[] { "100.2" },
                                        new[] { "0" }
                                    ),
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-02T12:00:00Z"),
                                        new[] { "100.3" },
                                        new[] { "0" }
                                    ),
                                }
                            )
                        },
                        EventData: new EventData(
                            "3",
                            new[]
                            {
                                new EventDataSet(
                                    DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                                    DataChannelId.Parse("0011"),
                                    "HIGH",
                                    "0"
                                ),
                                new EventDataSet(
                                    DateTimeOffset.Parse("2016-01-01T12:00:01Z"),
                                    DataChannelId.Parse("0021"),
                                    "HIGH",
                                    "0"
                                ),
                                new EventDataSet(
                                    DateTimeOffset.Parse("2016-01-01T12:00:23Z"),
                                    DataChannelId.Parse("0011"),
                                    "NORMAL",
                                    "0"
                                ),
                            }
                        ),
                        new Dictionary<string, object>()
                    ),
                    new TimeSeriesData(
                        new ConfigurationReference(
                            "DataChannelList.xml",
                            DateTimeOffset.Parse("2016-01-03T00:00:00Z")
                        ),
                        TabularData: new[]
                        {
                            new TabularData(
                                "1",
                                "2",
                                new[] { DataChannelId.Parse("0010"), DataChannelId.Parse("0020"), },
                                new[]
                                {
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                                        new[] { "101.0", "210.0" },
                                        new[] { "0", "0" }
                                    ),
                                }
                            ),
                            new TabularData(
                                "2",
                                "1",
                                new[] { DataChannelId.Parse("0110"), },
                                new[]
                                {
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-03T00:00:00Z"),
                                        new[] { "100.8", },
                                        new[] { "0", }
                                    ),
                                    new TabularDataSet(
                                        DateTimeOffset.Parse("2016-01-03T12:00:00Z"),
                                        new[] { "101.0", },
                                        new[] { "0", }
                                    ),
                                }
                            ),
                        },
                        EventData: null,
                        new Dictionary<string, object>()
                    ),
                }
            )
        );

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
}
