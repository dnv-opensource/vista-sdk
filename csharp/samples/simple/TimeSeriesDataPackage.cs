using System.Globalization;
using Vista.SDK.Transport.Json.TimeSeriesData;

record BallastEvent(
    string Mode,
    string Location,
    DateTimeOffset Start,
    DateTimeOffset Stop,
    int Volume,
    int UVTreatment,
    int Salinity,
    (double Lat, double Lng) Position
);

static class TimeSeries
{
    internal static TimeSeriesDataPackage GetPackage(IReadOnlyList<BallastEvent> sourceData)
    {
        // Shipboard server extracts sensor data and prepares to ship it to shore using the TimeSeriesDataPackage
        var now = DateTimeOffset.UtcNow;

        var dataChannelId = new[]
        {
            "Mode",
            "Location",
            "Start",
            "Stop",
            "Volume",
            "UVTreatment",
            "Salinity",
            "Latitude",
            "Longitude",
        };
        var dataAsTable = sourceData
            .Select(
                e =>
                    (
                        Timestamp: e.Start,
                        Data: new[]
                        {
                            e.Mode,
                            e.Location,
                            e.Start.ToString(),
                            e.Stop.ToString(),
                            e.Volume.ToString(CultureInfo.InvariantCulture),
                            e.UVTreatment.ToString(CultureInfo.InvariantCulture),
                            e.Salinity.ToString(CultureInfo.InvariantCulture),
                            e.Position.Lat.ToString(CultureInfo.InvariantCulture),
                            e.Position.Lng.ToString(CultureInfo.InvariantCulture),
                        }
                    )
            )
            .ToArray();

        var from = sourceData.Min(d => d.Start);
        var to = sourceData.Max(d => d.Stop);

        var data = new TimeSeriesData(
            dataConfiguration: null, // May optionally refer to a specific DataChannelList,
            eventData: new EventData([], 0),
            tabularData: new[]
            {
                new TabularData(
                    dataChannelID: dataChannelId,
                    dataSet: dataAsTable
                        .Select(d => new DataSet_Tabular(quality: null, timeStamp: d.Timestamp, value: d.Data))
                        .ToArray(),
                    numberOfDataChannel: dataChannelId.Length,
                    numberOfDataSet: dataAsTable.Length // One sample per datachannel/timestamp for this scenario
                )
            }
        );

        var header = new Header(
            author: "DNV",
            dateCreated: now,
            dateModified: now,
            shipID: "1234567",
            systemConfiguration: Array.Empty<ConfigurationReference>(), // A reference to the source system, e.g. DBMS
            timeSpan: new Vista.SDK.Transport.Json.TimeSeriesData.TimeSpan(from, to) // Oldest and newest dataset
        );
        var package = new TimeSeriesDataPackage(new Package(header, new[] { data }));
        return package;
    }
}
