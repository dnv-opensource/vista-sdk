"""Sensor Data Flow Example: Proprietary -> ISO19848.

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
"""

from collections import defaultdict
from dataclasses import dataclass, field
from datetime import datetime, timezone

from samples.iso19848_json import create_sample_data_channel_list

from vista_sdk.imo_number import ImoNumber
from vista_sdk.system_text_json import JsonExtensions, Serializer
from vista_sdk.transport.data_channel.data_channel import (
    DataChannel,
    DataChannelListPackage,
)
from vista_sdk.transport.ship_id import ShipId
from vista_sdk.transport.time_series_data import (
    DataChannelId,
    TabularData,
    TimeSeriesData,
    TimeSeriesDataPackage,
)
from vista_sdk.transport.time_series_data import Header as TimeSeriesHeader
from vista_sdk.transport.time_series_data import Package as TimeSeriesPackage
from vista_sdk.transport.time_series_data.time_series_data import (
    TabularDataSet,
    TimeSpan,
)


@dataclass
class SensorReading:
    """A proprietary sensor reading with system ID."""

    system_id: str  # The short_id used internally
    value: float
    timestamp: datetime
    quality: str = "Good"


@dataclass
class ISO19848Filter:
    """Filter that transforms proprietary sensor data to ISO19848 format.

    Uses the DataChannelList as the source of truth for:
    - Valid channel IDs (short_id)
    - Mapping from short_id to LocalId
    - Update cycle for grouping channels (ISO19848 optimization)

    Any reading with an unknown system_id is rejected.
    Channels with the same update_cycle are grouped into one TabularData.
    """

    data_channel_list_package: DataChannelListPackage
    _buffer: dict[str, list[SensorReading]] = field(default_factory=dict)

    def _get_update_cycle(self, dc: DataChannel) -> float | None:
        """Get the update cycle for a data channel."""
        return dc.property_.data_channel_type.update_cycle

    def receive(self, reading: SensorReading) -> bool:
        """Receive a sensor reading. Returns True if accepted."""
        result, _ = (
            self.data_channel_list_package.data_channel_list.try_get_by_short_id(
                reading.system_id
            )
        )
        if not result:
            print(f"    ✗ Rejected: {reading.system_id} (unknown channel)")
            return False

        if reading.system_id not in self._buffer:
            self._buffer[reading.system_id] = []

        self._buffer[reading.system_id].append(reading)
        print(f"    ✓ Accepted: {reading.system_id} = {reading.value}")
        return True

    def flush(self, ship_id: ShipId) -> TimeSeriesDataPackage | None:
        """Flush buffered readings to a TimeSeriesDataPackage.

        Groups channels by update_cycle into TabularData blocks.
        This is recommended by ISO19848 to reduce payload size.
        """
        if not self._buffer:
            return None

        # Reference the specific DataChannelList version (recommended)
        configuration = self.data_channel_list_package.package.header.data_channel_list_id.as_time_series_reference()  # noqa: E501

        # Group channels by update_cycle
        channels_by_cycle: dict[float | None, list[tuple[DataChannel, str]]] = (
            defaultdict(list)
        )

        for system_id in self._buffer:
            result, dc = (
                self.data_channel_list_package.data_channel_list.try_get_by_short_id(
                    system_id
                )
            )
            if result and dc is not None:
                update_cycle = self._get_update_cycle(dc)
                channels_by_cycle[update_cycle].append((dc, system_id))

        # Build one TabularData per update_cycle group
        tabular_data_list: list[TabularData] = []

        for update_cycle, channel_pairs in channels_by_cycle.items():
            # Collect all unique timestamps across channels in this group
            all_timestamps: set[datetime] = set()
            for _, system_id in channel_pairs:
                for buf_reading in self._buffer[system_id]:
                    all_timestamps.add(buf_reading.timestamp)

            sorted_timestamps = sorted(all_timestamps)

            # Build data channel IDs list for this group
            data_channel_ids = [
                DataChannelId.from_local_id(dc.data_channel_id.local_id)
                for dc, _ in channel_pairs
            ]

            # Build data sets - one per timestamp, with values for all channels
            data_sets: list[TabularDataSet] = []
            for ts in sorted_timestamps:
                values: list[str] = []
                qualities: list[str] = []

                for _, system_id in channel_pairs:
                    # Find reading for this channel at this timestamp
                    reading = next(
                        (r for r in self._buffer[system_id] if r.timestamp == ts),
                        None,
                    )
                    if reading:
                        values.append(str(reading.value))
                        qualities.append(reading.quality)
                    else:
                        # No reading for this channel at this timestamp
                        values.append("")
                        qualities.append("Bad")

                data_sets.append(
                    TabularDataSet(time_stamp=ts, value=values, quality=qualities)
                )

            cycle_str = f"{update_cycle}s" if update_cycle else "None"
            print(
                f"    TabularData: {len(channel_pairs)} channels, "
                f"{len(data_sets)} rows (update_cycle={cycle_str})"
            )

            tabular_data_list.append(
                TabularData(data_channel_ids=data_channel_ids, data_sets=data_sets)
            )

        # Build the package
        all_timestamps_list = [
            r.timestamp for readings in self._buffer.values() for r in readings
        ]

        package = TimeSeriesPackage(
            header=TimeSeriesHeader(
                ship_id=ship_id,
                time_span=TimeSpan(
                    start=min(all_timestamps_list),
                    end=max(all_timestamps_list),
                ),
                author="ISO19848Filter",
            ),
            time_series_data=[
                TimeSeriesData(
                    data_configuration=configuration,
                    tabular_data=tabular_data_list,
                    event_data=None,
                )
            ],
        )

        self._buffer.clear()
        return TimeSeriesDataPackage(package)


def main() -> None:
    """Demonstrate sensor data flowing through an ISO19848 filter."""
    print("=" * 60)
    print("Sensor Data Flow: Proprietary -> ISO19848")
    print("=" * 60)

    # Step 1: Load DataChannelList (defines valid channels + mappings)
    print("\n[1] Loading DataChannelList...")
    dcl = create_sample_data_channel_list()
    print(f"    Loaded {len(dcl.data_channel_list)} channels:")
    for dc in dcl.data_channel_list:
        cycle = dc.property_.data_channel_type.update_cycle
        cycle_str = f"{cycle}s" if cycle else "None"
        print(f"      {dc.data_channel_id.short_id} (update_cycle={cycle_str})")

    # Step 2: Create the filter
    print("\n[2] Creating ISO19848 filter...")
    iso_filter = ISO19848Filter(dcl)
    print("    Filter ready")
    print("    (Channels with same update_cycle will be grouped)")

    # Step 3: Receive sensor readings
    print("\n[3] Receiving sensor readings...")
    base_time = datetime(2024, 6, 15, 10, 0, 0, tzinfo=timezone.utc)

    # Simulate readings from multiple sensors at same timestamps
    readings = [
        # Temperature readings (1.0s update cycle)
        SensorReading("TEMP001", 45.2, base_time),
        SensorReading("TEMP001", 46.1, base_time.replace(minute=5)),
        # Power readings (same 1.0s update cycle - will be grouped!)
        SensorReading("PWR001", 2500.0, base_time),
        SensorReading("PWR001", 2650.5, base_time.replace(minute=5)),
        # Alert channel (different update cycle)
        SensorReading("ALT001", 1.0, base_time),
        # Unknown sensor (will be rejected)
        SensorReading("UNKNOWN", 123.4, base_time),
    ]

    for reading in readings:
        iso_filter.receive(reading)

    # Step 4: Flush to TimeSeriesDataPackage
    print("\n[4] Generating TimeSeriesDataPackage (grouped by update_cycle)...")
    ship_id = ShipId(ImoNumber(1234567))
    package = iso_filter.flush(ship_id)

    if package:
        print("    ✓ Package created")
        if package.package.header:
            h = package.package.header
            print(f"    Ship: {h.ship_id}")
            if h.time_span:
                print(f"    TimeSpan: {h.time_span.start} - {h.time_span.end}")

    # Step 5: Serialize to JSON
    print("\n[5] Serializing to JSON...")
    if package:
        json_dto = JsonExtensions.TimeSeriesData.to_json_dto(package)
        json_str = Serializer.serialize(json_dto)
        byte_count = len(json_str.encode("utf-8"))
        print(f"    ✓ {byte_count} bytes")
        print(f"    Preview: {json_str[:80]}...")

    print("\n" + "=" * 60)
    print("Done! Data is now ISO19848-compliant.")
    print("=" * 60)


if __name__ == "__main__":
    main()
