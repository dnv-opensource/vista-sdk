"""Extensions for converting DataChannelList between domain objects and JSON DTOs."""

from __future__ import annotations

from datetime import datetime
from typing import Any, cast

from vista_sdk.local_id import LocalId
from vista_sdk.system_text_json.serializer import _normalize_datetime_string
from vista_sdk.transport.data_channel import data_channel as domain
from vista_sdk.transport.ship_id import ShipId

from . import data_channel_list as dto


def to_json_dto(package: domain.DataChannelListPackage) -> dto.DataChannelListPackage:  # noqa: C901
    """Convert domain DataChannelListPackage to JSON DTO.

    Mirrors C#'s ToJsonDto extension method.
    """
    p = package.package
    h = p.header

    # Convert header
    config_ref = dto.ConfigurationReference(
        ID=h.data_channel_list_id.id,
        TimeStamp=_format_datetime(h.data_channel_list_id.timestamp),
    )
    if h.data_channel_list_id.version is not None:
        config_ref["Version"] = h.data_channel_list_id.version

    header = dto.Header(
        ShipID=str(h.ship_id),
        DataChannelListID=config_ref,
    )
    if h.version_information is not None:
        version_info = dto.VersionInformation(
            NamingRule=h.version_information.naming_rule,
            NamingSchemeVersion=h.version_information.naming_scheme_version,
        )
        if h.version_information.reference_url is not None:
            version_info["ReferenceURL"] = h.version_information.reference_url
        header["VersionInformation"] = version_info
    if h.author is not None:
        header["Author"] = h.author
    if h.date_created is not None:
        header["DateCreated"] = _format_datetime(h.date_created)
    if h.custom_headers is not None:
        header.update(cast(Any, h.custom_headers))

    # Convert data channels
    channels: list[dto.DataChannel] = []
    for c in p.data_channel_list:
        # Convert local ID (use non-verbose mode like C#)
        local_id = c.data_channel_id.local_id
        if local_id.verbose_mode:
            local_id_str = str(local_id.builder.with_verbose_mode(False).build())
        else:
            local_id_str = str(local_id.builder)

        # Build DataChannelID
        data_channel_id = dto.DataChannelID(LocalID=local_id_str)
        if c.data_channel_id.short_id is not None:
            data_channel_id["ShortID"] = c.data_channel_id.short_id
        if c.data_channel_id.name_object is not None:
            name_obj = dto.NameObject(
                NamingRule=c.data_channel_id.name_object.naming_rule
            )
            if c.data_channel_id.name_object.custom_name_objects is not None:
                name_obj.update(
                    cast(Any, c.data_channel_id.name_object.custom_name_objects)
                )
            data_channel_id["NameObject"] = name_obj

        # Build restriction
        restriction = None
        if c.property_.format.restriction is not None:
            r = c.property_.format.restriction
            restriction = dto.Restriction()
            if r.enumeration is not None:
                restriction["Enumeration"] = r.enumeration
            if r.fraction_digits is not None:
                restriction["FractionDigits"] = int(r.fraction_digits)
            if r.length is not None:
                restriction["Length"] = int(r.length)
            if r.max_exclusive is not None:
                restriction["MaxExclusive"] = r.max_exclusive
            if r.max_inclusive is not None:
                restriction["MaxInclusive"] = r.max_inclusive
            if r.max_length is not None:
                restriction["MaxLength"] = int(r.max_length)
            if r.min_exclusive is not None:
                restriction["MinExclusive"] = r.min_exclusive
            if r.min_inclusive is not None:
                restriction["MinInclusive"] = r.min_inclusive
            if r.min_length is not None:
                restriction["MinLength"] = int(r.min_length)
            if r.pattern is not None:
                restriction["Pattern"] = r.pattern
            if r.total_digits is not None:
                restriction["TotalDigits"] = int(r.total_digits)
            if r.white_space is not None:
                # Cast to the expected Literal type
                ws_name = r.white_space.name
                if ws_name in ("Preserve", "Replace", "Collapse"):
                    restriction["WhiteSpace"] = ws_name  # type: ignore[typeddict-item]

        # Build format
        data_format = dto.Format(Type=c.property_.format.type)
        if restriction:
            data_format["Restriction"] = restriction

        # Build data channel type
        data_channel_type = dto.DataChannelType(Type=c.property_.data_channel_type.type)
        if c.property_.data_channel_type.update_cycle is not None:
            data_channel_type["UpdateCycle"] = (
                c.property_.data_channel_type.update_cycle
            )
        if c.property_.data_channel_type.calculation_period is not None:
            data_channel_type["CalculationPeriod"] = (
                c.property_.data_channel_type.calculation_period
            )

        # Build property
        prop = dto.Property(
            DataChannelType=data_channel_type,
            Format=data_format,
        )
        if c.property_.range is not None:
            prop["Range"] = dto.Range(
                High=float(c.property_.range.high), Low=c.property_.range.low
            )
        if c.property_.unit is not None:
            unit = dto.Unit(UnitSymbol=c.property_.unit.unit_symbol)
            if c.property_.unit.quantity_name is not None:
                unit["QuantityName"] = c.property_.unit.quantity_name
            if c.property_.unit.custom_elements is not None:
                unit.update(cast(Any, c.property_.unit.custom_elements))
            prop["Unit"] = unit
        if c.property_.quality_coding is not None:
            prop["QualityCoding"] = c.property_.quality_coding
        if c.property_.alert_priority is not None:
            prop["AlertPriority"] = c.property_.alert_priority
        if c.property_.name is not None:
            prop["Name"] = c.property_.name
        if c.property_.remarks is not None:
            prop["Remarks"] = c.property_.remarks
        if c.property_.custom_properties is not None:
            prop.update(cast(Any, c.property_.custom_properties))

        channel = dto.DataChannel(
            DataChannelID=data_channel_id,
            Property=prop,
        )
        channels.append(channel)

    return dto.DataChannelListPackage(
        Package=dto.Package(
            Header=header,
            DataChannelList=dto.DataChannelList(DataChannel=channels),
        )
    )


def to_domain_model(
    package: dto.DataChannelListPackage,
) -> domain.DataChannelListPackage:
    """Convert JSON DTO DataChannelListPackage to domain model.

    Mirrors C#'s ToDomainModel extension method.
    """
    p = package["Package"]
    h = p["Header"]

    # Convert header
    version_info_dto = h.get("VersionInformation")
    header = domain.Header(
        ship_id=ShipId.parse(h["ShipID"]),
        data_channel_list_id=domain.ConfigurationReference(
            config_id=h["DataChannelListID"]["ID"],
            version=h["DataChannelListID"].get("Version"),
            timestamp=_parse_datetime(h["DataChannelListID"]["TimeStamp"]),
        ),
        version_information=(
            domain.VersionInformation(
                naming_rule=version_info_dto["NamingRule"],
                naming_scheme_version=version_info_dto["NamingSchemeVersion"],
                reference_url=version_info_dto.get("ReferenceURL"),
            )
            if version_info_dto is not None
            else None
        ),
        author=h.get("Author"),
        date_created=_parse_datetime(date_created)
        if (date_created := h.get("DateCreated"))
        else None,
        custom_headers=_extract_custom_headers(h),
    )

    # Convert data channels
    data_channel_list = domain.DataChannelList()

    for c in p["DataChannelList"]["DataChannel"]:
        # Parse local ID
        local_id = LocalId.parse(c["DataChannelID"]["LocalID"])

        # Convert name object
        name_obj = None
        name_obj_dto = c["DataChannelID"].get("NameObject")
        if name_obj_dto is not None:
            name_obj = domain.NameObject(
                naming_rule=name_obj_dto["NamingRule"],
                custom_name_objects=_extract_custom_name_objects(name_obj_dto),
            )

        # Convert restriction
        restriction = None
        restriction_dto = c["Property"]["Format"].get("Restriction")
        if restriction_dto is not None:
            white_space = None
            ws_value = restriction_dto.get("WhiteSpace")
            if ws_value is not None:
                white_space = domain.WhiteSpace[ws_value]
            enumeration_dto = restriction_dto.get("Enumeration")
            restriction = domain.Restriction(
                enumeration=list(enumeration_dto) if enumeration_dto else None,
                fraction_digits=restriction_dto.get("FractionDigits"),
                length=restriction_dto.get("Length"),
                max_exclusive=restriction_dto.get("MaxExclusive"),
                max_inclusive=restriction_dto.get("MaxInclusive"),
                max_length=restriction_dto.get("MaxLength"),
                min_exclusive=restriction_dto.get("MinExclusive"),
                min_inclusive=restriction_dto.get("MinInclusive"),
                min_length=restriction_dto.get("MinLength"),
                pattern=restriction_dto.get("Pattern"),
                total_digits=restriction_dto.get("TotalDigits"),
                white_space=white_space,
            )

        # Convert format
        data_format = domain.Format(
            format_type=c["Property"]["Format"]["Type"],
            restriction=restriction,
        )

        # Convert range
        data_range = None
        range_dto = c["Property"].get("Range")
        if range_dto is not None:
            data_range = domain.Range(
                high=range_dto["High"],
                low=range_dto["Low"],
            )

        # Convert unit
        unit = None
        unit_dto = c["Property"].get("Unit")
        if unit_dto is not None:
            unit = domain.Unit(
                unit_symbol=unit_dto["UnitSymbol"],
                quantity_name=unit_dto.get("QuantityName"),
                custom_elements=_extract_custom_unit_elements(unit_dto),
            )

        # Convert data channel type
        data_channel_type = domain.DataChannelType(
            channel_type=c["Property"]["DataChannelType"]["Type"],
            update_cycle=c["Property"]["DataChannelType"].get("UpdateCycle"),
            calculation_period=c["Property"]["DataChannelType"].get(
                "CalculationPeriod"
            ),
        )

        # Create property
        prop = domain.Property(
            data_channel_type=data_channel_type,
            data_format=data_format,
            data_range=data_range,
            unit=unit,
            quality_coding=c["Property"].get("QualityCoding"),
            alert_priority=c["Property"].get("AlertPriority"),
            name=c["Property"].get("Name"),
            remarks=c["Property"].get("Remarks"),
            custom_properties=_extract_custom_properties(c["Property"]),
        )

        # Create data channel
        data_channel = domain.DataChannel(
            data_channel_id=domain.DataChannelId(
                local_id=local_id,
                short_id=c["DataChannelID"].get("ShortID"),
                name_object=name_obj,
            ),
            property_=prop,
        )
        data_channel_list.add(data_channel)

    # Create package
    domain_package = domain.Package(header=header, data_channel_list=data_channel_list)
    return domain.DataChannelListPackage(package=domain_package)


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


def _extract_custom_headers(h: dto.Header) -> dict[str, Any] | None:
    """Extract custom headers not in standard keys."""
    known_keys = _get_typed_dict_keys(dto.Header)
    custom = {k: v for k, v in h.items() if k not in known_keys}
    return custom if custom else None


def _extract_custom_name_objects(obj: dto.NameObject) -> dict[str, Any] | None:
    """Extract custom name object properties."""
    known_keys = _get_typed_dict_keys(dto.NameObject)
    custom = {k: v for k, v in obj.items() if k not in known_keys}
    return custom if custom else None


def _extract_custom_unit_elements(unit: dto.Unit) -> dict[str, Any] | None:
    """Extract custom unit elements."""
    known_keys = _get_typed_dict_keys(dto.Unit)
    custom = {k: v for k, v in unit.items() if k not in known_keys}
    return custom if custom else None


def _extract_custom_properties(prop: dto.Property) -> dict[str, Any] | None:
    """Extract custom properties."""
    known_keys = _get_typed_dict_keys(dto.Property)
    custom = {k: v for k, v in prop.items() if k not in known_keys}
    return custom if custom else None
