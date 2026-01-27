"""JSON serialization for Vista SDK transport types.

Mirrors C#'s Vista.SDK.Transport.Json.Serializer.
Uses TypedDict DTOs - json.loads() returns dicts that match TypedDict types directly.
"""

from __future__ import annotations

import json
import re
from datetime import datetime
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    from vista_sdk.system_text_json.data_channel_list import DataChannelListPackage
    from vista_sdk.system_text_json.time_series_data import TimeSeriesDataPackage

# ISO 8601 datetime pattern for detection
_ISO_DATETIME_PATTERN = re.compile(
    r"^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:\d{2})$"
)


def _normalize_datetime_string(s: str) -> str:
    """Normalize datetime string to use Z suffix for UTC."""
    return s.replace("+00:00", "Z")


def _normalize_datetimes_hook(obj: dict[str, Any]) -> dict[str, Any]:
    """Object hook for json.loads to normalize datetime strings.

    Recursively processes dicts and normalizes ISO 8601 datetime strings
    to use 'Z' suffix for UTC instead of '+00:00'.
    """
    for key, value in obj.items():
        if isinstance(value, str) and _ISO_DATETIME_PATTERN.match(value):
            obj[key] = _normalize_datetime_string(value)
    return obj


class DateTimeEncoder(json.JSONEncoder):
    """Custom JSON encoder that handles datetime objects.

    Converts datetime to ISO 8601 format with 'Z' suffix for UTC.
    """

    def default(self, obj: Any) -> Any:  # noqa: ANN401
        """Encode datetime objects to ISO 8601 strings."""
        if isinstance(obj, datetime):
            # Use Z suffix for UTC, otherwise use offset
            iso = obj.isoformat()
            return _normalize_datetime_string(iso)
        return super().default(obj)


class Serializer:
    """JSON serialization for Vista SDK transport types.

    Handles JSON parsing and stringifying only.
    Automatically normalizes datetime strings to use 'Z' suffix for UTC.

    Example:
        >>> from vista_sdk.system_text_json import Serializer
        >>> json_str = Serializer.serialize(dto)
        >>> dto = Serializer.deserialize_data_channel_list(json_str)
    """

    @staticmethod
    def serialize(obj: Any, *, use_datetime_encoder: bool = True) -> str:  # noqa: ANN401
        """Serialize an object to JSON string.

        Args:
            obj: The object to serialize.
            use_datetime_encoder: If True, uses DateTimeEncoder to handle
                datetime objects. Defaults to True.
        """
        if use_datetime_encoder:
            return json.dumps(obj, cls=DateTimeEncoder)
        return json.dumps(obj)

    @staticmethod
    def deserialize_data_channel_list(
        json_str: str, *, normalize_datetimes: bool = True
    ) -> DataChannelListPackage:
        """Deserialize JSON string to DataChannelListPackage.

        Args:
            json_str: The JSON string to deserialize.
            normalize_datetimes: If True, normalizes datetime strings to use
                'Z' suffix for UTC. Defaults to True.
        """
        if normalize_datetimes:
            return json.loads(json_str, object_hook=_normalize_datetimes_hook)  # type: ignore[return-value]
        return json.loads(json_str)  # type: ignore[return-value]

    @staticmethod
    def deserialize_time_series_data(
        json_str: str, *, normalize_datetimes: bool = True
    ) -> TimeSeriesDataPackage:
        """Deserialize JSON string to TimeSeriesDataPackage.

        Args:
            json_str: The JSON string to deserialize.
            normalize_datetimes: If True, normalizes datetime strings to use
                'Z' suffix for UTC. Defaults to True.
        """
        if normalize_datetimes:
            return json.loads(json_str, object_hook=_normalize_datetimes_hook)  # type: ignore[return-value]
        return json.loads(json_str)  # type: ignore[return-value]
