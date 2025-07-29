"""Implementation of JSON serialization utilities for Vista SDK types."""

import json
from datetime import datetime
from typing import Any, TypeVar

T = TypeVar("T")


def serialize_datetime(dt: datetime) -> str:
    """Serialize datetime to ISO 8601 format."""
    return dt.isoformat()


def deserialize_datetime(dt_str: str) -> datetime:
    """Deserialize datetime from ISO 8601 format."""
    return datetime.fromisoformat(dt_str)


class JsonSerializer:
    """JSON serialization utilities for Vista SDK types."""

    @staticmethod
    def serialize(obj: Any) -> str:  # noqa : ANN401
        """Serialize an object to JSON string."""
        return json.dumps(obj, default=lambda o: o.__dict__)

    @staticmethod
    def deserialize(json_str: str, cls: type[T]) -> T:
        """Deserialize JSON string to specified type."""
        data = json.loads(json_str)
        return cls(**data)
