"""Test for the Locations and Location classes."""

from __future__ import annotations

import pytest

from tests.testdata import LocationsTestDataItem, TestData
from vista_sdk.location_builder import LocationBuilder
from vista_sdk.locations import Location, LocationGroup
from vista_sdk.vis_version import VisVersion, VisVersions


class TestLocations:
    """Test class for the Locations functionality in the VIS SDK."""

    @pytest.fixture(autouse=True)
    def setUp(self) -> None:
        """Set up the test case with a VIS instance and locations."""
        from vista_sdk.vis import VIS

        self.vis = VIS()
        self.locations = self.vis.get_locations(VisVersion.v3_4a)

    @pytest.mark.parametrize("vis_version", VisVersions.all_versions())
    def test_locations_loads(self, vis_version: VisVersion) -> None:
        """Test that locations are loaded correctly."""
        self.locations = self.vis.get_locations(vis_version)
        assert self.locations is not None
        assert self.locations.groups is not None

    def test_location_groups_properties(self) -> None:
        """Test the properties of LocationGroup."""
        values = list(LocationGroup)
        values_int = [value.value for value in values]
        assert len(values_int) == len(set(values_int)), (
            "LocationGroup values should be unique"
        )
        assert len(values_int) == 5, "There should be exactly 5 LocationGroup values"
        assert LocationGroup.Number.value == 0, (
            "The NUMBER group should have a value of 0"
        )
        for i in range(len(values_int) - 1):
            assert values_int[i + 1] == values_int[i] + 1, (
                "LocationGroup values should be sequential"
            )

    @pytest.mark.parametrize(
        "test_data", TestData.get_locations_data("Locations").locations
    )
    def test_locations(self, test_data: LocationsTestDataItem) -> None:
        """Test the locations functionality."""
        value = test_data.value
        success = test_data.success
        output = test_data.output
        expected_error_messages = test_data.expected_error_messages
        succeeded, parsed_location, error_messages = (
            self.locations.try_parse_with_errors(value)
        )

        if not success:
            assert not succeeded, f"Parsing should have failed for: {value}"
            assert parsed_location is None, "Parsed location should be None on failure"
            if expected_error_messages:
                assert error_messages.has_errors, (
                    "There should be error messages on failure"
                )
                actual_errors = [err[1] for err in error_messages]
                assert actual_errors == expected_error_messages
        else:
            assert succeeded, f"Parsing should have succeeded for: {value}"
            assert error_messages.has_errors is False, (
                "There should be no error messages on success"
            )
            assert parsed_location is not None, "Parsed location should not be None"
            assert str(parsed_location) == output, (
                f"Expected output: {output}, got: {parsed_location!s}"
            )

    def test_location_parse_throwing(self) -> None:
        """Test that parsing invalid locations raises ValueError."""
        with pytest.raises(ValueError, match=".*"):
            self.locations.parse(None)
        with pytest.raises(ValueError, match=".*"):
            self.locations.parse("")

    def test_location_builder(self) -> None:
        """Test the LocationBuilder functionality."""
        location_str = "11FIPU"
        location = self.locations.parse(location_str)

        builder = LocationBuilder.create(self.locations)

        builder = (
            builder.with_number(11)
            .with_side("P")
            .with_transverse("I")
            .with_longitudinal("F")
            .with_value_char("U")
        )

        assert str(builder) == "11FIPU"
        assert builder.number == 11
        assert builder.side == "P"
        assert builder.vertical == "U"
        assert builder.transverse == "I"
        assert builder.longitudinal == "F"

        # Test validation exceptions
        with pytest.raises(ValueError):  # noqa: PT011
            builder.with_value_char("X")
        with pytest.raises(ValueError):  # noqa: PT011
            builder.with_number(-1)
        with pytest.raises(ValueError):  # noqa: PT011
            builder.with_number(0)
        with pytest.raises(ValueError):  # noqa: PT011
            builder.with_side("A")
        with pytest.raises(ValueError):  # noqa: PT011
            builder.with_value_char("a")

        assert location == builder.build()

        # Test with_location from built location
        builder = LocationBuilder.create(self.locations).with_location(builder.build())

        assert str(builder) == "11FIPU"
        assert builder.number == 11
        assert builder.side == "P"
        assert builder.vertical == "U"
        assert builder.transverse == "I"
        assert builder.longitudinal == "F"

        # Test modifying values
        builder = builder.with_value_char("S").with_value(2)

        assert str(builder) == "2FISU"
        assert builder.number == 2
        assert builder.side == "S"
        assert builder.vertical == "U"
        assert builder.transverse == "I"
        assert builder.longitudinal == "F"

    def test_location_builder_with_location_single_digit(self) -> None:
        """Test LocationBuilder with single digit locations."""
        # Test single digit location parsing
        builder1 = LocationBuilder.create(self.locations).with_location(Location("1"))
        assert builder1.number == 1
        assert str(builder1) == "1"

        builder5 = LocationBuilder.create(self.locations).with_location(Location("5"))
        assert builder5.number == 5
        assert str(builder5) == "5"

        builder9 = LocationBuilder.create(self.locations).with_location(Location("9"))
        assert builder9.number == 9
        assert str(builder9) == "9"

        # Test single digit with characters
        builder_mixed = LocationBuilder.create(self.locations).with_location(
            Location("1FIPU")
        )
        assert builder_mixed.number == 1
        assert builder_mixed.side == "P"
        assert builder_mixed.vertical == "U"
        assert builder_mixed.transverse == "I"
        assert builder_mixed.longitudinal == "F"
        assert str(builder_mixed) == "1FIPU"

    def test_location_builder_multi_digit_number_not_sorted(self) -> None:
        """Test that multi-digit numbers are NOT sorted."""
        # Test that multi-digit numbers are NOT sorted
        builder = (
            LocationBuilder.create(self.locations)
            .with_number(10)
            .with_side("S")
            .with_vertical("U")
            .with_longitudinal("F")
        )

        # Should be "10FSU" NOT "01FSU"
        # The number "10" should stay together, not be sorted as individual characters
        assert str(builder) == "10FSU"
        assert builder.number == 10

    def test_locations_equality(self) -> None:
        """Test Location equality."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)

        node1 = gmod["C101.663"].with_location("FIPU")
        node2 = gmod["C101.663"].with_location("FIPU")

        assert node1 == node2
        assert node1 is not node2
