"""# Local ID query module."""

from vista_sdk.local_id import LocalId
from vista_sdk.local_id_query_builder import LocalIdQueryBuilder


class LocalIdQuery:
    """A query for a local ID."""

    def __init__(self, builder: LocalIdQueryBuilder) -> None:
        """Initialize with a builder."""
        self._builder = builder

    def match(self, other: str | LocalId) -> bool:
        """Check if this query matches the provided local ID."""
        return self._builder.match(other)
