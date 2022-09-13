using Domain = Vista.SDK.Transport.TimeSeries;

namespace Vista.SDK.Transport.Avro.TimeSeriesData;

public static class Extensions
{
    public static TimeSeriesDataPackage ToAvroDto(this Domain.TimeSeriesDataPackage package)
    {
        var p = package.Package;
        var h = package.Package.Header;
        return new TimeSeriesDataPackage()
        {
            Package = new Package()
            {
                Header = h is null
                    ? null
                    : new Header()
                      {
                          Author = h.Author,
                          DateCreated = h.DateCreated?.DateTime,
                          DateModified = h.DateModified?.DateTime,
                          ShipID = h.ShipId.ToString(),
                          SystemConfiguration = h.SystemConfiguration
                              ?.Select(
                                  s =>
                                      new SystemConfiguration()
                                      {
                                          ID = s.Id,
                                          TimeStamp = s.TimeStamp.DateTime
                                      }
                              )
                              .ToList(),
                          TimeSpan = h.TimeSpan is null
                              ? null
                              : new TimeSpan()
                                {
                                    Start = h.TimeSpan.Start.DateTime,
                                    End = h.TimeSpan.End.DateTime,
                                },
                      },
                TimeSeriesData = p.TimeSeriesData
                    .Select(
                        t =>
                            new TimeSeriesData()
                            {
                                DataConfiguration = t.DataConfiguration is null
                                    ? null
                                    : new DataConfiguration()
                                      {
                                          ID = t.DataConfiguration.Id,
                                          TimeStamp = t.DataConfiguration.TimeStamp.DateTime,
                                      },
                                EventData = t.EventData is null
                                    ? null
                                    : new EventData()
                                      {
                                          DataSet = t.EventData.DataSet
                                              ?.Select(
                                                  d =>
                                                      new EventDataSet()
                                                      {
                                                          DataChannelID =
                                                              d.DataChannelId.ToString(),
                                                          Quality = d.Quality,
                                                          TimeStamp = d.TimeStamp.DateTime,
                                                          Value = d.Value,
                                                      }
                                              )
                                              .ToList(),
                                          NumberOfDataSet = t.EventData.NumberOfDataSet,
                                      },
                                TabularData = t.TabularData
                                    ?.Select(
                                        td =>
                                            new TabularData()
                                            {
                                                DataChannelID = td.DataChannelId
                                                    ?.Select(i => i.ToString())
                                                    .ToList(),
                                                DataSet = td.DataSet
                                                    ?.Select(
                                                        ds =>
                                                            new TabularDataSet()
                                                            {
                                                                Quality = ds.Quality?.ToList(),
                                                                TimeStamp = ds.TimeStamp.DateTime,
                                                                Value = ds.Value.ToList(),
                                                            }
                                                    )
                                                    .ToList(),
                                                NumberOfDataSet = td.NumberOfDataSet,
                                                NumberOfDataChannel = td.NumberOfDataChannel,
                                            }
                                    )
                                    .ToList(),
                            }
                    )
                    .ToList(),
            },
        };
    }

    public static Transport.TimeSeries.TimeSeriesDataPackage ToDomainModel(
        this TimeSeriesDataPackage package
    )
    {
        var p = package.Package;
        var h = package.Package.Header;
        return new Domain.TimeSeriesDataPackage(
            new Domain.Package(
                h is null
                  ? null
                  : new Domain.Header(
                        ShipId.Parse(h.ShipID),
                        h.TimeSpan is null
                          ? null
                          : new Domain.TimeSpan(h.TimeSpan.Start, h.TimeSpan.End),
                        h.DateCreated,
                        h.DateModified,
                        h.Author,
                        h.SystemConfiguration
                            ?.Select(c => new Domain.ConfigurationReference(c.ID, c.TimeStamp))
                            .ToList(),
                        // h.AdditionalProperties.CopyProperties()
                        new Dictionary<string, object>()
                    ),
                p.TimeSeriesData
                    .Select(
                        t =>
                            new Domain.TimeSeriesData(
                                t.DataConfiguration is null
                                  ? null
                                  : new Domain.ConfigurationReference(
                                        t.DataConfiguration.ID,
                                        t.DataConfiguration.TimeStamp
                                    ),
                                t.TabularData
                                    ?.Select(
                                        td =>
                                            new Domain.TabularData(
                                                td.NumberOfDataSet,
                                                td.NumberOfDataChannel,
                                                td.DataChannelID
                                                    .Select(i => DataChannelId.Parse(i))
                                                    ?.ToList(),
                                                td.DataSet
                                                    ?.Select(
                                                        tds =>
                                                            new Domain.TabularDataSet(
                                                                tds.TimeStamp,
                                                                tds.Value.ToList(),
                                                                tds.Quality.ToList()
                                                            )
                                                    )
                                                    .ToList()
                                            )
                                    )
                                    .ToList(),
                                t.EventData is null
                                  ? null
                                  : new Domain.EventData(
                                        t.EventData.NumberOfDataSet,
                                        t.EventData.DataSet
                                            ?.Select(
                                                ed =>
                                                    new Domain.EventDataSet(
                                                        ed.TimeStamp,
                                                        DataChannelId.Parse(ed.DataChannelID),
                                                        ed.Value,
                                                        ed.Quality
                                                    )
                                            )
                                            .ToList()
                                    ),
                                // t.AdditionalProperties.CopyProperties()
                                new Dictionary<string, object>()
                            )
                    )
                    .ToList()
            )
        );
    }
}
