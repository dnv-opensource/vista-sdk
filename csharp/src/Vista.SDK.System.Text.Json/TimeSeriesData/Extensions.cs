using Domain = Vista.SDK.Transport.TimeSeries;

namespace Vista.SDK.Transport.Json.TimeSeriesData;

public static class Extensions
{
    public static TimeSeriesDataPackage ToJsonDto(this Domain.TimeSeriesDataPackage package)
    {
        var p = package.Package;
        var h = package.Package.Header;
        return new TimeSeriesDataPackage(
            new Package(
                h is null
                    ? null
                    : new Header(
                        h.Author,
                        h.DateCreated,
                        h.DateModified,
                        h.ShipId.ToString(),
                        h.SystemConfiguration is null
                            ? null
                            : h.SystemConfiguration.Select(r => new ConfigurationReference(r.Id, r.TimeStamp)).ToList(),
                        h.TimeSpan is null ? null : new TimeSpan(h.TimeSpan.End, h.TimeSpan.Start)
                    )
                    {
                        CustomHeaders = h.CustomHeaders?.CopyProperties()
                    },
                p.TimeSeriesData
                    .Select(
                        t =>
                            new TimeSeriesData(
                                t.DataConfiguration is null
                                    ? null
                                    : new ConfigurationReference(t.DataConfiguration.Id, t.DataConfiguration.TimeStamp),
                                t.EventData is null
                                    ? null
                                    : new EventData(
                                        t.EventData
                                            .DataSet
                                            ?.Select(
                                                d =>
                                                    new DataSet_Event(
                                                        d.DataChannelId.ToString(),
                                                        d.Quality,
                                                        d.TimeStamp,
                                                        d.Value
                                                    )
                                            )
                                            .ToList(),
                                        t.EventData.NumberOfDataSet
                                    ),
                                t.TabularData
                                    ?.Select(
                                        d =>
                                            new TabularData(
                                                d.DataChannelIds?.Select(i => i.ToString()).ToList(),
                                                d.DataSets
                                                    ?.Select(
                                                        td => new DataSet_Tabular(td.Quality, td.TimeStamp, td.Value)
                                                    )
                                                    .ToList(),
                                                d.NumberOfDataChannels,
                                                d.NumberOfDataSets
                                            )
                                    )
                                    .ToList()
                            )
                            {
                                CustomData = t.CustomDataKinds?.CopyProperties()
                            }
                    )
                    .ToList()
            )
        );
    }

    public static Domain.TimeSeriesDataPackage ToDomainModel(this TimeSeriesDataPackage package)
    {
        var p = package.Package;
        var h = package.Package.Header;
        return new Domain.TimeSeriesDataPackage
        {
            Package = new Domain.Package
            {
                Header = h is null
                    ? null
                    : new Domain.Header
                    {
                        ShipId = ShipId.Parse(h.ShipID),
                        TimeSpan = h.TimeSpan is null
                            ? null
                            : new Domain.TimeSpan { Start = h.TimeSpan.Start, End = h.TimeSpan.End },
                        DateCreated = h.DateCreated,
                        DateModified = h.DateModified,
                        Author = h.Author,
                        SystemConfiguration = h.SystemConfiguration
                            ?.Select(c => new Domain.ConfigurationReference { Id = c.ID, TimeStamp = c.TimeStamp })
                            .ToList(),
                        CustomHeaders = h.CustomHeaders?.CopyProperties()
                    },
                TimeSeriesData = p.TimeSeriesData
                    .Select(
                        t =>
                            new Domain.TimeSeriesData
                            {
                                DataConfiguration = t.DataConfiguration is null
                                    ? null
                                    : new Domain.ConfigurationReference
                                    {
                                        Id = t.DataConfiguration.ID,
                                        TimeStamp = t.DataConfiguration.TimeStamp
                                    },
                                TabularData = t.TabularData
                                    ?.Select(td =>
                                    {
                                        if (td.NumberOfDataChannel != td.DataChannelID?.Count)
                                            throw new ArgumentException(
                                                "Number of data channels does not match the expected count"
                                            );
                                        if (td.NumberOfDataSet != td.DataSet?.Count)
                                            throw new ArgumentException(
                                                "Number of data sets does not match the expected count"
                                            );
                                        return new Domain.TabularData
                                        {
                                            DataChannelIds = td.DataChannelID
                                                ?.Select(i => DataChannelId.Parse(i))
                                                .ToList(),
                                            DataSets = td.DataSet
                                                ?.Select(
                                                    tds =>
                                                        new Domain.TabularDataSet
                                                        {
                                                            TimeStamp = tds.TimeStamp,
                                                            Value = tds.Value.ToList(),
                                                            Quality = tds.Quality?.ToList()
                                                        }
                                                )
                                                .ToList()
                                        };
                                    })
                                    .ToList(),
                                EventData = t.EventData is null
                                    ? null
                                    : new Domain.EventData
                                    {
                                        DataSet = t.EventData
                                            .DataSet
                                            ?.Select(
                                                ed =>
                                                    new Domain.EventDataSet
                                                    {
                                                        TimeStamp = ed.TimeStamp,
                                                        DataChannelId = DataChannelId.Parse(ed.DataChannelID),
                                                        Value = ed.Value,
                                                        Quality = ed.Quality
                                                    }
                                            )
                                            .ToList()
                                    },
                                CustomDataKinds = t.CustomData?.CopyProperties()
                            }
                    )
                    .ToList()
            }
        };
    }
}
