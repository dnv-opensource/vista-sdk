"""Test suite for GmodPathQueryBuilder functionality."""

from collections.abc import Callable
from dataclasses import dataclass

import pytest

from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path_query import GmodPathQueryBuilder
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


@dataclass
class InputData:
    """Input data for GmodPathQuery tests."""

    path: str
    vis_version: VisVersion
    parameters: list[tuple[str, list[str | None] | None]]
    expected_match: bool


# Test data for Path builder tests
TEST_PATH_DATA = [
    InputData("411.1-1/C101", VisVersion.v3_4a, [], True),
    InputData("411.1-1/C101", VisVersion.v3_4a, [("411.1", ["1"])], True),
    InputData("411.1-1/C101", VisVersion.v3_4a, [("411.1", ["A"])], False),
    InputData("433.1-P/C322.31/C173", VisVersion.v3_4a, [("C322.31", None)], True),
    InputData(
        "433.1-P/C322.31-2/C173",
        VisVersion.v3_4a,
        [("433.1", ["P"]), ("C322.31", None)],
        True,
    ),
    InputData(
        "433.1-P/C322.31-2/C173",
        VisVersion.v3_4a,
        [("433.1", ["A"]), ("C322.31", None)],
        False,
    ),
    InputData(
        "433.1-P/C322.31-2/C173",
        VisVersion.v3_4a,
        [("433.1", ["P"]), ("C322.31", ["1"])],
        False,
    ),
    InputData(
        "433.1-A/C322.31-2/C173",
        VisVersion.v3_4a,
        [("433.1", ["P"]), ("C322.31", ["1"])],
        False,
    ),
    InputData(
        "433.1-A/C322.31-2/C173",
        VisVersion.v3_4a,
        [("433.1", None), ("C322.31", None)],
        True,
    ),
    InputData("433.1/C322.31-2/C173", VisVersion.v3_4a, [("433.1", ["A"])], False),
    InputData("433.1/C322.31-2/C173", VisVersion.v3_4a, [("433.1", [])], True),
]

# Test data for Nodes builder tests
TEST_NODES_DATA = [
    InputData("411.1-1/C101", VisVersion.v3_4a, [("411.1", ["1"])], True),
    InputData(
        "411.1-1/C101.61/S203.3/S110.2/C101", VisVersion.v3_7a, [("411.1", ["1"])], True
    ),
    InputData(
        "411.1/C101.61-1/S203.3/S110.2/C101",
        VisVersion.v3_7a,
        [("C101.61", ["1"])],
        True,
    ),
    InputData(
        "511.11/C101.61-1/S203.3/S110.2/C101",
        VisVersion.v3_7a,
        [("C101.61", ["1"])],
        True,
    ),
    InputData(
        "411.1/C101.61-1/S203.3/S110.2/C101",
        VisVersion.v3_7a,
        [("C101.61", None)],
        True,
    ),
    InputData(
        "511.11/C101.61-1/S203.3/S110.2/C101",
        VisVersion.v3_7a,
        [("C101.61", None)],
        True,
    ),
    InputData(
        "221.11/C1141.421/C1051.7/C101.61-2/S203",
        VisVersion.v3_7a,
        [("C101.61", None)],
        True,
    ),
    InputData(
        "411.1/C101.61-1/S203.3/S110.2/C101",
        VisVersion.v3_7a,
        [("411.1", None), ("C101.61", None)],
        True,
    ),
    InputData(
        "511.11/C101.61-1/S203.3/S110.2/C101",
        VisVersion.v3_7a,
        [("411.1", None), ("C101.61", None)],
        False,
    ),
    InputData(
        "411.1/C101.61/S203.3-1/S110.2/C101",
        VisVersion.v3_7a,
        [("S203.3", ["1"])],
        True,
    ),
    InputData(
        "411.1/C101.61/S203.3-1/S110.2/C101",
        VisVersion.v3_7a,
        [("S203.3", ["1"])],
        True,
    ),
]


@pytest.mark.parametrize("data", TEST_PATH_DATA)
def test_path_builder(data: InputData) -> None:
    """Test the Path builder functionality."""
    vis = VIS()
    vis_version = data.vis_version
    locations = vis.get_locations(vis_version)
    gmod = vis.get_gmod(vis_version)

    path_str = data.path
    path = gmod.parse_path(path_str)

    builder = GmodPathQueryBuilder.from_path(path)

    query = builder.build()
    # For consistency, the query should always match itself
    assert query.match(path) is True

    for node, locs in data.parameters:

        def node_selector(node: str) -> Callable[[dict[str, GmodNode]], GmodNode]:
            return lambda nodes: nodes[node]

        if locs is None or len(locs) == 0:
            builder = builder.with_node(node_selector(node), match_all_locations=True)
        else:
            location_objects = [locations.parse(loc) for loc in locs]
            builder = builder.with_node(node_selector(node), False, *location_objects)

    query = builder.build()
    match = query.match(path)
    assert match == data.expected_match


@pytest.mark.parametrize("data", TEST_NODES_DATA)
def test_nodes_builder(data: InputData) -> None:
    """Test the Nodes builder functionality."""
    vis = VIS()
    vis_version = data.vis_version
    locations = vis.get_locations(vis_version)
    gmod = vis.get_gmod(vis_version)

    path_str = data.path
    path = gmod.parse_path(path_str)

    builder = GmodPathQueryBuilder.empty()

    for n, locs in data.parameters:
        node = gmod[n]
        if locs is None or len(locs) == 0:
            builder = builder.with_node(node, match_all_locations=True)
        else:
            location_objects = [locations.parse(loc) for loc in locs]
            builder = builder.with_node(node, False, *location_objects)

    query = builder.build()
    match = query.match(path)
    assert match == data.expected_match


def test_with_any_node_before() -> None:
    """Test the WithAnyNodeBefore functionality."""
    vis = VIS()
    gmod = vis.get_gmod(VisVersion.v3_9a)

    # Base path: 411.1/C101.31
    base_path = gmod.parse_path("411.1/C101.31")

    # Build query that ignores nodes before C101, but C101.31 must match
    query = (
        GmodPathQueryBuilder.from_path(base_path)
        .with_any_node_before(lambda nodes: nodes["C101"])
        .without_locations()
        .build()
    )

    # Should match the base path
    assert query.match(base_path) is True

    # Should match path with same C101.31 but different parent (511.11 instead of 411.1)
    path_different_parent = gmod.parse_path("511.11/C101.31")
    assert query.match(path_different_parent) is True

    # Should match path with location on C101.31
    path_with_location = gmod.parse_path("411.1/C101.31-2")
    assert query.match(path_with_location) is True

    # Should NOT match path with different C-node (C102.31 instead of C101.31)
    path_different_c_node = gmod.parse_path("411.1/C102.31")
    assert query.match(path_different_c_node) is False

    # Should NOT match path that doesn't contain C101.31
    path_without_c101 = gmod.parse_path("411.1/C101.61/S203")
    assert query.match(path_without_c101) is False


def test_with_any_node_before_with_locations() -> None:
    """Test WithAnyNodeBefore with locations."""
    vis = VIS()
    gmod = vis.get_gmod(VisVersion.v3_4a)

    # Base path with location: 433.1-P/C322.31
    base_path = gmod.parse_path("433.1-P/C322.31")

    # Build query that ignores nodes before C322, keeps location requirement on C322.31
    query = (
        GmodPathQueryBuilder.from_path(base_path)
        .with_any_node_before(lambda nodes: nodes["C322"])
        .build()
    )

    # Should match the base path
    assert query.match(base_path) is True

    # Should match path with different parent but same C322.31
    path_different_parent = gmod.parse_path("433.1-S/C322.31")
    assert query.match(path_different_parent) is True

    # Should match path without location on parent since we ignore nodes before C322
    path_no_parent_location = gmod.parse_path("433.1/C322.31")
    assert query.match(path_no_parent_location) is True
