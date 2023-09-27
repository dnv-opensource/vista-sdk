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
                        AdditionalProperties = h.CustomHeaders.CopyProperties(),
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
                                        t.EventData.DataSet
                                            ?.Select(
                                                d =>
                                                    new EventDataSet(
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
                                                d.DataChannelId?.Select(i => i.ToString()).ToList(),
                                                d.DataSet
                                                    ?.Select(
                                                        td =>
                                                            new TabularDataSet(
                                                                td.Quality?.ToList(),
                                                                td.TimeStamp,
                                                                td.Value
                                                            )
                                                    )
                                                    .ToList(),
                                                d.NumberOfDataChannel,
                                                d.NumberOfDataSet
                                            )
                                    )
                                    .ToList()
                            )
                            {
                                AdditionalProperties = t.CustomProperties.CopyProperties(),
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
        return new Domain.TimeSeriesDataPackage(
            new Domain.Package(
                h is null
                    ? null
                    : new Domain.Header(
                        ShipId.Parse(h.ShipID),
                        h.TimeSpan is null ? null : new Domain.TimeSpan(h.TimeSpan.Start, h.TimeSpan.End),
                        h.DateCreated,
                        h.DateModified,
                        h.Author,
                        h.SystemConfiguration
                            ?.Select(c => new Domain.ConfigurationReference(c.ID, c.TimeStamp))
                            .ToList(),
                        h.AdditionalProperties.CopyProperties()
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
                                                td.DataChannelID?.Select(i => DataChannelId.Parse(i)).ToList(),
                                                td.DataSet
                                                    ?.Select(
                                                        tds =>
                                                            new Domain.TabularDataSet(
                                                                tds.TimeStamp,
                                                                tds.Value.ToList(),
                                                                tds.Quality?.ToList()
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
                                t.AdditionalProperties.CopyProperties()
                            )
                    )
                    .ToList()
            )
        );
    }
}
