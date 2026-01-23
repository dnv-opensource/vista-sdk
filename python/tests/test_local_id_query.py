"""Tests for the LocalIdQuery module."""

from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path as FilePath

import pytest

from vista_sdk.codebook_names import CodebookName
from vista_sdk.gmod_path import GmodPath
from vista_sdk.gmod_path_query import GmodPathQueryBuilder
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_query_builder import (
    LocalIdQuery,
    LocalIdQueryBuilder,
    NodesConfig,
    PathConfig,
)
from vista_sdk.metadata_tag import MetadataTag
from vista_sdk.metadata_tag_query import (
    MetadataTagsQuery,
    MetadataTagsQueryBuilder,
)
from vista_sdk.vis import VIS, VisVersion


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


@pytest.fixture(params=[])
def test_data() -> list[TestData]:
    """Test data fixture."""
    return []


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

    @pytest.mark.parametrize("data", get_test_data())
    def test_match_all(self, data: TestData) -> None:
        """Test that an empty query matches all local IDs."""
        builder = LocalIdQueryBuilder.empty()
        query = builder.build()
        assert query.match(data.local_id)

    @pytest.mark.parametrize(
        "local_id_str",
        [
            "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature/state-high",
            "/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature/state-high",
            "/dnv-v2/vis-3-4a/412.723-A/C261/sec/411.1/C101/meta/qty-temperature/state-high/cmd-slow.down",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-high",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-running",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-failure",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-start",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-stop",
            "/dnv-v2/vis-3-4a/623.22i-1/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/cnt-lubricating.oil",
            "/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-remote.control",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-running",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-failure",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-start",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-stop",
            "/dnv-v2/vis-3-4a/623.22i-2/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/cnt-lubricating.oil",
            "/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-remote.control",
            "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-stand.by/cmd-start",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-control.location",
            "/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/detail-stand.by.start.or.power.failure",
            "/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-high",
            "/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-high",
            "/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-low",
            "/dnv-v2/vis-3-4a/412.723-A/CS6d/meta/qty-temperature",
            "/dnv-v2/vis-3-4a/411.1/C101.64i-1/S201.1/C151.2/S110/meta/cnt-hydraulic.oil/state-running",
        ],
    )
    def test_samples(self, local_id_str: str) -> None:
        """Test sample local IDs."""
        local_id = LocalId.parse(local_id_str)
        assert local_id is not None

        builder = LocalIdQueryBuilder.from_local_id(local_id)
        query = builder.build()
        assert query.match(local_id)

        query = LocalIdQueryBuilder.empty().build()
        assert query.match(local_id)

    def test_consistency_smoke_test(self) -> None:
        """Test consistency by reading LocalIds.txt and testing all."""
        path = FilePath(__file__).parent / "testdata" / "LocalIds.txt"
        if not path.exists():
            pytest.skip("LocalIds.txt not found")

        errored: list[tuple[str, Exception | None]] = []

        with path.open() as f:
            for line in f:
                local_id_str = line.strip()
                if not local_id_str:
                    continue
                try:
                    local_id = LocalId.parse(local_id_str)
                    builder = LocalIdQueryBuilder.from_local_id(local_id)
                    assert builder is not None
                    query = builder.build()
                    assert query is not None
                    match = query.match(local_id)
                    if not match:
                        errored.append((local_id_str, None))
                except Exception as e:
                    errored.append((local_id_str, e))

        if errored:
            for lid_str, exception in errored:
                print(f"Failed on {lid_str}")
                if exception:
                    print(exception)

        assert len(errored) == 0

    def test_unspecified_secondary(self) -> None:
        """Test secondary item requirement handling."""
        base_local_id = LocalId.parse("/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power")
        query_builder = LocalIdQueryBuilder.from_local_id(base_local_id)
        other_local_id = LocalId.parse(
            "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power"
        )
        query = query_builder.build()
        assert query.match(base_local_id)
        assert not query.match(other_local_id)

        query = query_builder.without_secondary_item().build()
        assert not query.match(other_local_id)

        query = query_builder.with_any_secondary_item().build()
        assert query.match(other_local_id)

    def test_with_any_node_before(self) -> None:
        """Test matching with any node before a specific node."""
        codebooks = VIS().get_codebooks(VisVersion.v3_9a)

        # Build query: Any path before C101, but C101 onwards must match,
        # plus qty-power tag
        base_path = GmodPath.parse("411.1/C101.31", VisVersion.v3_9a)
        base_local_id = LocalId.parse(
            "/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power"
        )
        specific_query = (
            LocalIdQueryBuilder.from_local_id(base_local_id)
            .with_primary_item(
                PathConfig(
                    lambda path: path.with_any_node_before(lambda p: p["C101"]).build()
                )
            )
            .build()
        )
        general_query = (
            LocalIdQueryBuilder.empty()
            .with_primary_item(
                base_path,
                PathConfig(
                    lambda path: path.with_any_node_before(lambda p: p["C101"]).build()
                ),
            )
            .with_tags(
                lambda tags: tags.with_tag(
                    codebooks.create_tag(CodebookName.Quantity, "power")
                ).build()
            )
            .build()
        )

        # Base - Should match
        assert general_query.match(base_local_id)

        l2 = LocalId.parse("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.3/meta/qty-power")
        assert specific_query.match(l2)
        assert general_query.match(l2)

        l3 = LocalId.parse("/dnv-v2/vis-3-9a/411.1/C101.31/meta/qty-power")
        assert not specific_query.match(l3)  # No sec node
        assert general_query.match(l3)

        l4 = LocalId.parse("/dnv-v2/vis-3-9a/411.1/C102.31/meta/qty-power")
        assert not general_query.match(l4)  # Different C-node
        assert not specific_query.match(l4)

        l5 = LocalId.parse("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.2/meta/qty-power")
        assert not specific_query.match(l5)  # Different sec
        assert general_query.match(l5)

        l6 = LocalId.parse("/dnv-v2/vis-3-9a/411.1/C101.31/sec/412.2/meta/qty-pressure")
        assert not specific_query.match(l6)  # Different quantity
        assert not general_query.match(l6)

    def test_use_case_1(self) -> None:
        """Test matching with multiple locations."""
        locations = VIS().get_locations(VIS().latest_vis_version)
        local_id = LocalId.parse(
            "/dnv-v2/vis-3-7a/433.1-P/C322/meta/qty-linear.vibration.amplitude"
            "/pos-driving.end/detail-iso.10816"
        )
        # Match both 433.1-P and 433.1-S
        query = (
            LocalIdQueryBuilder.from_local_id(local_id)
            .with_primary_item(
                PathConfig(
                    lambda builder: builder.with_node(
                        lambda nodes: nodes["433.1"],
                        False,
                        locations.parse("P"),
                        locations.parse("S"),
                    ).build()
                )
            )
            .build()
        )
        assert query.match(local_id)
        assert query.match(
            "/dnv-v2/vis-3-7a/433.1-S/C322/meta/qty-linear.vibration.amplitude"
            "/pos-driving.end/detail-iso.10816"
        )

    def test_use_case_2(self) -> None:
        """Test matching with gmod node."""
        local_id = LocalId.parse(
            "/dnv-v2/vis-3-7a/511.31/C121/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816"
        )
        gmod = VIS().get_gmod(local_id.vis_version)
        # Match all Wind turbine arrangements
        query = (
            LocalIdQueryBuilder.from_local_id(local_id)
            .with_primary_item(
                NodesConfig(
                    lambda builder: builder.with_node(gmod["511.3"], True).build()
                )
            )
            .build()
        )
        assert query.match(local_id)

        # Should not match Solar panel arrangements
        query = (
            LocalIdQueryBuilder.empty()
            .with_primary_item(
                NodesConfig(
                    lambda builder: builder.with_node(
                        gmod["511.4"], match_all_locations=True
                    ).build()
                )
            )
            .build()
        )
        assert not query.match(local_id)

    def test_use_case_3(self) -> None:
        """Test only matching 100% matches with tags."""
        local_id = LocalId.parse(
            "/dnv-v2/vis-3-7a/433.1-S/C322.91/S205/meta/qty-conductivity/detail-relative"
        )
        lid_builder = local_id.builder
        vis_version = local_id.vis_version

        query = (
            LocalIdQueryBuilder.from_local_id(local_id)
            .with_tags(lambda builder: builder.with_allow_other_tags(False).build())
            .build()
        )
        assert query.match(local_id)

        gmod = VIS().get_gmod(vis_version)
        codebooks = VIS().get_codebooks(vis_version)

        l1 = lid_builder.with_metadata_tag(
            codebooks.create_tag(CodebookName.Content, "random")
        ).build()
        l2 = lid_builder.with_primary_item(gmod.parse_path("433.1-1S")).build()
        assert not query.match(l1)
        assert not query.match(l2)

    def test_use_case_4(self) -> None:
        """Test generic query matching."""
        query = (
            LocalIdQueryBuilder.from_string(
                "/dnv-v2/vis-3-7a/511.11/C101/meta/qty-pressure/cnt-lubricating.oil"
            )
            .with_primary_item(
                PathConfig(lambda builder: builder.without_locations().build())
            )
            .build()
        )
        other = LocalId.parse(
            "/dnv-v2/vis-3-7a/511.11-1/C101/meta/qty-pressure/cnt-lubricating.oil"
        )

        assert query.match(other)
