"""Extensions for converting TimeSeriesData between domain objects and JSON DTOs."""

from __future__ import annotations

from datetime import datetime
from typing import Any, cast

from vista_sdk.system_text_json.serializer import _normalize_datetime_string
from vista_sdk.transport.ship_id import ShipId
from vista_sdk.transport.time_series_data import time_series_data as domain
from vista_sdk.transport.time_series_data.data_channel_id import DataChannelId

from . import time_series_data as dto


def to_json_dto(  # noqa: C901
    domain_package: domain.TimeSeriesDataPackage,
) -> dto.TimeSeriesDataPackage:
    """Convert domain TimeSeriesDataPackage to JSON DTO.

    Mirrors C#'s ToJsonDto extension method for TimeSeriesData.
    """
    p = domain_package.package
    h = p.header

    # Convert header
    header = None
    if h is not None:
        header = dto.Header(ShipID=str(h.ship_id))

        if h.time_span is not None:
            header["TimeSpan"] = dto.TimeSpan(
                Start=_format_datetime(h.time_span.start),
                End=_format_datetime(h.time_span.end),
            )
        if h.date_created is not None:
            header["DateCreated"] = _format_datetime(h.date_created)
        if h.date_modified is not None:
            header["DateModified"] = _format_datetime(h.date_modified)
        if h.author is not None:
            header["Author"] = h.author
        if h.system_configuration is not None:
            header["SystemConfiguration"] = [
                dto.ConfigurationReference(
                    ID=r.id, TimeStamp=_format_datetime(r.time_stamp)
                )
                for r in h.system_configuration
            ]
        if h.custom_headers is not None:
            cast(Any, header).update(h.custom_headers)

    # Convert time series data list
    time_series_data_list: list[dto.TimeSeriesData] = []
    for t in p.time_series_data:
        ts_data = dto.TimeSeriesData()

        # Data configuration
        if t.data_configuration is not None:
            ts_data["DataConfiguration"] = dto.ConfigurationReference(
                ID=t.data_configuration.id,
                TimeStamp=_format_datetime(t.data_configuration.time_stamp),
            )

        # Tabular data
        if t.tabular_data is not None:
            tabular_data_list: list[dto.TabularData] = []
            for td in t.tabular_data:
                tabular = dto.TabularData()
                if td.number_of_data_sets is not None:
                    tabular["NumberOfDataSet"] = td.number_of_data_sets
                if td.number_of_data_channels is not None:
                    tabular["NumberOfDataChannel"] = td.number_of_data_channels
                if td.data_channel_ids is not None:
                    tabular["DataChannelID"] = [
                        str(dc_id) for dc_id in td.data_channel_ids
                    ]
                if td.data_sets is not None:
                    data_set_list: list[dto.DataSetTabular] = []
                    for ds in td.data_sets:
                        data_set = dto.DataSetTabular(
                            TimeStamp=_format_datetime(ds.time_stamp),
                            Value=ds.value,
                        )
                        if ds.quality is not None:
                            data_set["Quality"] = ds.quality
                        data_set_list.append(data_set)
                    tabular["DataSet"] = data_set_list
                tabular_data_list.append(tabular)
            ts_data["TabularData"] = tabular_data_list

        # Event data
        if t.event_data is not None and t.event_data.data_set:
            data_set_list_event: list[dto.DataSetEvent] = []
            for ed in t.event_data.data_set:
                event_ds = dto.DataSetEvent(
                    TimeStamp=_format_datetime(ed.time_stamp),
                    DataChannelID=str(ed.data_channel_id),
                    Value=ed.value,
                )
                if ed.quality is not None:
                    event_ds["Quality"] = ed.quality
                data_set_list_event.append(event_ds)
            ts_data["EventData"] = dto.EventData(
                NumberOfDataSet=len(data_set_list_event),
                DataSet=data_set_list_event,
            )

        # Custom data kinds
        if t.custom_data_kinds is not None:
            cast(Any, ts_data).update(t.custom_data_kinds)

        time_series_data_list.append(ts_data)

    # Build package
    package = dto.Package(TimeSeriesData=time_series_data_list)
    if header is not None:
        package["Header"] = header

    return dto.TimeSeriesDataPackage(Package=package)


def to_domain_model(package: dto.TimeSeriesDataPackage) -> domain.TimeSeriesDataPackage:  # noqa: C901
    """Convert JSON DTO TimeSeriesDataPackage to domain model.

    Mirrors C#'s ToDomainModel extension method for TimeSeriesData.
    """
    p = package["Package"]
    h = p.get("Header")

    # Convert header
    header = None
    if h is not None:
        time_span = None
        time_span_dto = h.get("TimeSpan")
        if time_span_dto is not None:
            time_span = domain.TimeSpan(
                start=_parse_datetime(time_span_dto["Start"]),
                end=_parse_datetime(time_span_dto["End"]),
            )

        system_config = None
        sys_config_dto = h.get("SystemConfiguration")
        if sys_config_dto is not None:
            system_config = [
                domain.ConfigurationReference(
                    id_value=r["ID"],
                    time_stamp=_parse_datetime(r["TimeStamp"]),
                )
                for r in sys_config_dto
            ]

        date_created_str = h.get("DateCreated")
        date_modified_str = h.get("DateModified")

        header = domain.Header(
            ship_id=ShipId.parse(h["ShipID"]),
            time_span=time_span,
            author=h.get("Author"),
            date_created=_parse_datetime(date_created_str)
            if date_created_str
            else None,
            date_modified=_parse_datetime(date_modified_str)
            if date_modified_str
            else None,
            system_configuration=system_config,
            custom_headers=_extract_custom_headers(cast(dict[str, Any], h)),
        )

    # Convert time series data
    time_series_data_list = []
    for t in p["TimeSeriesData"]:
        # Data configuration
        data_config = None
        data_config_dto = t.get("DataConfiguration")
        if data_config_dto is not None:
            data_config = domain.ConfigurationReference(
                id_value=data_config_dto["ID"],
                time_stamp=_parse_datetime(data_config_dto["TimeStamp"]),
            )

        # Tabular data
        tabular_data = None
        tabular_data_dto = t.get("TabularData")
        if tabular_data_dto is not None:
            tabular_data = []
            for td in tabular_data_dto:
                # Validate counts match
                expected_channels = td.get("NumberOfDataChannel")
                dc_ids = td.get("DataChannelID")
                actual_channels = len(dc_ids) if dc_ids else 0
                if (
                    expected_channels is not None
                    and expected_channels != actual_channels
                ):
                    raise ValueError(
                        "Number of data channels does not match the expected count"
                    )

                expected_sets = td.get("NumberOfDataSet")
                data_sets_dto = td.get("DataSet")
                actual_sets = len(data_sets_dto) if data_sets_dto else 0
                if expected_sets is not None and expected_sets != actual_sets:
                    raise ValueError(
                        "Number of data sets does not match the expected count"
                    )

                data_channel_ids = None
                if dc_ids is not None:
                    data_channel_ids = [DataChannelId.parse(dc_id) for dc_id in dc_ids]

                data_sets = None
                if data_sets_dto is not None:
                    data_sets = []
                    for ds in data_sets_dto:
                        quality = ds.get("Quality")
                        data_sets.append(
                            domain.TabularDataSet(
                                time_stamp=_parse_datetime(ds["TimeStamp"]),
                                value=list(ds["Value"]),
                                quality=list(quality) if quality else None,
                            )
                        )

                tabular_data.append(
                    domain.TabularData(
                        data_channel_ids=data_channel_ids,
                        data_sets=data_sets,
                    )
                )

        # Event data
        event_data = None
        event_data_dto = t.get("EventData")
        if event_data_dto is not None:
            event_data_set_dto = event_data_dto.get("DataSet")
            if event_data_set_dto:
                data_set = [
                    domain.EventDataSet(
                        time_stamp=_parse_datetime(ed["TimeStamp"]),
                        data_channel_id=DataChannelId.parse(ed["DataChannelID"]),
                        value=ed["Value"],
                        quality=ed.get("Quality"),
                    )
                    for ed in event_data_set_dto
                ]
                event_data = domain.EventData(data_set=data_set)

        time_series_data_list.append(
            domain.TimeSeriesData(
                data_configuration=data_config,
                tabular_data=tabular_data,
                event_data=event_data,
                custom_data_kinds=_extract_custom_data_kinds(cast(dict[str, Any], t)),
            )
        )

    # Create package
    domain_package = domain.Package(
        header=header,
        time_series_data=time_series_data_list,
    )
    return domain.TimeSeriesDataPackage(package=domain_package)


def _format_datetime(dt: datetime | str) -> str:
    """Format datetime to ISO 8601 string with Z suffix for UTC."""
    if isinstance(dt, str):
        return _normalize_datetime_string(dt)
    return _normalize_datetime_string(dt.isoformat())


def _parse_datetime(s: str) -> datetime:
    """Parse ISO 8601 string to datetime.

    Raises:
        ValueError: If the string is not a valid ISO 8601 format.
    """
    # Handle ISO 8601 format with or without timezone
    try:
        return datetime.fromisoformat(s.replace("Z", "+00:00"))
    except ValueError as e:
        raise ValueError(f"Invalid ISO 8601 format: '{s}'") from e


def _get_typed_dict_keys(td: type) -> set[str]:
    """Get the known keys from a TypedDict class."""
    return set(td.__annotations__.keys())


def _extract_custom_headers(h: dict[str, Any]) -> dict[str, Any] | None:
    """Extract custom headers not in standard keys."""
    known_keys = _get_typed_dict_keys(dto.Header)
    custom = {k: v for k, v in h.items() if k not in known_keys}
    return custom if custom else None


def _extract_custom_data_kinds(t: dict[str, Any]) -> dict[str, Any] | None:
    """Extract custom data kinds from time series data."""
    known_keys = _get_typed_dict_keys(dto.TimeSeriesData)
    custom = {k: v for k, v in t.items() if k not in known_keys}
    return custom if custom else None
