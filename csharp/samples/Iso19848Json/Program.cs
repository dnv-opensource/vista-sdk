// ISO19848 JSON Serialization Examples
//
// This sample demonstrates how to work with ISO19848 data structures,
// including JSON serialization/deserialization, domain model conversion,
// and data channel lookups.
//
// Examples:
//     - Loading DataChannelList from JSON
//     - Loading TimeSeriesData from JSON
//     - Converting between JSON DTOs and domain models
//     - Looking up data channels by ShortId and LocalId
//
// For sensor data flow examples, see SensorDataFlow sample.

using Vista.SDK;
using Vista.SDK.Transport;
using Vista.SDK.Transport.Json;
using Vista.SDK.Transport.Json.DataChannel;
using Vista.SDK.Transport.Json.TimeSeriesData;
using Domain = Vista.SDK.Transport.DataChannel;
using DomainTimeSeries = Vista.SDK.Transport.TimeSeries;

Console.WriteLine(new string('=', 60));
Console.WriteLine("ISO19848 JSON Serialization Examples");
Console.WriteLine(new string('=', 60));

// Run all examples
ExampleJsonToDomain();
ExampleGetByShortId();
ExampleGetByLocalId();
ExampleTimeSeriesJsonToDomain();
ExampleTimeSeriesJsonRoundtrip();

Console.WriteLine("\n" + new string('=', 60));
Console.WriteLine("All examples completed successfully!");
Console.WriteLine(new string('=', 60));

// =========================================================================
// Example: JSON to Domain Model Conversion
// =========================================================================
void ExampleJsonToDomain()
{
    Console.WriteLine("\n" + new string('=', 60));
    Console.WriteLine("Example: JSON to Domain Model Conversion");
    Console.WriteLine(new string('=', 60));

    // Create sample data
    // This would typically be stored somewhere either as a JSON file or constructed from a database
    var original = CreateSampleDataChannelList();
    var channelCount = original.DataChannelList.Count;
    Console.WriteLine($"\n1. Created DataChannelListPackage with {channelCount} channels");

    // Convert domain model to JSON DTO
    var jsonDto = original.ToJsonDto();
    Console.WriteLine("2. Converted to JSON DTO");

    // Serialize to JSON string
    var jsonStr = jsonDto.Serialize();
    var byteCount = System.Text.Encoding.UTF8.GetByteCount(jsonStr);
    Console.WriteLine($"3. Serialized to JSON ({byteCount} bytes)");
    Console.WriteLine($"   Preview: {jsonStr[..Math.Min(100, jsonStr.Length)]}...");

    // Deserialize JSON string to DTO
    var loadedDto = Serializer.DeserializeDataChannelList(jsonStr);
    Console.WriteLine("4. Deserialized JSON string to DTO");

    // Convert DTO back to domain model
    if (loadedDto is not null)
    {
        var domainModel = loadedDto.ToDomainModel();
        var dcCount = domainModel.DataChannelList.Count;
        Console.WriteLine($"5. Converted to domain model with {dcCount} channels");
    }

    Console.WriteLine("\n✓ JSON roundtrip successful!");
}

// =========================================================================
// Example: Get Data Channel by ShortId
// =========================================================================
void ExampleGetByShortId()
{
    Console.WriteLine("\n" + new string('=', 60));
    Console.WriteLine("Example: Get Data Channel by ShortId");
    Console.WriteLine(new string('=', 60));

    // Create sample data
    var package = CreateSampleDataChannelList();
    var dcList = package.DataChannelList;

    // Look up by ShortId - typically your system IDs
    string[] shortIdsToFind = ["TEMP001", "PWR001", "ALT001", "UNKNOWN"];

    foreach (var shortId in shortIdsToFind)
    {
        if (dcList.TryGetByShortId(shortId, out var dataChannel))
        {
            Console.WriteLine($"\n✓ Found ShortId '{shortId}':");
            Console.WriteLine($"  Name: {dataChannel.Property.Name}");
            Console.WriteLine($"  LocalId: {dataChannel.DataChannelId.LocalId}");
            Console.WriteLine($"  Type: {dataChannel.Property.DataChannelType.Type}");
            if (dataChannel.Property.Unit is not null)
            {
                Console.WriteLine($"  Unit: {dataChannel.Property.Unit.UnitSymbol}");
            }
        }
        else
        {
            Console.WriteLine($"\n✗ ShortId '{shortId}' not found");
        }
    }
}

// =========================================================================
// Example: Get Data Channel by LocalId
// =========================================================================
void ExampleGetByLocalId()
{
    Console.WriteLine("\n" + new string('=', 60));
    Console.WriteLine("Example: Get Data Channel by LocalId");
    Console.WriteLine(new string('=', 60));

    // Create sample data
    var package = CreateSampleDataChannelList();
    var dcList = package.DataChannelList;

    // Look up by LocalId
    string[] localIdStrs =
    [
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air",
        "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power",
        "/dnv-v2/vis-3-4a/999.99/meta/unknown", // Non-existent
    ];

    foreach (var localIdStr in localIdStrs)
    {
        if (LocalId.TryParse(localIdStr, out _, out var localId))
        {
            if (dcList.TryGetByLocalId(localId, out var dataChannel))
            {
                Console.WriteLine($"\n✓ Found LocalId:");
                Console.WriteLine($"  LocalId: {localId}");
                Console.WriteLine($"  ShortId: {dataChannel.DataChannelId.ShortId}");
                Console.WriteLine($"  Name: {dataChannel.Property.Name}");
            }
            else
            {
                Console.WriteLine($"\n✗ LocalId not found: {localIdStr}");
            }
        }
        else
        {
            Console.WriteLine($"\n✗ Invalid LocalId: {localIdStr}");
        }
    }
}

// =========================================================================
// Example: TimeSeriesData JSON to Domain Model
// =========================================================================
void ExampleTimeSeriesJsonToDomain()
{
    Console.WriteLine("\n" + new string('=', 60));
    Console.WriteLine("Example: TimeSeriesData JSON to Domain Model");
    Console.WriteLine(new string('=', 60));

    // Create sample data
    var dcListPackage = CreateSampleDataChannelList();
    var original = CreateSampleTimeSeriesData(dcListPackage);

    // Serialize to JSON
    var jsonDto = original.ToJsonDto();
    var jsonStr = jsonDto.Serialize();
    var byteCount = System.Text.Encoding.UTF8.GetByteCount(jsonStr);

    Console.WriteLine($"\n1. Received JSON payload ({byteCount} bytes)");
    Console.WriteLine($"   Preview: {jsonStr[..Math.Min(80, jsonStr.Length)]}...");

    // Deserialize JSON string to DTO
    var dto = Serializer.DeserializeTimeSeriesData(jsonStr);
    Console.WriteLine("2. Deserialized to JSON DTO");

    // Convert to domain model
    if (dto is not null)
    {
        var domain = dto.ToDomainModel();
        Console.WriteLine("3. Converted to domain model");

        // Access structured data
        Console.WriteLine("\n4. Accessing domain model data:");
        if (domain.Package.Header is not null)
        {
            var h = domain.Package.Header;
            Console.WriteLine($"   Ship ID: {h.ShipId}");
            Console.WriteLine($"   Author: {h.Author}");
            if (h.TimeSpan is not null)
            {
                Console.WriteLine($"   Time Span: {h.TimeSpan.Start} to {h.TimeSpan.End}");
            }
        }

        for (var i = 0; i < domain.Package.TimeSeriesData.Count; i++)
        {
            var tsData = domain.Package.TimeSeriesData[i];
            Console.WriteLine($"\n   TimeSeriesData #{i + 1}:");
            if (tsData.TabularData is not null)
            {
                for (var j = 0; j < tsData.TabularData.Count; j++)
                {
                    var tab = tsData.TabularData[j];
                    if (tab.DataChannelIds is null || tab.DataSets is null)
                        continue;
                    Console.WriteLine($"     Tabular #{j + 1}: {tab.DataChannelIds.Count} channels");
                    Console.WriteLine($"       {tab.DataSets.Count} data sets");
                }
            }
            if (tsData.EventData?.DataSet is not null)
            {
                Console.WriteLine($"     Events: {tsData.EventData.DataSet.Count} events");
            }
        }
    }

    Console.WriteLine("\n✓ JSON to domain conversion successful!");
}

// =========================================================================
// Example: TimeSeriesData JSON Roundtrip
// =========================================================================
void ExampleTimeSeriesJsonRoundtrip()
{
    Console.WriteLine("\n" + new string('=', 60));
    Console.WriteLine("Example: TimeSeriesData JSON Roundtrip");
    Console.WriteLine(new string('=', 60));

    // Create sample data
    var dcListPackage = CreateSampleDataChannelList();
    var original = CreateSampleTimeSeriesData(dcListPackage);

    Console.WriteLine("\n1. Created TimeSeriesDataPackage");
    if (original.Package.Header is not null)
    {
        Console.WriteLine($"   Ship: {original.Package.Header.ShipId}");
        if (original.Package.Header.TimeSpan is not null)
        {
            var ts = original.Package.Header.TimeSpan;
            Console.WriteLine($"   TimeSpan: {ts.Start} - {ts.End}");
        }
    }

    // Convert to JSON DTO and serialize
    var jsonDto = original.ToJsonDto();
    var jsonStr = jsonDto.Serialize();
    var byteCount = System.Text.Encoding.UTF8.GetByteCount(jsonStr);
    Console.WriteLine($"2. Serialized to JSON ({byteCount} bytes)");

    // Deserialize and convert back
    var loadedDto = Serializer.DeserializeTimeSeriesData(jsonStr);
    if (loadedDto is not null)
    {
        var domainModel = loadedDto.ToDomainModel();
        Console.WriteLine("3. Deserialized and converted to domain model");

        // Verify structure
        var origLen = original.Package.TimeSeriesData.Count;
        var loadedLen = domainModel.Package.TimeSeriesData.Count;
        if (origLen != loadedLen)
        {
            Console.WriteLine($"   ✗ TimeSeriesData count mismatch: {origLen} vs {loadedLen}");
        }
        else
        {
            Console.WriteLine($"   ✓ TimeSeriesData count matches: {origLen}");
        }
    }

    Console.WriteLine("\n✓ JSON roundtrip successful!");
}

// =========================================================================
// Helper: Create sample DataChannelList
// =========================================================================
Domain.DataChannelListPackage CreateSampleDataChannelList()
{
    var dcList = new Domain.DataChannelList();

    // Data channel 1: Temperature sensor
    var localId1 = LocalId.Parse("/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air");
    var dcId1 = new Domain.DataChannelId { LocalId = localId1, ShortId = "TEMP001" };
    var prop1 = new Domain.Property
    {
        DataChannelType = new Domain.DataChannelType { Type = "Inst", UpdateCycle = 1.0 },
        Format = new Domain.Format
        {
            Type = "Decimal",
            Restriction = new Domain.Restriction
            {
                FractionDigits = 1,
                MaxInclusive = 200.0,
                MinInclusive = -50.0
            },
        },
        Range = new Domain.Range { Low = 0.0, High = 150.0 },
        Unit = new Domain.Unit { UnitSymbol = "°C", QuantityName = "Temperature" },
        QualityCoding = "OPC_QUALITY",
        AlertPriority = null,
        Name = "Main Engine Air Cooler Temperature",
    };
    dcList.Add(new Domain.DataChannel { DataChannelId = dcId1, Property = prop1 });

    // Data channel 2: Power measurement
    var localId2 = LocalId.Parse("/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power");
    var dcId2 = new Domain.DataChannelId { LocalId = localId2, ShortId = "PWR001" };
    var prop2 = new Domain.Property
    {
        DataChannelType = new Domain.DataChannelType { Type = "Inst", UpdateCycle = 1.0 },
        Format = new Domain.Format { Type = "Decimal", Restriction = null },
        Range = new Domain.Range { Low = 0.0, High = 10000.0 },
        Unit = new Domain.Unit { UnitSymbol = "kW", QuantityName = "Power" },
        AlertPriority = null,
        Name = "Generator Power Output",
    };
    dcList.Add(new Domain.DataChannel { DataChannelId = dcId2, Property = prop2 });

    // Data channel 3: Alert channel
    var localId3 = LocalId.Parse("/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop");
    var dcId3 = new Domain.DataChannelId { LocalId = localId3, ShortId = "ALT001" };
    var prop3 = new Domain.Property
    {
        DataChannelType = new Domain.DataChannelType { Type = "Alert" },
        Format = new Domain.Format
        {
            Type = "String",
            Restriction = new Domain.Restriction { MaxLength = 100 }
        },
        Range = null,
        Unit = null,
        AlertPriority = "Warning",
        Name = "Diesel Oil Low Level Alert",
    };
    dcList.Add(new Domain.DataChannel { DataChannelId = dcId3, Property = prop3 });

    // Create header
    var shipId = ShipId.Parse("IMO1234567");
    var configRef = new Domain.ConfigurationReference
    {
        Id = "DataChannelList-v1",
        Version = "1.0",
        TimeStamp = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
    };
    var versionInfo = new Domain.VersionInformation
    {
        NamingRule = "dnv",
        NamingSchemeVersion = "v2",
        ReferenceUrl = "https://docs.vista.dnv.com",
    };
    var header = new Domain.Header
    {
        ShipId = shipId,
        DataChannelListId = configRef,
        VersionInformation = versionInfo,
        Author = "Vista SDK Sample",
        DateCreated = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
    };

    var package = new Domain.Package { Header = header, DataChannelList = dcList };
    return new Domain.DataChannelListPackage { Package = package };
}

// =========================================================================
// Helper: Create sample TimeSeriesData
// =========================================================================
DomainTimeSeries.TimeSeriesDataPackage CreateSampleTimeSeriesData(Domain.DataChannelListPackage dcListPackage)
{
    // Get data channel IDs from the list
    var dcIds = dcListPackage
        .DataChannelList
        .Select(dc => DataChannelId.Parse(dc.DataChannelId.LocalId.ToString()))
        .ToList();

    // Create header
    var shipId = ShipId.Parse("IMO1234567");
    var timeSpan = new DomainTimeSeries.TimeSpan
    {
        Start = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
        End = new DateTimeOffset(2024, 1, 1, 1, 0, 0, System.TimeSpan.Zero),
    };
    var header = new DomainTimeSeries.Header
    {
        ShipId = shipId,
        TimeSpan = timeSpan,
        Author = "Vista SDK Sample",
        DateCreated = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
    };

    // Create tabular data with measurements (first two channels - numeric)
    var tabularData = new DomainTimeSeries.TabularData
    {
        DataChannelIds =  [.. dcIds.Take(2)],
        DataSets =
        [
            new DomainTimeSeries.TabularDataSet
            {
                TimeStamp = new DateTimeOffset(2024, 1, 1, 0, 0, 0, System.TimeSpan.Zero),
                Value = ["25.5", "1500.0"],
                Quality = ["0", "0"],
            },
            new DomainTimeSeries.TabularDataSet
            {
                TimeStamp = new DateTimeOffset(2024, 1, 1, 0, 30, 0, System.TimeSpan.Zero),
                Value = ["26.1", "1520.0"],
                Quality = ["0", "0"],
            },
            new DomainTimeSeries.TabularDataSet
            {
                TimeStamp = new DateTimeOffset(2024, 1, 1, 1, 0, 0,     System.TimeSpan.Zero),
                Value = ["25.8", "1480.0"],
                Quality = ["0", "0"],
            },
        ],
    };

    // Create event data with alerts (third channel)
    var eventData = new DomainTimeSeries.EventData
    {
        DataSet =
        [
            new DomainTimeSeries.EventDataSet
            {
                TimeStamp = new DateTimeOffset(2024, 1, 1, 0, 15, 0, System.TimeSpan.Zero),
                DataChannelId = dcIds[2],
                Value = "LOW_LEVEL_WARNING",
                Quality = "0",
            },
            new DomainTimeSeries.EventDataSet
            {
                TimeStamp = new DateTimeOffset(2024, 1, 1, 0, 45, 0, System.TimeSpan.Zero),
                DataChannelId = dcIds[2],
                Value = "NORMAL",
                Quality = "0",
            },
        ],
    };

    // Create TimeSeriesData with reference to DataChannelList
    var dataConfig = new DomainTimeSeries.ConfigurationReference
    {
        Id = dcListPackage.Package.Header.DataChannelListId.Id,
        TimeStamp = dcListPackage.Package.Header.DataChannelListId.TimeStamp,
    };
    var tsData = new DomainTimeSeries.TimeSeriesData
    {
        DataConfiguration = dataConfig,
        TabularData =  [tabularData],
        EventData = eventData,
    };

    var pkg = new DomainTimeSeries.Package { Header = header, TimeSeriesData =  [tsData] };
    return new DomainTimeSeries.TimeSeriesDataPackage { Package = pkg };
}
