"""Metadata tags query module."""

from vista_sdk.codebook import CodebookName
from vista_sdk.local_id import LocalId
from vista_sdk.metadata_tag import MetadataTag


class MetadataTagsQuery:
    """Query for matching metadata tags."""

    def __init__(self, builder: "MetadataTagsQueryBuilder") -> None:
        """Initialize with a builder."""
        self.builder = builder

    def match(self, local_id: LocalId | None) -> bool:
        """Check if this query matches the provided local ID's metadata tags."""
        return self.builder.match(local_id)


class MetadataTagsQueryBuilder:
    """Builder for creating metadata tags queries."""

    def __init__(self) -> None:
        """Initialize a MetadataTagsQueryBuilder."""
        self._tags: dict[CodebookName, MetadataTag] = {}
        self._match_exact: bool = False

    @staticmethod
    def empty() -> "MetadataTagsQueryBuilder":
        """Create an empty builder."""
        return MetadataTagsQueryBuilder()

    @staticmethod
    def from_local_id(
        local_id: LocalId, allow_other_tags: bool = True
    ) -> "MetadataTagsQueryBuilder":
        """Create a builder from a LocalId object."""
        builder = MetadataTagsQueryBuilder()
        for tag in local_id.metadata_tags:
            builder = builder.with_tag(tag)

        return builder.with_allow_other_tags(allow_other_tags)

    def build(self) -> MetadataTagsQuery:
        """Build the query."""
        return MetadataTagsQuery(self)

    def with_tag(
        self, name_or_tag: CodebookName | MetadataTag, value: str | None = None
    ) -> "MetadataTagsQueryBuilder":
        """Add a tag to the query."""
        new_builder = MetadataTagsQueryBuilder()
        # Copy existing tags
        new_builder._tags = dict(self._tags)
        new_builder._match_exact = self._match_exact

        # Handle different overloads
        if isinstance(name_or_tag, CodebookName) and value is not None:
            # CodebookName and value overload
            tag = MetadataTag(name_or_tag, value)
            new_builder._tags[tag.name] = tag
        elif isinstance(name_or_tag, MetadataTag):
            # MetadataTag overload
            new_builder._tags[name_or_tag.name] = name_or_tag
        else:
            raise TypeError("Invalid arguments for with_tag")

        return new_builder

    def with_allow_other_tags(self, allow_others: bool) -> "MetadataTagsQueryBuilder":
        """Configure whether to allow other tags."""
        new_builder = MetadataTagsQueryBuilder()
        new_builder._tags = dict(self._tags)
        new_builder._match_exact = not allow_others
        return new_builder

    def match(self, local_id: LocalId | None) -> bool:  # noqa : C901
        """Check if this query matches the provided local ID."""
        if local_id is None:
            return False

        metadata_tags = {tag.name: tag for tag in local_id.metadata_tags}

        if len(self._tags) > 0:
            if self._match_exact:
                # When matching exactly, the counts must match and tags must be equal
                if len(self._tags) != len(metadata_tags):
                    return False

                for key, tag in self._tags.items():
                    if key not in metadata_tags:
                        return False
                    if tag != metadata_tags[key]:
                        return False

                return True

            # When not matching exactly, all our tags must be in the target
            for tag in self._tags.values():
                if tag.name not in metadata_tags:
                    return False
                if tag != metadata_tags[tag.name]:
                    return False

            return True

        # Empty tag list matches everything unless match_exact is True
        return not self._match_exact
