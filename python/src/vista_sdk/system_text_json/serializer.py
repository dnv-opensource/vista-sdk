import re
from datetime import datetime, timezone
import json
from typing import Any, Type, TypeVar
from dataclasses import asdict

T = TypeVar('T')

class DateTimeConverter:
    """Converter for handling datetime objects in JSON serialization."""

    ISO8601_STRICT_REGEX = re.compile(
        r'^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[+-]\d{2}:\d{2})$'
    )

    @staticmethod
    def parse(value: str) -> datetime:
        """Parse an ISO 8601 datetime string."""
        if not value:
            raise ValueError("Empty datetime string")

        if not DateTimeConverter.ISO8601_STRICT_REGEX.match(value):
            raise ValueError(f"Invalid ISO 8601-1 format: '{value}'")

        return datetime.fromisoformat(value.replace('Z', '+00:00'))

    @staticmethod
    def format(dt: datetime) -> str:
        """Format a datetime object as ISO 8601 string."""
        return dt.astimezone(timezone.utc).isoformat()

class JsonSerializer:
    """JSON serializer with datetime handling."""

    @staticmethod
    def serialize(obj: Any) -> str:
        """Serialize an object to JSON string."""
        def _default(o: Any) -> Any:
            if isinstance(o, datetime):
                return DateTimeConverter.format(o)
            if hasattr(o, '__dict__'):
                return asdict(o)
            return str(o)

        return json.dumps(obj, default=_default)

    @staticmethod
    def deserialize(json_str: str, cls: Type[T]) -> T:
        """Deserialize a JSON string to an object."""
        def _object_hook(d: dict) -> Any:
            for key, value in d.items():
                if isinstance(value, str) and DateTimeConverter.ISO8601_STRICT_REGEX.match(value):
                    d[key] = DateTimeConverter.parse(value)
            return cls(**d)

        return json.loads(json_str, object_hook=_object_hook)
