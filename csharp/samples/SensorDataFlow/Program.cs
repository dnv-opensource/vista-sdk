/*
    Sensor Data Flow Example: Proprietary -> ISO19848
    This example demonstrates transforming proprietary sensor data into an
    ISO19848-compliant TimeSeriesDataPackage.
    Note: This is a simplified example for illustration purposes and may not
    cover all aspects of a production ISO19848 implementation.
    The DataChannelList serves as the mapping between system IDs (short_id)
    and ISO19848 LocalIds - no separate mapping configuration is needed.
    Flow:
        1. Load ISO19848 DataChannelList (defines valid channels + mappings)
        2. Receive proprietary sensor readings with system IDs
        3. Filter validates readings against DataChannelList
        4. Group channels by update_cycle into TabularData (ISO19848 recommendation)
        5. Produce ISO19848 TimeSeriesDataPackage
        6. Serialize to standard JSON format
*/

using Vista.SDK;
using Vista.SDK.Transport;
using DataChannelId = Vista.SDK.Transport.DataChannelId;
using DomainDc = Vista.SDK.Transport.DataChannel;
using DomainTs = Vista.SDK.Transport.TimeSeries;
using Json = Vista.SDK.Transport.Json;

Console.WriteLine(new string('=', 60));
Console.WriteLine("Sensor Data Flow: Proprietary -> ISO19848");
Console.WriteLine(new string('=', 60));

// Step 1: Load DataChannelList (defines valid channels + mappings)
Console.WriteLine("\n[1] Loading DataChannelList...");
var dcl = CreateSampleDataChannelList();
Console.WriteLine($"    Loaded {dcl.DataChannelList.Count} channels:");
foreach (var dc in dcl.DataChannelList)
{
    var cycle = dc.Property.DataChannelType.UpdateCycle;
    var cycleStr = cycle.HasValue ? $"{cycle}s" : "None";
    Console.WriteLine($"      {dc.DataChannelId.ShortId} (update_cycle={cycleStr})");
}

// Step 2: Create the filter
Console.WriteLine("\n[2] Creating ISO19848 filter...");
var isoFilter = new ISO19848Filter(dcl);
Console.WriteLine("    Filter ready");
Console.WriteLine("    (Channels with same update_cycle will be grouped)");

// Step 3: Receive sensor readings
Console.WriteLine("\n[3] Receiving sensor readings...");
var baseTime = new DateTimeOffset(2024, 6, 15, 10, 0, 0, System.TimeSpan.Zero);

// Simulate readings from multiple sensors at same timestamps
SensorReading[] readings =
[
    // Temperature readings (1.0s update cycle)
    new("TEMP001", 45.2, baseTime),
    new("TEMP001", 46.1, baseTime.AddMinutes(5)),
    // Power readings (same 1.0s update cycle - will be grouped!)
    new("PWR001", 2500.0, baseTime),
    new("PWR001", 2650.5, baseTime.AddMinutes(5)),
    // Alert channel (different update cycle)
    new("ALT001", 1.0, baseTime),
    // Unknown sensor (will be rejected)
    new("UNKNOWN", 123.4, baseTime),
];

foreach (var reading in readings)
{
    isoFilter.Receive(reading);
}

// Step 4: Flush to TimeSeriesDataPackage
Console.WriteLine("\n[4] Generating TimeSeriesDataPackage (grouped by update_cycle)...");
var shipId = ShipId.Parse("IMO1234567");
var package = isoFilter.Flush(shipId);

if (package is not null)
{
    Console.WriteLine("    ✓ Package created");
    if (package.Package.Header is not null)
    {
        var h = package.Package.Header;
        Console.WriteLine($"    Ship: {h.ShipId}");
        if (h.TimeSpan is not null)
        {
            Console.WriteLine($"    TimeSpan: {h.TimeSpan.Start} - {h.TimeSpan.End}");
        }
    }
}

// Step 5: Serialize to JSON
Console.WriteLine("\n[5] Serializing to JSON...");
if (package is not null)
{
    var jsonStr = Json.Serializer.Serialize(package);
    var byteCount = System.Text.Encoding.UTF8.GetByteCount(jsonStr);
    Console.WriteLine($"    ✓ {byteCount} bytes");
    Console.WriteLine($"    Preview: {jsonStr[..Math.Min(80, jsonStr.Length)]}...");
}

Console.WriteLine("\n" + new string('=', 60));
Console.WriteLine("Done! Data is now ISO19848-compliant.");
Console.WriteLine(new string('=', 60));

// =========================================================================
// Helper: Create sample DataChannelList
// =========================================================================
static DomainDc.DataChannelListPackage CreateSampleDataChannelList()
{
    var dcList = new DomainDc.DataChannelList();

    // Data channel 1: Temperature sensor
    var localId1 = LocalId.Parse("/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air");
    var dcId1 = new DomainDc.DataChannelId { LocalId = localId1, ShortId = "TEMP001", };
    var prop1 = new DomainDc.Property
    {
        DataChannelType = new DomainDc.DataChannelType { Type = "Inst", UpdateCycle = 1.0 },
        Format = new DomainDc.Format
        {
            Type = "Decimal",
            Restriction = new DomainDc.Restriction
            {
                FractionDigits = 1,
                MaxInclusive = 200.0,
                MinInclusive = -50.0
            },
        },
        Range = new DomainDc.Range { Low = 0.0, High = 150.0 },
        Unit = new DomainDc.Unit { UnitSymbol = "°C", QuantityName = "Temperature" },
        QualityCoding = "OPC_QUALITY",
        AlertPriority = null,
        Name = "Main Engine Air Cooler Temperature",
    };
    dcList.Add(new DomainDc.DataChannel { DataChannelId = dcId1, Property = prop1 });

    // Data channel 2: Power measurement (same update_cycle as TEMP001 for grouping)
    var localId2 = LocalId.Parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power");
    var dcId2 = new DomainDc.DataChannelId { LocalId = localId2, ShortId = "PWR001" };
    var prop2 = new DomainDc.Property
    {
        DataChannelType = new DomainDc.DataChannelType { Type = "Inst", UpdateCycle = 1.0 },
        Format = new DomainDc.Format { Type = "Decimal", Restriction = null },
        Range = new DomainDc.Range { Low = 0.0, High = 10000.0 },
        Unit = new DomainDc.Unit { UnitSymbol = "kW", QuantityName = "Power" },
        AlertPriority = null,
        Name = "Generator Power Output",
    };
    dcList.Add(new DomainDc.DataChannel { DataChannelId = dcId2, Property = prop2 });
    // Data channel 3: Alert channel (string type, no range/unit needed)
    var localId3 = LocalId.Parse("/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop");
    var dcId3 = new DomainDc.DataChannelId { LocalId = localId3, ShortId = "ALT001" };
    var prop3 = new DomainDc.Property
    {
        DataChannelType = new DomainDc.DataChannelType { Type = "Alert" },
        Format = new DomainDc.Format
        {
            Type = "String",
            Restriction = new DomainDc.Restriction { MaxLength = 100 }
        },
        Range = null,
        Unit = null,
        AlertPriority = "Warning",
        Name = "Diesel Oil Low Level Alert",
    };
    dcList.Add(new DomainDc.DataChannel { DataChannelId = dcId3, Property = prop3 });

    // Create header
    var shipId = ShipId.Parse("IMO1234567");
    var configRef = new DomainDc.ConfigurationReference
    {
        Id = "DataChannelList-v1",
        Version = "1.0",
        TimeStamp = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
    };
    var versionInfo = new DomainDc.VersionInformation
    {
        NamingRule = "dnv",
        NamingSchemeVersion = "v2",
        ReferenceUrl = "https://docs.vista.dnv.com",
    };
    var header = new DomainDc.Header
    {
        ShipId = shipId,
        DataChannelListId = configRef,
        VersionInformation = versionInfo,
        Author = "Vista SDK Sample",
        DateCreated = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
    };

    var package = new DomainDc.Package { Header = header, DataChannelList = dcList };
    return new DomainDc.DataChannelListPackage { Package = package };
}

// =========================================================================
// Types
// =========================================================================

/// <summary>
/// A proprietary sensor reading with system ID.
/// </summary>
record SensorReading(string SystemId, double Value, DateTimeOffset Timestamp, string Quality = "Good");

/// <summary>
/// Filter that transforms proprietary sensor data to ISO19848 format.
///
/// Uses the DataChannelList as the source of truth for:
/// - Valid channel IDs (short_id)
/// - Mapping from short_id to LocalId
/// - Update cycle for grouping channels (ISO19848 optimization)
///
/// Any reading with an unknown system_id is rejected.
/// Channels with the same update_cycle are grouped into one TabularData.
/// </summary>
class ISO19848Filter
{
    private readonly DomainDc.DataChannelListPackage _dataChannelListPackage;
    private readonly Dictionary<string, List<SensorReading>> _buffer = [];

    public ISO19848Filter(DomainDc.DataChannelListPackage dataChannelListPackage)
    {
        _dataChannelListPackage = dataChannelListPackage;
    }

    private double? GetUpdateCycle(DomainDc.DataChannel dc) => dc.Property.DataChannelType.UpdateCycle;

    /// <summary>
    /// Receive a sensor reading. Returns true if accepted.
    /// </summary>
    public bool Receive(SensorReading reading)
    {
        if (!_dataChannelListPackage.DataChannelList.TryGetByShortId(reading.SystemId, out _))
        {
            Console.WriteLine($"    ✗ Rejected: {reading.SystemId} (unknown channel)");
            return false;
        }

        if (!_buffer.TryGetValue(reading.SystemId, out var list))
        {
            list =  [];
            _buffer[reading.SystemId] = list;
        }

        list.Add(reading);
        Console.WriteLine($"    ✓ Accepted: {reading.SystemId} = {reading.Value}");
        return true;
    }

    /// <summary>
    /// Flush buffered readings to a TimeSeriesDataPackage.
    /// Groups channels by update_cycle into TabularData blocks.
    /// This is recommended by ISO19848 to reduce payload size.
    /// </summary>
    public DomainTs.TimeSeriesDataPackage? Flush(ShipId shipId)
    {
        if (_buffer.Count == 0)
            return null;

        // Reference the specific DataChannelList version (recommended)
        var configuration = DomainTs.ConfigurationReference.From(_dataChannelListPackage);

        // Group channels by update_cycle (use string key to avoid nullable constraint issue)
        var channelsByCycle = new Dictionary<string, List<(DomainDc.DataChannel dc, string systemId)>>();

        foreach (var systemId in _buffer.Keys)
        {
            if (_dataChannelListPackage.DataChannelList.TryGetByShortId(systemId, out var dc))
            {
                var updateCycle = GetUpdateCycle(dc);
                var cycleKey = updateCycle?.ToString() ?? "none";
                if (!channelsByCycle.TryGetValue(cycleKey, out var channelList))
                {
                    channelList =  [];
                    channelsByCycle[cycleKey] = channelList;
                }
                channelList.Add((dc, systemId));
            }
        }

        // Build one TabularData per update_cycle group
        var tabularDataList = new List<DomainTs.TabularData>();

        foreach (var (updateCycle, channelPairs) in channelsByCycle)
        {
            // Collect all unique timestamps across channels in this group
            var allTimestamps = new SortedSet<DateTimeOffset>();
            foreach (var (_, systemId) in channelPairs)
            {
                foreach (var reading in _buffer[systemId])
                {
                    allTimestamps.Add(reading.Timestamp);
                }
            }

            // Build data channel IDs list for this group
            var dataChannelIds = channelPairs
                .Select(pair => DataChannelId.Parse(pair.dc.DataChannelId.LocalId.ToString()))
                .ToList();

            // Build data sets - one per timestamp, with values for all channels
            var dataSets = new List<DomainTs.TabularDataSet>();
            foreach (var ts in allTimestamps)
            {
                var values = new List<string>();
                var qualities = new List<string>();

                foreach (var (_, systemId) in channelPairs)
                {
                    // Find reading for this channel at this timestamp
                    var reading = _buffer[systemId].FirstOrDefault(r => r.Timestamp == ts);
                    if (reading is not null)
                    {
                        values.Add(reading.Value.ToString());
                        qualities.Add(reading.Quality);
                    }
                    else
                    {
                        // No reading for this channel at this timestamp
                        values.Add("");
                        qualities.Add("Bad");
                    }
                }

                dataSets.Add(
                    new DomainTs.TabularDataSet
                    {
                        TimeStamp = ts,
                        Value = values,
                        Quality = qualities
                    }
                );
            }

            var cycleStr = updateCycle == "none" ? "None" : $"{updateCycle}s";
            Console.WriteLine(
                $"    TabularData: {channelPairs.Count} channels, {dataSets.Count} rows (update_cycle={cycleStr})"
            );

            tabularDataList.Add(new DomainTs.TabularData { DataChannelIds = dataChannelIds, DataSets = dataSets });
        }

        // Build the package
        var allTimestampsList = _buffer.Values.SelectMany(readings => readings.Select(r => r.Timestamp)).ToList();

        var pkg = new DomainTs.Package
        {
            Header = new DomainTs.Header
            {
                ShipId = shipId,
                TimeSpan = new DomainTs.TimeSpan { Start = allTimestampsList.Min(), End = allTimestampsList.Max() },
                Author = "ISO19848Filter",
            },
            TimeSeriesData =
            [
                new DomainTs.TimeSeriesData
                {
                    DataConfiguration = configuration,
                    TabularData = tabularDataList,
                    EventData = null,
                },
            ],
        };

        _buffer.Clear();
        return new DomainTs.TimeSeriesDataPackage { Package = pkg };
    }
}
