"""Local ID query builder module."""

from collections.abc import Callable
from typing import overload

from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_path_query import GmodPathQuery, GmodPathQueryBuilder, Nodes, Path
from vista_sdk.local_id import LocalId
from vista_sdk.metadata_tag_query import (
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
)
from vista_sdk.vis import VIS

# Type aliases for query configuration
TagsQueryConfiguration = Callable[[MetadataTagsQueryBuilder], MetadataTagsQuery]


class PathConfig:
    """Wrapper for path configuration callables.

    Use this to wrap a lambda or function that configures a Path builder.

    Example:
        builder.with_primary_item(PathConfig(lambda p: p.without_locations().build()))
    """

    def __init__(self, fn: Callable[[Path], GmodPathQuery]) -> None:
        """Initialize with a path configuration function."""
        self._fn = fn

    def __call__(self, path: Path) -> GmodPathQuery:
        """Execute the configuration function with the given path."""
        return self._fn(path)


class NodesConfig:
    """Wrapper for nodes configuration callables.

    Use this to wrap a lambda or function that configures a Nodes builder.

    Example:
        builder.with_primary_item(NodesConfig(lambda n: n.with_node(...).build()))
    """

    def __init__(self, fn: Callable[[Nodes], GmodPathQuery]) -> None:
        """Initialize with a nodes configuration function."""
        self._fn = fn

    def __call__(self, nodes: Nodes) -> GmodPathQuery:
        """Execute the configuration function with the given nodes."""
        return self._fn(nodes)


class LocalIdQueryBuilder:
    """Builder for creating LocalId queries."""

    def __init__(self) -> None:
        """Initialize a LocalIdQueryBuilder."""
        self._primary_item: GmodPathQuery | None = None
        self._secondary_item: GmodPathQuery | None = None
        self._tags: MetadataTagsQuery | None = None
        self._require_secondary_item: bool | None = None

    def _clone(self) -> "LocalIdQueryBuilder":
        """Create a copy of this builder."""
        new_builder = LocalIdQueryBuilder()
        new_builder._primary_item = self._primary_item
        new_builder._secondary_item = self._secondary_item
        new_builder._tags = self._tags
        new_builder._require_secondary_item = self._require_secondary_item
        return new_builder

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
        else:
            builder = builder.without_secondary_item()

        return builder.with_tags(
            MetadataTagsQueryBuilder.from_local_id(local_id).build()
        )

    # --- Primary Item Overloads ---

    @overload
    def with_primary_item(
        self,
        arg1: NodesConfig,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_primary_item(
        self,
        arg1: PathConfig,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_primary_item(
        self,
        arg1: GmodPath,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_primary_item(
        self,
        arg1: GmodPath,
        configure: PathConfig,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_primary_item(
        self,
        arg1: GmodPathQuery,
    ) -> "LocalIdQueryBuilder": ...

    def with_primary_item(
        self,
        arg1: NodesConfig | PathConfig | GmodPath | GmodPathQuery,
        configure: PathConfig | None = None,
    ) -> "LocalIdQueryBuilder":
        """Configure the primary item for this query."""
        # GmodPathQuery directly
        if isinstance(arg1, GmodPathQuery):
            new_builder = self._clone()
            new_builder._primary_item = arg1
            return new_builder

        # GmodPath with optional configure
        if isinstance(arg1, GmodPath):
            if configure is not None:
                builder = GmodPathQueryBuilder.from_path(arg1)
                return self.with_primary_item(configure(builder))
            return self.with_primary_item(GmodPathQueryBuilder.from_path(arg1).build())

        # PathConfig - requires existing primary item
        if isinstance(arg1, PathConfig):
            if self._primary_item is None:
                raise ValueError("No existing primary item to configure as Path.")
            builder = GmodPathQueryBuilder.from_path(
                self._primary_item.builder.gmod_path  # type: ignore
            )
            return self.with_primary_item(arg1(builder))

        # NodesConfig - build from scratch
        if isinstance(arg1, NodesConfig):
            nodes = GmodPathQueryBuilder.empty()
            query = arg1(nodes)
            return self.with_primary_item(query)

        raise TypeError(f"Unsupported argument type: {type(arg1)}")

    # --- Secondary Item Overloads ---

    @overload
    def with_secondary_item(
        self,
        arg1: NodesConfig,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_secondary_item(
        self,
        arg1: PathConfig,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_secondary_item(
        self,
        arg1: GmodPath,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_secondary_item(
        self,
        arg1: GmodPath,
        configure: PathConfig,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_secondary_item(
        self,
        arg1: GmodPathQuery,
    ) -> "LocalIdQueryBuilder": ...

    def with_secondary_item(
        self,
        arg1: NodesConfig | PathConfig | GmodPath | GmodPathQuery,
        configure: PathConfig | None = None,
    ) -> "LocalIdQueryBuilder":
        """Configure the secondary item for this query."""
        # GmodPathQuery directly
        if isinstance(arg1, GmodPathQuery):
            new_builder = self._clone()
            new_builder._secondary_item = arg1
            new_builder._require_secondary_item = True
            return new_builder

        # GmodPath with optional configure
        if isinstance(arg1, GmodPath):
            if configure is not None:
                builder = GmodPathQueryBuilder.from_path(arg1)
                return self.with_secondary_item(configure(builder))
            return self.with_secondary_item(
                GmodPathQueryBuilder.from_path(arg1).build()
            )

        # PathConfig - requires existing secondary item
        if isinstance(arg1, PathConfig):
            if self._secondary_item is None:
                raise ValueError("No existing secondary item to configure as Path.")
            builder = GmodPathQueryBuilder.from_path(
                self._secondary_item.builder.gmod_path  # type: ignore
            )
            return self.with_secondary_item(arg1(builder))

        # NodesConfig - build from scratch
        if isinstance(arg1, NodesConfig):
            nodes = GmodPathQueryBuilder.empty()
            query = arg1(nodes)
            return self.with_secondary_item(query)

        raise TypeError(f"Unsupported argument type: {type(arg1)}")

    def with_any_secondary_item(self) -> "LocalIdQueryBuilder":
        """Match any LocalId regardless of whether it has a secondary item or not."""
        new_builder = self._clone()
        new_builder._secondary_item = None
        new_builder._require_secondary_item = None
        return new_builder

    def without_secondary_item(self) -> "LocalIdQueryBuilder":
        """Match only LocalIds that do not have a secondary item."""
        new_builder = self._clone()
        new_builder._secondary_item = None
        new_builder._require_secondary_item = False
        return new_builder

    # --- Tags Overloads ---

    @overload
    def with_tags(
        self,
        arg1: TagsQueryConfiguration,
    ) -> "LocalIdQueryBuilder": ...

    @overload
    def with_tags(
        self,
        arg1: MetadataTagsQuery,
    ) -> "LocalIdQueryBuilder": ...

    def with_tags(
        self,
        arg1: TagsQueryConfiguration | MetadataTagsQuery,
    ) -> "LocalIdQueryBuilder":
        """Configure tags for this query."""
        # MetadataTagsQuery directly
        if isinstance(arg1, MetadataTagsQuery):
            new_builder = self._clone()
            new_builder._tags = arg1
            return new_builder

        # TagsQueryConfiguration callable
        if callable(arg1):
            builder = (
                self._tags.builder
                if self._tags is not None
                else MetadataTagsQueryBuilder.empty()
            )
            return self.with_tags(arg1(builder))

        raise TypeError(f"Unsupported argument type: {type(arg1)}")

    def without_locations(self) -> "LocalIdQueryBuilder":
        """Remove locations from both primary and secondary items."""
        builder = self
        if self._primary_item is not None and isinstance(
            self._primary_item.builder, Path
        ):
            builder = builder.with_primary_item(
                PathConfig(lambda p: p.without_locations().build())
            )
        if self._secondary_item is not None and isinstance(
            self._secondary_item.builder, Path
        ):
            builder = builder.with_secondary_item(
                PathConfig(lambda p: p.without_locations().build())
            )
        return builder

    # --- Match Overloads ---

    @overload
    def match(self, other: str) -> bool: ...

    @overload
    def match(self, other: LocalId) -> bool: ...

    def match(self, other: str | LocalId) -> bool:
        """Check if this query matches the provided local ID."""
        if isinstance(other, str):
            return self.match(LocalId.parse(other))

        local_id = other
        if other.vis_version.value < VIS().latest_vis_version.value:
            converted = VIS().convert_local_id(other, VIS().latest_vis_version)
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

        if self._require_secondary_item is not None:
            has_secondary = local_id.secondary_item is not None
            if self._require_secondary_item != has_secondary:
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
