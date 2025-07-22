"""Data channel ID for time series data."""

from __future__ import annotations

from collections.abc import Callable
from typing import TypeVar

from vista_sdk.local_id import LocalId

T = TypeVar("T")


class DataChannelId:
    """Data channel identifier that can be either a LocalId or a short ID string."""

    def __init__(
        self, local_id: LocalId | None = None, short_id: str | None = None
    ) -> None:
        """Initialize DataChannelId with either LocalId or short ID."""
        if local_id is not None and short_id is not None:
            raise ValueError("DataChannelId cannot have both local_id and short_id")
        if local_id is None and short_id is None:
            raise ValueError("DataChannelId must have either local_id or short_id")

        self._tag = 1 if local_id is not None else 2
        self._local_id = local_id
        self._short_id = short_id

    @property
    def is_local_id(self) -> bool:
        """Check if this is a LocalId."""
        return self._tag == 1

    @property
    def is_short_id(self) -> bool:
        """Check if this is a short ID."""
        return self._tag == 2

    @property
    def local_id(self) -> LocalId | None:
        """Get the LocalId if this is a LocalId type."""
        return self._local_id if self._tag == 1 else None

    @property
    def short_id(self) -> str | None:
        """Get the short ID if this is a short ID type."""
        return self._short_id if self._tag == 2 else None

    def match(
        self, on_local_id: Callable[[LocalId], T], on_short_id: Callable[[str], T]
    ) -> T:
        """Pattern match on the data channel ID type."""
        if self._tag == 1 and self._local_id:
            return on_local_id(self._local_id)
        if self._tag == 2 and self._short_id:
            return on_short_id(self._short_id)
        raise RuntimeError("Tried to match on invalid DataChannelId")

    def switch(
        self,
        on_local_id: Callable[[LocalId], None],
        on_short_id: Callable[[str], None],
    ) -> None:
        """Switch on the data channel ID type."""
        if self._tag == 1 and self._local_id:
            on_local_id(self._local_id)
            return
        if self._tag == 2 and self._short_id:
            on_short_id(self._short_id)
            return
        raise RuntimeError("Tried to switch on invalid DataChannelId")

    def __str__(self) -> str:
        """Return string representation."""
        if self._tag == 1 and self._local_id:
            return str(self._local_id)
        if self._tag == 2 and self._short_id:
            return self._short_id
        raise RuntimeError("Invalid state exception")

    @classmethod
    def parse(cls, value: str) -> DataChannelId:
        """Parse a string into a DataChannelId."""
        if value is None:
            raise ValueError("value cannot be None")

        # Try to parse as LocalId first
        try:
            local_id = LocalId.parse(value)
            return cls(local_id=local_id)
        except (ValueError, AttributeError):
            # If LocalId parsing fails, treat as short ID
            return cls(short_id=value)

    @classmethod
    def from_local_id(cls, local_id: LocalId) -> DataChannelId:
        """Create DataChannelId from LocalId."""
        return cls(local_id=local_id)

    @classmethod
    def from_short_id(cls, short_id: str) -> DataChannelId:
        """Create DataChannelId from short ID."""
        return cls(short_id=short_id)

    def __eq__(self, other: object) -> bool:
        """Check equality with another DataChannelId."""
        if not isinstance(other, DataChannelId):
            return False
        return (
            self._tag == other._tag
            and self._local_id == other._local_id
            and self._short_id == other._short_id
        )

    def __hash__(self) -> int:
        """Return hash of the DataChannelId."""
        if self._tag == 1:
            return hash(("local_id", self._local_id))
        return hash(("short_id", self._short_id))
