"""Local ID query builder module."""

from collections.abc import Callable

from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_path_query import GmodPathQuery, GmodPathQueryBuilder, Path
from vista_sdk.local_id import LocalId
from vista_sdk.metadata_tag_query_builder import (
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
)
from vista_sdk.vis import VIS

# Type aliases for query configuration
PathQueryConfiguration = Callable[[GmodPathQueryBuilder], GmodPathQuery]
NodesQueryConfiguration = Callable[[GmodPathQueryBuilder], GmodPathQuery]


class LocalIdQueryBuilder:
    """Builder for creating LocalId queries."""

    def __init__(self) -> None:
        """Initialize a LocalIdQueryBuilder."""
        self._primary_item: GmodPathQuery | None = None
        self._secondary_item: GmodPathQuery | None = None
        self._tags: MetadataTagsQuery | None = None

    def build(self) -> "LocalIdQuery":
        """Build the query."""
        return LocalIdQuery(self)

    @staticmethod
    def empty() -> "LocalIdQueryBuilder":
        """Create an empty builder."""
        return LocalIdQueryBuilder()

    @property
    def primary_item(self) -> GmodPath | None:
        """Get the primary item path if available."""
        if self._primary_item is not None and isinstance(
            self._primary_item.builder, Path
        ):
            return self._primary_item.builder.gmod_path
        return None

    @property
    def secondary_item(self) -> GmodPath | None:
        """Get the secondary item path if available."""
        if self._secondary_item is not None and isinstance(
            self._secondary_item.builder, Path
        ):
            return self._secondary_item.builder.gmod_path
        return None

    @staticmethod
    def from_string(local_id_str: str) -> "LocalIdQueryBuilder":
        """Create a builder from a local ID string."""
        return LocalIdQueryBuilder.from_local_id(LocalId.parse(local_id_str))

    @staticmethod
    def from_local_id(local_id: LocalId) -> "LocalIdQueryBuilder":
        """Create a builder from a LocalId object."""
        builder = LocalIdQueryBuilder().with_primary_item(
            GmodPathQueryBuilder.from_path(local_id.primary_item).build()
        )

        if local_id.secondary_item is not None:
            builder = builder.with_secondary_item(
                GmodPathQueryBuilder.from_path(local_id.secondary_item).build()
            )

        return builder.with_tags(
            MetadataTagsQueryBuilder.from_local_id(local_id).build()
        )

    def with_primary_item(
        self,
        arg1: NodesQueryConfiguration
        | GmodPath
        | GmodPathQuery
        | PathQueryConfiguration,
        configure: PathQueryConfiguration | None = None,
    ) -> "LocalIdQueryBuilder":
        """Configure the primary item for this query."""
        # Case 1: arg1 is a callable (could be either configuration type)
        if callable(arg1):
            # If self.primary_item is None, assume it's a NodesQueryConfiguration
            if self.primary_item is None:
                # NodesQueryConfiguration - build from empty nodes
                try:
                    nodes = GmodPathQueryBuilder.empty()
                    query = arg1(nodes)
                    return self.with_primary_item(query)
                except Exception as e:
                    # If that fails, it's probably not a NodesQueryConfiguration
                    raise ValueError(
                        f"Failed to configure primary item with callable: {e}"
                    ) from e

            # Try as PathQueryConfiguration with existing primary item
            if self.primary_item is not None:
                return self.with_primary_item(self.primary_item, arg1)

            raise ValueError(
                "Cannot determine configuration type or primary item is null"
            )

        # Case 2: GmodPath with configure callable
        if isinstance(arg1, GmodPath) and configure is not None:
            builder = GmodPathQueryBuilder.from_path(arg1)
            return self.with_primary_item(configure(builder))

        # Case 3: GmodPath alone
        if isinstance(arg1, GmodPath):
            return self.with_primary_item(GmodPathQueryBuilder.from_path(arg1).build())

        # Case 4: GmodPathQuery directly
        if isinstance(arg1, GmodPathQuery):
            new_builder = LocalIdQueryBuilder()
            new_builder._primary_item = arg1
            new_builder._secondary_item = self._secondary_item
            new_builder._tags = self._tags
            return new_builder

        raise TypeError(f"Unsupported argument type: {type(arg1)}")

    def with_secondary_item(
        self,
        arg1: NodesQueryConfiguration
        | GmodPath
        | GmodPathQuery
        | PathQueryConfiguration,
        configure: PathQueryConfiguration | None = None,
    ) -> "LocalIdQueryBuilder":
        """Configure the secondary item for this query."""
        # Handle different overloads
        if callable(arg1):
            # NodesQueryConfiguration overload
            if isinstance(arg1, type(lambda: None)):
                return self.with_secondary_item(arg1(GmodPathQueryBuilder.empty()))

            # PathQueryConfiguration overload
            if self.secondary_item is None:
                raise ValueError("Secondary item is null")
            return self.with_secondary_item(self.secondary_item, arg1)

        # GmodPath with configure overload
        if isinstance(arg1, GmodPath) and configure is not None:
            builder = GmodPathQueryBuilder.from_path(arg1)
            return self.with_secondary_item(configure(builder))

        # GmodPath overload
        if isinstance(arg1, GmodPath):
            return self.with_secondary_item(
                GmodPathQueryBuilder.from_path(arg1).build()
            )

        # GmodPathQuery overload
        if isinstance(arg1, GmodPathQuery):
            new_builder = LocalIdQueryBuilder()
            new_builder._primary_item = self._primary_item
            new_builder._secondary_item = arg1
            new_builder._tags = self._tags
            return new_builder

        raise TypeError("Unsupported argument type")

    def with_tags(
        self,
        arg: Callable[[MetadataTagsQueryBuilder], MetadataTagsQuery]
        | MetadataTagsQuery,
    ) -> "LocalIdQueryBuilder":
        """Configure tags for this query."""
        # Function configuration overload
        if callable(arg):
            builder = (
                self._tags.builder
                if self._tags is not None
                else MetadataTagsQueryBuilder.empty()
            )
            return self.with_tags(arg(builder))

        # Direct MetadataTagsQuery overload
        if isinstance(arg, MetadataTagsQuery):
            new_builder = LocalIdQueryBuilder()
            new_builder._primary_item = self._primary_item
            new_builder._secondary_item = self._secondary_item
            new_builder._tags = arg
            return new_builder

        raise TypeError("Unsupported argument type")

    def match(self, other: str | LocalId) -> bool:
        """Check if this query matches the provided local ID."""
        if isinstance(other, str):
            return self.match(LocalId.parse(other))

        local_id = other
        if other.vis_version.value < VIS().LatestVisVersion.value:
            converted = VIS().convert_local_id(other, VIS().LatestVisVersion)
            if converted is None:
                raise Exception("Failed to convert local id")
            local_id = converted

        if self._primary_item is not None and not self._primary_item.match(
            local_id.primary_item
        ):
            return False

        if self._secondary_item is not None and not self._secondary_item.match(
            local_id.secondary_item
        ):
            return False

        return self._tags is None or self._tags.match(local_id)


class LocalIdQuery:
    """Query for matching LocalId objects based on specified criteria."""

    def __init__(self, builder: LocalIdQueryBuilder) -> None:
        """Initialize with a builder."""
        self.builder = builder

    def match(self, other: str | LocalId) -> bool:
        """Check if this query matches the provided local ID."""
        return self.builder.match(other)


class PathItem:
    """Internal class for representing a path item."""

    def __init__(self, path: GmodPath, individualized: bool) -> None:
        """Initialize a PathItem."""
        self.path = path
        self.individualized = individualized
