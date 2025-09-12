"""Module to test the codebooks functionality of the VISTA SDK."""

import pytest

from vista_sdk.codebook_names import CodebookName
from vista_sdk.vis_version import VisVersion


class TestCodebooks:
    """Tests for the Codebooks functionality in the VISTA SDK."""

    @pytest.fixture(autouse=True)
    def setUp(self) -> None:
        """Set up the test environment."""
        from vista_sdk.vis import VIS

        # Get VIS instance
        self.vis = VIS()

    def test_codebooks_loads(self) -> None:
        """Test that codebooks load correctly."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_4a)

        assert codebooks is not None, "Codebooks should not be None"

    def test_codebooks_equality(self) -> None:
        """Test that codebooks are equal."""
        codebooks1 = self.vis.get_codebooks(VisVersion.v3_4a)
        codebooks2 = self.vis.get_codebooks(VisVersion.v3_4a)

        assert codebooks1 == codebooks2, "Codebooks should be equal"
        assert codebooks1[CodebookName.Position].has_standard_value("centre"), (
            "Position codebook should have 'centre' as a standard value"
        )

    def test_codebook_name_properties(self) -> None:
        """Test that codebooks have the correct properties."""
        values = [item.value for item in CodebookName]

        assert len(values) == len(set(values)), "CodebookName values should be unique"

        for i, value in enumerate(sorted(values)):
            assert i + 1 == value, (
                f"Expected value {i + 1}, got {value} at position {i}"
            )
