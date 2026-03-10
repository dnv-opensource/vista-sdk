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
// Types
// =========================================================================

interface SensorReading {
    systemId: string;
    value: number;
    timestamp: Date;
    quality: string;
}

// =========================================================================
// ISO19848 Filter
// =========================================================================

class ISO19848Filter {
    private readonly dcl: DataChannelList.DataChannelListPackage;
    private readonly shortIdMap: Map<string, DataChannelList.DataChannel>;
    private readonly buffer: Map<string, SensorReading[]> = new Map();

    constructor(dcl: DataChannelList.DataChannelListPackage) {
        this.dcl = dcl;
        // Build a shortId lookup map
        this.shortIdMap = new Map();
        for (const dc of dcl.package.dataChannelList.dataChannel) {
            if (dc.dataChannelId.shortId) {
                this.shortIdMap.set(dc.dataChannelId.shortId, dc);
            }
        }
    }

    receive(reading: SensorReading): boolean {
        if (!this.shortIdMap.has(reading.systemId)) {
            console.log(
                `    ✗ Rejected: ${reading.systemId} (unknown channel)`,
            );
            return false;
        }

        if (!this.buffer.has(reading.systemId)) {
            this.buffer.set(reading.systemId, []);
        }
        this.buffer.get(reading.systemId)!.push(reading);
        console.log(`    ✓ Accepted: ${reading.systemId} = ${reading.value}`);
        return true;
    }

    async flush(
        shipId: ShipId,
    ): Promise<TimeSeries.TimeSeriesDataPackage | undefined> {
        if (this.buffer.size === 0) return undefined;

        // Group channels by update_cycle
        const channelsByCycle = new Map<
            string,
            { dc: DataChannelList.DataChannel; systemId: string }[]
        >();

        for (const systemId of this.buffer.keys()) {
            const dc = this.shortIdMap.get(systemId);
            if (!dc) continue;

            const updateCycle = dc.property.dataChannelType.updateCycle;
            const cycleKey =
                updateCycle !== undefined ? updateCycle.toString() : "none";

            if (!channelsByCycle.has(cycleKey)) {
                channelsByCycle.set(cycleKey, []);
            }
            channelsByCycle.get(cycleKey)!.push({ dc, systemId });
        }

        // Build one TabularData per update_cycle group
        const tabularDataList: TimeSeries.TabularData[] = [];

        for (const [updateCycle, channelPairs] of channelsByCycle) {
            // Collect all unique timestamps
            const allTimestamps = new Set<number>();
            for (const { systemId } of channelPairs) {
                for (const reading of this.buffer.get(systemId)!) {
                    allTimestamps.add(reading.timestamp.getTime());
                }
            }
            const sortedTimestamps = [...allTimestamps].sort();

            // Build data channel IDs using parseAsync
            const dataChannelIds: DataChannelId[] = [];
            for (const { dc } of channelPairs) {
                dataChannelIds.push(
                    await DataChannelId.parseAsync(
                        dc.dataChannelId.localId.toString(),
                    ),
                );
            }

            // Build data sets
            const dataSets: TimeSeries.TabularDataSet[] = [];
            for (const ts of sortedTimestamps) {
                const values: string[] = [];
                const qualities: string[] = [];

                for (const { systemId } of channelPairs) {
                    const reading = this.buffer
                        .get(systemId)!
                        .find((r) => r.timestamp.getTime() === ts);
                    if (reading) {
                        values.push(reading.value.toString());
                        qualities.push(reading.quality);
                    } else {
                        values.push("");
                        qualities.push("Bad");
                    }
                }

                dataSets.push({
                    timeStamp: new Date(ts),
                    value: values,
                    quality: qualities,
                });
            }

            const cycleStr =
                updateCycle === "none" ? "None" : `${updateCycle}s`;
            console.log(
                `    TabularData: ${channelPairs.length} channels, ${dataSets.length} rows (update_cycle=${cycleStr})`,
            );

            tabularDataList.push({
                dataChannelId: dataChannelIds,
                dataSet: dataSets,
            });
        }

        // Build the package
        const allTimestampsList: Date[] = [];
        for (const readings of this.buffer.values()) {
            for (const r of readings) {
                allTimestampsList.push(r.timestamp);
            }
        }

        const minTime = new Date(
            Math.min(...allTimestampsList.map((d) => d.getTime())),
        );
        const maxTime = new Date(
            Math.max(...allTimestampsList.map((d) => d.getTime())),
        );

        const pkg: TimeSeries.TimeSeriesDataPackage = {
            package: {
                header: {
                    shipId,
                    timeSpan: { start: minTime, end: maxTime },
                    author: "ISO19848Filter",
                },
                timeSeriesData: [
                    {
                        dataConfiguration: {
                            id: this.dcl.package.header.dataChannelListId.id,
                            timeStamp:
                                this.dcl.package.header.dataChannelListId
                                    .timestamp,
                        },
                        tabularData: tabularDataList,
                    },
                ],
            },
        };

        this.buffer.clear();
        return pkg;
    }
}

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
// Main
// =========================================================================
async function main() {
    console.log("=".repeat(60));
    console.log("Sensor Data Flow: Proprietary -> ISO19848");
    console.log("=".repeat(60));

    // Step 1: Load DataChannelList
    console.log("\n[1] Loading DataChannelList...");
    const dcl = await createSampleDataChannelList();
    console.log(
        `    Loaded ${dcl.package.dataChannelList.dataChannel.length} channels:`,
    );
    for (const dc of dcl.package.dataChannelList.dataChannel) {
        const cycle = dc.property.dataChannelType.updateCycle;
        const cycleStr = cycle !== undefined ? `${cycle}s` : "None";
        console.log(
            `      ${dc.dataChannelId.shortId} (update_cycle=${cycleStr})`,
        );
    }

    // Step 2: Create the filter
    console.log("\n[2] Creating ISO19848 filter...");
    const isoFilter = new ISO19848Filter(dcl);
    console.log("    Filter ready");
    console.log("    (Channels with same update_cycle will be grouped)");

    // Step 3: Receive sensor readings
    console.log("\n[3] Receiving sensor readings...");
    const baseTime = new Date("2024-06-15T10:00:00Z");
    const fiveMin = 5 * 60 * 1000;

    const readings: SensorReading[] = [
        // Temperature readings (1.0s update cycle)
        {
            systemId: "TEMP001",
            value: 45.2,
            timestamp: baseTime,
            quality: "Good",
        },
        {
            systemId: "TEMP001",
            value: 46.1,
            timestamp: new Date(baseTime.getTime() + fiveMin),
            quality: "Good",
        },
        // Power readings (same 1.0s update cycle - will be grouped!)
        {
            systemId: "PWR001",
            value: 2500.0,
            timestamp: baseTime,
            quality: "Good",
        },
        {
            systemId: "PWR001",
            value: 2650.5,
            timestamp: new Date(baseTime.getTime() + fiveMin),
            quality: "Good",
        },
        // Alert channel (different update cycle)
        {
            systemId: "ALT001",
            value: 1.0,
            timestamp: baseTime,
            quality: "Good",
        },
        // Unknown sensor (will be rejected)
        {
            systemId: "UNKNOWN",
            value: 123.4,
            timestamp: baseTime,
            quality: "Good",
        },
    ];

    for (const reading of readings) {
        isoFilter.receive(reading);
    }

    // Step 4: Flush to TimeSeriesDataPackage
    console.log(
        "\n[4] Generating TimeSeriesDataPackage (grouped by update_cycle)...",
    );
    const shipId = ShipId.parse("IMO1234567");
    const pkg = await isoFilter.flush(shipId);

    if (pkg) {
        console.log("    ✓ Package created");
        if (pkg.package.header) {
            const h = pkg.package.header;
            console.log(`    Ship: ${h.shipId}`);
            if (h.timeSpan) {
                console.log(
                    `    TimeSpan: ${h.timeSpan.start.toISOString()} - ${h.timeSpan.end.toISOString()}`,
                );
            }
        }
    }

    // Step 5: Serialize to JSON
    console.log("\n[5] Serializing to JSON...");
    if (pkg) {
        const jsonDto = JSONExtensions.TimeSeries.toJsonDto(pkg);
        const jsonStr = JSONSerializer.serializeTimeSeriesData(jsonDto);
        const byteCount = Buffer.byteLength(jsonStr, "utf-8");
        console.log(`    ✓ ${byteCount} bytes`);
        console.log(
            `    Preview: ${jsonStr.substring(0, Math.min(80, jsonStr.length))}...`,
        );
    }

    console.log("\n" + "=".repeat(60));
    console.log("Done! Data is now ISO19848-compliant.");
    console.log("=".repeat(60));
}

main().catch(console.error);
