import json
from typing import Any, Type, TypeVar
from datetime import datetime

T = TypeVar('T')

def serialize_datetime(dt: datetime) -> str:
    """Serialize datetime to ISO 8601 format"""
    return dt.isoformat()

def deserialize_datetime(dt_str: str) -> datetime:
    """Deserialize datetime from ISO 8601 format"""
    return datetime.fromisoformat(dt_str)

class JsonSerializer:
    """JSON serialization utilities for Vista SDK types"""

    @staticmethod
    def serialize(obj: Any) -> str:
        """Serialize an object to JSON string"""
        return json.dumps(obj, default=lambda o: o.__dict__)

    @staticmethod
    def deserialize(json_str: str, cls: Type[T]) -> T:
        """Deserialize JSON string to specified type"""
        data = json.loads(json_str)
        return cls(**data)
