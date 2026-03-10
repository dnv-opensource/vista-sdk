/*
    ISO19848 JSON Serialization Examples

    This sample demonstrates how to work with ISO19848 data structures,
    including JSON serialization/deserialization, domain model conversion,
    and data channel lookups.

    Examples:
        - Loading DataChannelList from JSON
        - Loading TimeSeriesData from JSON
        - Converting between JSON DTOs and domain models
        - Looking up data channels by ShortId and LocalId
        - JSON roundtrip serialization

    For sensor data flow examples, see SensorDataFlow sample.
*/

import {
    DataChannelId,
    DataChannelList,
    JSONExtensions,
    JSONSerializer,
    LocalId,
    ShipId,
    TimeSeries,
    Version,
} from "dnv-vista-sdk";

// =========================================================================
// Helper: Create sample DataChannelList
// =========================================================================
async function createSampleDataChannelList(): Promise<DataChannelList.DataChannelListPackage> {
    const localId1 = await LocalId.parseAsync(
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air",
    );
    const localId2 = await LocalId.parseAsync(
        "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power",
    );
    const localId3 = await LocalId.parseAsync(
        "/dnv-v2/vis-3-4a/621.22i/S110/meta/cnt-diesel.oil/cmd-stop",
    );

    const shipId = ShipId.parse("IMO1234567");
    const timestamp = new Date("2024-01-01T00:00:00Z");

    return {
        package: {
            header: {
                shipId,
                dataChannelListId: {
                    id: "DataChannelList-v1",
                    version: Version.parse("1.0"),
                    timestamp,
                },
                versionInformation: {
                    namingRule: "dnv",
                    namingSchemeVersion: "v2",
                    referenceUrl: "https://docs.vista.dnv.com",
                },
                author: "Vista SDK Sample",
                dateCreated: timestamp,
            },
            dataChannelList: {
                dataChannel: [
                    {
                        dataChannelId: {
                            localId: localId1,
                            shortId: "TEMP001",
                        },
                        property: {
                            dataChannelType: {
                                type: "Inst",
                                updateCycle: 1.0,
                            },
                            format: {
                                type: "Decimal",
                                restriction: {
                                    fractionDigits: 1,
                                    maxInclusive: 200.0,
                                    minInclusive: -50.0,
                                },
                            },
                            range: { low: 0.0, high: 150.0 },
                            unit: {
                                unitSymbol: "°C",
                                quantityName: "Temperature",
                            },
                            qualityCoding: "OPC_QUALITY",
                            name: "Main Engine Air Cooler Temperature",
                        },
                    },
                    {
                        dataChannelId: {
                            localId: localId2,
                            shortId: "PWR001",
                        },
                        property: {
                            dataChannelType: {
                                type: "Inst",
                                updateCycle: 1.0,
                            },
                            format: { type: "Decimal" },
                            range: { low: 0.0, high: 10000.0 },
                            unit: {
                                unitSymbol: "kW",
                                quantityName: "Power",
                            },
                            name: "Generator Power Output",
                        },
                    },
                    {
                        dataChannelId: {
                            localId: localId3,
                            shortId: "ALT001",
                        },
                        property: {
                            dataChannelType: { type: "Alert" },
                            format: {
                                type: "String",
                                restriction: { maxLength: 100 },
                            },
                            alertPriority: "Warning",
                            name: "Diesel Oil Low Level Alert",
                        },
                    },
                ],
            },
        },
    };
}

// =========================================================================
// Helper: Create sample TimeSeriesData
// =========================================================================
async function createSampleTimeSeriesData(
    dcListPackage: DataChannelList.DataChannelListPackage,
): Promise<TimeSeries.TimeSeriesDataPackage> {
    const shipId = ShipId.parse("IMO1234567");

    // Create header
    const header: TimeSeries.Header = {
        shipId,
        timeSpan: {
            start: new Date("2024-01-01T00:00:00Z"),
            end: new Date("2024-01-01T01:00:00Z"),
        },
        author: "Vista SDK Sample",
        dateCreated: new Date("2024-01-01T00:00:00Z"),
    };

    // Get data channel IDs from the list (first two for tabular, third for events)
    const channels = dcListPackage.package.dataChannelList.dataChannel;

    // Create tabular data with measurements (first two channels - numeric)
    // Use parseAsync to create DataChannelId from LocalId strings
    const tabularChannelIds: DataChannelId[] = [];
    for (const dc of channels.slice(0, 2)) {
        tabularChannelIds.push(
            await DataChannelId.parseAsync(dc.dataChannelId.localId.toString()),
        );
    }

    const tabularData: TimeSeries.TabularData = {
        dataChannelId: tabularChannelIds,
        dataSet: [
            {
                timeStamp: new Date("2024-01-01T00:00:00Z"),
                value: ["25.5", "1500.0"],
                quality: ["0", "0"],
            },
            {
                timeStamp: new Date("2024-01-01T00:30:00Z"),
                value: ["26.1", "1520.0"],
                quality: ["0", "0"],
            },
            {
                timeStamp: new Date("2024-01-01T01:00:00Z"),
                value: ["25.8", "1480.0"],
                quality: ["0", "0"],
            },
        ],
    };

    // Create event data with alerts (third channel)
    const eventChannelId = await DataChannelId.parseAsync(
        channels[2].dataChannelId.localId.toString(),
    );

    const eventData: TimeSeries.EventData = {
        dataSet: [
            {
                timeStamp: new Date("2024-01-01T00:15:00Z"),
                dataChannelId: eventChannelId,
                value: "LOW_LEVEL_WARNING",
                quality: "0",
            },
            {
                timeStamp: new Date("2024-01-01T00:45:00Z"),
                dataChannelId: eventChannelId,
                value: "NORMAL",
                quality: "0",
            },
        ],
    };

    // Create TimeSeriesData with reference to DataChannelList
    const tsData: TimeSeries.TimeSeriesData = {
        dataConfiguration: {
            id: dcListPackage.package.header.dataChannelListId.id,
            timeStamp: dcListPackage.package.header.dataChannelListId.timestamp,
        },
        tabularData: [tabularData],
        eventData,
    };

    return {
        package: {
            header,
            timeSeriesData: [tsData],
        },
    };
}

// =========================================================================
// Example: JSON to Domain Model Conversion
// =========================================================================
async function exampleJsonToDomain(): Promise<void> {
    console.log("\n" + "=".repeat(60));
    console.log("Example: JSON to Domain Model Conversion");
    console.log("=".repeat(60));

    // Create sample data
    const original = await createSampleDataChannelList();
    const channelCount = original.package.dataChannelList.dataChannel.length;
    console.log(
        `\n1. Created DataChannelListPackage with ${channelCount} channels`,
    );

    // Convert domain model to JSON DTO
    const jsonDto = JSONExtensions.DataChannelList.toJsonDto(original);
    console.log("2. Converted to JSON DTO");

    // Serialize to JSON string
    const jsonStr = JSONSerializer.serializeDataChannelList(jsonDto);
    const byteCount = Buffer.byteLength(jsonStr, "utf-8");
    console.log(`3. Serialized to JSON (${byteCount} bytes)`);
    console.log(
        `   Preview: ${jsonStr.substring(0, Math.min(100, jsonStr.length))}...`,
    );

    // Deserialize JSON string to DTO
    const loadedDto = JSONSerializer.deserializeDataChannelList(jsonStr);
    console.log("4. Deserialized JSON string to DTO");

    // Convert DTO back to domain model
    if (loadedDto) {
        const domainModel =
            await JSONExtensions.DataChannelList.toDomainModel(loadedDto);
        const dcCount = domainModel.package.dataChannelList.dataChannel.length;
        console.log(`5. Converted to domain model with ${dcCount} channels`);
    }

    console.log("\n✓ JSON roundtrip successful!");
}

// =========================================================================
// Example: Get Data Channel by ShortId
// =========================================================================
async function exampleGetByShortId(): Promise<void> {
    console.log("\n" + "=".repeat(60));
    console.log("Example: Get Data Channel by ShortId");
    console.log("=".repeat(60));

    const package_ = await createSampleDataChannelList();
    const dcList = package_.package.dataChannelList.dataChannel;

    // Build a shortId lookup map
    const shortIdMap = new Map<string, DataChannelList.DataChannel>();
    for (const dc of dcList) {
        if (dc.dataChannelId.shortId) {
            shortIdMap.set(dc.dataChannelId.shortId, dc);
        }
    }

    // Look up by ShortId
    const shortIdsToFind = ["TEMP001", "PWR001", "ALT001", "UNKNOWN"];

    for (const shortId of shortIdsToFind) {
        const dataChannel = shortIdMap.get(shortId);
        if (dataChannel) {
            console.log(`\n✓ Found ShortId '${shortId}':`);
            console.log(`  Name: ${dataChannel.property.name}`);
            console.log(`  LocalId: ${dataChannel.dataChannelId.localId}`);
            console.log(`  Type: ${dataChannel.property.dataChannelType.type}`);
            if (dataChannel.property.unit) {
                console.log(`  Unit: ${dataChannel.property.unit.unitSymbol}`);
            }
        } else {
            console.log(`\n✗ ShortId '${shortId}' not found`);
        }
    }
}

// =========================================================================
// Example: Get Data Channel by LocalId
// =========================================================================
async function exampleGetByLocalId(): Promise<void> {
    console.log("\n" + "=".repeat(60));
    console.log("Example: Get Data Channel by LocalId");
    console.log("=".repeat(60));

    const package_ = await createSampleDataChannelList();
    const dcList = package_.package.dataChannelList.dataChannel;

    // Build a localId lookup map
    const localIdMap = new Map<string, DataChannelList.DataChannel>();
    for (const dc of dcList) {
        localIdMap.set(dc.dataChannelId.localId.toString(), dc);
    }

    // Look up by LocalId
    const localIdStrs = [
        "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/meta/qty-temperature/cnt-cooling.air",
        "/dnv-v2/vis-3-4a/511.15-1/E32/meta/qty-power",
        "/dnv-v2/vis-3-4a/999.99/meta/unknown", // Non-existent
    ];

    for (const localIdStr of localIdStrs) {
        const dataChannel = localIdMap.get(localIdStr);
        if (dataChannel) {
            console.log(`\n✓ Found LocalId:`);
            console.log(`  LocalId: ${localIdStr}`);
            console.log(`  ShortId: ${dataChannel.dataChannelId.shortId}`);
            console.log(`  Name: ${dataChannel.property.name}`);
        } else {
            console.log(`\n✗ LocalId not found: ${localIdStr}`);
        }
    }
}

// =========================================================================
// Example: TimeSeriesData JSON to Domain Model
// =========================================================================
async function exampleTimeSeriesJsonToDomain(): Promise<void> {
    console.log("\n" + "=".repeat(60));
    console.log("Example: TimeSeriesData JSON to Domain Model");
    console.log("=".repeat(60));

    // Create sample data
    const dcListPackage = await createSampleDataChannelList();
    const original = await createSampleTimeSeriesData(dcListPackage);

    // Serialize to JSON
    const jsonDto = JSONExtensions.TimeSeries.toJsonDto(original);
    const jsonStr = JSONSerializer.serializeTimeSeriesData(jsonDto);
    const byteCount = Buffer.byteLength(jsonStr, "utf-8");

    console.log(`\n1. Received JSON payload (${byteCount} bytes)`);
    console.log(
        `   Preview: ${jsonStr.substring(0, Math.min(80, jsonStr.length))}...`,
    );

    // Deserialize JSON string to DTO
    const dto = JSONSerializer.deserializeTimeSeriesData(jsonStr);
    console.log("2. Deserialized to JSON DTO");

    // Convert to domain model
    if (dto) {
        const domain = await JSONExtensions.TimeSeries.toDomainModel(dto);
        console.log("3. Converted to domain model");

        // Access structured data
        console.log("\n4. Accessing domain model data:");
        if (domain.package.header) {
            const h = domain.package.header;
            console.log(`   Ship ID: ${h.shipId}`);
            console.log(`   Author: ${h.author}`);
            if (h.timeSpan) {
                console.log(
                    `   Time Span: ${h.timeSpan.start} to ${h.timeSpan.end}`,
                );
            }
        }

        for (let i = 0; i < domain.package.timeSeriesData.length; i++) {
            const tsData = domain.package.timeSeriesData[i];
            console.log(`\n   TimeSeriesData #${i + 1}:`);
            if (tsData.tabularData) {
                for (let j = 0; j < tsData.tabularData.length; j++) {
                    const tab = tsData.tabularData[j];
                    if (!tab.dataChannelId || !tab.dataSet) continue;
                    console.log(
                        `     Tabular #${j + 1}: ${tab.dataChannelId.length} channels`,
                    );
                    console.log(`       ${tab.dataSet.length} data sets`);
                }
            }
            if (tsData.eventData?.dataSet) {
                console.log(
                    `     Events: ${tsData.eventData.dataSet.length} events`,
                );
            }
        }
    }

    console.log("\n✓ JSON to domain conversion successful!");
}

// =========================================================================
// Example: TimeSeriesData JSON Roundtrip
// =========================================================================
async function exampleTimeSeriesJsonRoundtrip(): Promise<void> {
    console.log("\n" + "=".repeat(60));
    console.log("Example: TimeSeriesData JSON Roundtrip");
    console.log("=".repeat(60));

    // Create sample data
    const dcListPackage = await createSampleDataChannelList();
    const original = await createSampleTimeSeriesData(dcListPackage);

    console.log("\n1. Created TimeSeriesDataPackage");
    if (original.package.header) {
        console.log(`   Ship: ${original.package.header.shipId}`);
        if (original.package.header.timeSpan) {
            const ts = original.package.header.timeSpan;
            console.log(
                `   TimeSpan: ${ts.start.toISOString()} - ${ts.end.toISOString()}`,
            );
        }
    }

    // Convert to JSON DTO and serialize
    const jsonDto = JSONExtensions.TimeSeries.toJsonDto(original);
    const jsonStr = JSONSerializer.serializeTimeSeriesData(jsonDto);
    const byteCount = Buffer.byteLength(jsonStr, "utf-8");
    console.log(`2. Serialized to JSON (${byteCount} bytes)`);

    // Deserialize and convert back
    const loadedDto = JSONSerializer.deserializeTimeSeriesData(jsonStr);
    if (loadedDto) {
        const domainModel =
            await JSONExtensions.TimeSeries.toDomainModel(loadedDto);
        console.log("3. Deserialized and converted to domain model");

        // Verify structure
        const origLen = original.package.timeSeriesData.length;
        const loadedLen = domainModel.package.timeSeriesData.length;
        if (origLen !== loadedLen) {
            console.log(
                `   ✗ TimeSeriesData count mismatch: ${origLen} vs ${loadedLen}`,
            );
        } else {
            console.log(`   ✓ TimeSeriesData count matches: ${origLen}`);
        }
    }

    console.log("\n✓ JSON roundtrip successful!");
}

// =========================================================================
// Main
// =========================================================================
async function main() {
    console.log("=".repeat(60));
    console.log("ISO19848 JSON Serialization Examples");
    console.log("=".repeat(60));

    await exampleJsonToDomain();
    await exampleGetByShortId();
    await exampleGetByLocalId();
    await exampleTimeSeriesJsonToDomain();
    await exampleTimeSeriesJsonRoundtrip();

    console.log("\n" + "=".repeat(60));
    console.log("All examples completed successfully!");
    console.log("=".repeat(60));
}

main().catch(console.error);
