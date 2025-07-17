"""Tests for the LocalIdQuery module."""

from collections.abc import Callable
from dataclasses import dataclass

import pytest

from vista_sdk.codebook_names import CodebookName
from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_path_query import GmodPathQueryBuilder
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_query_builder import LocalIdQuery, LocalIdQueryBuilder
from vista_sdk.metadata_tag import MetadataTag
from vista_sdk.metadata_tag_query_builder import (
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
)
from vista_sdk.vis import VisVersion


def create_tag_configurator(
    tag: MetadataTag,
) -> Callable[[MetadataTagsQueryBuilder], MetadataTagsQuery]:
    """Create a properly typed tag configuration function."""

    def configurator(tags: MetadataTagsQueryBuilder) -> MetadataTagsQuery:
        return tags.with_tag(tag).build()

    return configurator


@dataclass
class TestData:
    """Test data for local ID query tests."""

    local_id: str
    query: LocalIdQuery
    expected_match: bool


def get_test_data() -> list[TestData]:
    """Get test data for parametrized tests."""
    return [
        TestData(
            local_id="/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
            query=LocalIdQueryBuilder.empty()
            .with_primary_item(
                GmodPathQueryBuilder.from_path(
                    GmodPath.parse("1021.1i-6P/H123", VisVersion.v3_4a)
                )
                .without_locations()
                .build()
            )
            .build(),
            expected_match=True,
        ),
        TestData(
            local_id="/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
            query=LocalIdQueryBuilder.empty()
            .with_tags(
                lambda tags: tags.with_tag(CodebookName.Content, "sea.water").build()
            )
            .build(),
            expected_match=True,
        ),
        TestData(
            local_id="/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            query=LocalIdQueryBuilder.empty()
            .with_primary_item(GmodPath.parse("411.1/C101.31-1", VisVersion.v3_4a))
            .build(),
            expected_match=False,
        ),
        TestData(
            local_id="/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            query=LocalIdQueryBuilder.empty()
            .with_primary_item(
                GmodPathQueryBuilder.from_path(
                    GmodPath.parse("411.1-2/C101.63/S206", VisVersion.v3_4a)
                )
                .without_locations()
                .build()
            )
            .build(),
            expected_match=True,
        ),
        TestData(
            local_id="/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            query=LocalIdQueryBuilder.empty()
            .with_secondary_item(
                GmodPathQueryBuilder.from_path(
                    GmodPath.parse("/411.1/C101.31-2", VisVersion.v3_4a)
                )
                .without_locations()
                .build()
            )
            .build(),
            expected_match=True,
        ),
        TestData(
            local_id="/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low",
            query=LocalIdQueryBuilder.empty()
            .with_primary_item(
                GmodPathQueryBuilder.from_path(
                    GmodPath.parse("411.1", VisVersion.v3_4a)
                )
                .without_locations()
                .build()
            )
            .build(),
            expected_match=False,
        ),
        TestData(
            local_id="/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative",
            query=LocalIdQueryBuilder.from_string(
                "/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity"
            )
            .with_primary_item(
                GmodPathQueryBuilder.empty()
                .with_node(
                    GmodPath.parse("433.1", VisVersion.v3_7a).node,
                    match_all_locations=True,
                )
                .build()
            )
            .build(),
            expected_match=True,
        ),
    ]


class TestLocalIdQuery:
    """Test class for LocalIdQuery functionality."""

    @pytest.mark.parametrize("data", get_test_data())
    def test_matches(self, data: TestData) -> None:
        """Test that the query matches as expected."""
        local_id = LocalId.parse(data.local_id)
        assert local_id is not None

        match = data.query.match(local_id)
        assert match == data.expected_match

    @pytest.mark.parametrize(
        "local_id_str",
        [
            "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo/state-high.high/pos-stage-3/detail-discharge"
        ],
    )
    def test_happy_path(self, local_id_str: str) -> None:
        """Test the happy path with various combinations of matching."""
        local_id = LocalId.parse(local_id_str)

        primary_item = local_id.primary_item
        secondary_item = local_id.secondary_item
        assert secondary_item is not None

        # Match exact
        builder = LocalIdQueryBuilder.from_local_id(local_id)
        query = builder.build()
        assert query.match(local_id)

        def match_combination(individualized: bool) -> None:
            """Test different combinations of matching with individualization."""
            primary_query_builder = GmodPathQueryBuilder.from_path(primary_item)
            secondary_query_builder = GmodPathQueryBuilder.from_path(secondary_item)

            if not individualized:
                primary_query_builder = primary_query_builder.without_locations()
                secondary_query_builder = secondary_query_builder.without_locations()

            primary_query = primary_query_builder.build()
            secondary_query = secondary_query_builder.build()

            # Match primary
            builder = LocalIdQueryBuilder.empty().with_primary_item(primary_query)
            query = builder.build()
            assert query.match(local_id)

            # Match secondary
            builder = LocalIdQueryBuilder.empty().with_secondary_item(secondary_query)
            query = builder.build()
            assert query.match(local_id)

            # Match tags
            builder = LocalIdQueryBuilder.empty()
            for tag in local_id.metadata_tags:
                # Use helper function to create properly typed tag configurator
                builder = builder.with_tags(create_tag_configurator(tag))
            query = builder.build()
            assert query.match(local_id)

            # Match primary and secondary
            builder = (
                LocalIdQueryBuilder.empty()
                .with_primary_item(primary_query)
                .with_secondary_item(secondary_query)
            )
            query = builder.build()
            assert query.match(local_id)

            # Match primary and tags
            builder = LocalIdQueryBuilder.empty().with_primary_item(primary_query)
            for tag in local_id.metadata_tags:
                # Use helper function to create properly typed tag configurator
                builder = builder.with_tags(create_tag_configurator(tag))
            query = builder.build()
            assert query.match(local_id)

            # Match secondary and tags
            builder = LocalIdQueryBuilder.empty().with_secondary_item(secondary_query)
            for tag in local_id.metadata_tags:
                # Use helper function to create properly typed tag configurator
                builder = builder.with_tags(create_tag_configurator(tag))
            query = builder.build()
            assert query.match(local_id)

            # Match primary, secondary, and tags
            builder = (
                LocalIdQueryBuilder.empty()
                .with_primary_item(primary_query)
                .with_secondary_item(secondary_query)
            )
            for tag in local_id.metadata_tags:
                # Use helper function to create properly typed tag configurator
                builder = builder.with_tags(create_tag_configurator(tag))
            query = builder.build()
            assert query.match(local_id)

        # Test with and without individualization
        match_combination(individualized=True)
        match_combination(individualized=False)

    def test_variations(self) -> None:
        """Test variations of queries with different configurations."""
        local_id = LocalId.parse(
            "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo/state-high.high/pos-stage-3/detail-discharge"
        )

        primary_item = GmodPath.parse("1036.13i-2/C662", VisVersion.v3_4a)

        builder = LocalIdQueryBuilder.empty().with_primary_item(primary_item)
        query = builder.build()
        assert not query.match(local_id)

        # Use a builder to create a path without locations
        builder = builder.with_primary_item(
            GmodPathQueryBuilder.from_path(primary_item).without_locations().build()
        )
        query = builder.build()
        assert query.match(local_id)

        # Use path without locations
        primary_item_no_loc = primary_item.without_locations()
        builder = LocalIdQueryBuilder.empty().with_primary_item(primary_item_no_loc)
        query = builder.build()
        assert not query.match(local_id)

        # Use path without locations and builder
        builder = builder.with_primary_item(
            GmodPathQueryBuilder.from_path(primary_item_no_loc)
            .without_locations()
            .build()
        )
        query = builder.build()
        assert query.match(local_id)
