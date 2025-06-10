"""Test for the VIS singleton class."""

from __future__ import annotations

import pytest

from vista_sdk.location_builder import LocationBuilder
from vista_sdk.locations import Location, LocationGroup
from vista_sdk.vis_version import VisVersion


class TestLocations:
    """Test class for the Locations functionality in the VIS SDK."""

    @pytest.fixture(autouse=True)
    def setUp(self) -> None:
        """Set up the test case with a VIS instance and locations."""
        from vista_sdk.vis import VIS

        self.vis = VIS()
        self.locations = self.vis.get_locations(VisVersion.v3_4a)

    def test_locations_loads(self) -> None:
        """Test that locations are loaded correctly."""
        print("Locations loaded:", self.locations)
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
        assert LocationGroup.NUMBER.value == 0, (
            "The NUMBER group should have a value of 0"
        )
        for i in range(len(values_int) - 1):
            assert values_int[i + 1] == values_int[i] + 1, (
                "LocationGroup values should be sequential"
            )

    def test_locations(self) -> None:
        """Test the locations functionality."""
        value = "some_string"
        expected_error_messages = ["error_message1", "error_message2"]
        success, string_parsed_location = self.locations.try_parse(value)
        self.verify(
            (success, string_parsed_location),
            string_parsed_location,
            expected_error_messages,
        )

    def verify(
        self,
        succeeded: tuple[bool, Location | None],
        parsed_location: Location | None,
        _: object,
    ) -> None:
        """Verify the result of location parsing."""
        if not succeeded:
            assert parsed_location is None, (
                "Parsed location should be None when parsing fails"
            )
        else:
            assert parsed_location is not None, (
                "Parsed location should not be None when parsing succeeds"
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
        self.locations.parse(location_str)
        LocationBuilder.create(self.locations)
