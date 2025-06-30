"""Test cases for the Codebook functionality in the Vista SDK."""

import pytest

from tests.vista_sdk_testdata import VistaSDKTestData
from vista_sdk.codebook_names import CodebookName
from vista_sdk.vis_version import VisVersion


class TestCodebook:
    """Tests for the Codebook functionality."""

    @pytest.fixture(autouse=True)
    def setUp(self) -> None:
        """Set up the test environment."""
        from vista_sdk.vis import VIS  # noqa: PLC0415

        # Get VIS instance
        self.vis = VIS()

    @pytest.mark.parametrize(
        ("input_value", "expected_output"), VistaSDKTestData.add_valid_position_data()
    )
    def test_position_validation(self, input_value: str, expected_output: str) -> None:
        """Test position validation."""
        # Get VIS instance
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        codebook_type = codebooks[CodebookName.Position]
        valid_position = codebook_type.validate_position(input_value)
        parsed_expected_output = self._position_validation_from_string(expected_output)

        assert parsed_expected_output == valid_position, (
            f"Expected {expected_output}, got {valid_position}"
        )

    @pytest.mark.parametrize(
        ("invalid_standard_value", "valid_standard_value"),
        VistaSDKTestData.add_positions_data(),
    )
    def test_positions(
        self, invalid_standard_value: str, valid_standard_value: str
    ) -> None:
        """Test position standard values."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        positions = codebooks[CodebookName.Position]

        assert not positions.has_standard_value(invalid_standard_value), (
            f"'{invalid_standard_value}' should not be a standard value"
        )
        assert positions.has_standard_value(valid_standard_value), (
            f"'{valid_standard_value}' should be a standard value"
        )

    def test_standard_values(self) -> None:
        """Test standard values retrieval."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        positions = codebooks[CodebookName.Position]

        assert positions.has_standard_value("upper"), (
            "'upper' should be a standard value"
        )
        raw_data = positions.raw_data
        assert "Vertical" in raw_data, "'Vertical' should be a group in the raw data"
        assert "upper" in raw_data["Vertical"], (
            "'upper' should be in the 'Vertical' group of raw data"
        )

    @pytest.mark.parametrize(
        ("invalid_group", "valid_value", "valid_group", "second_valid_value"),
        VistaSDKTestData.add_states_data(),
    )
    def test_states(
        self,
        invalid_group: str,
        valid_value: str,
        valid_group: str,
        second_valid_value: str,
    ) -> None:
        """Test state groups and values."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        states = codebooks[CodebookName.State]

        assert states is not None, "States codebook should not be None"
        assert not states.has_group(invalid_group), (
            f"'{invalid_group}' should not be a valid group"
        )

        assert states.has_standard_value(valid_value), (
            f"'{valid_value}' should be a standard value"
        )

        assert states.has_group(valid_group), f"'{valid_group}' should be a valid group"

        assert states.has_standard_value(second_valid_value), (
            f"'{second_valid_value}' should be a standard value"
        )

    @pytest.mark.parametrize(
        (
            "first_tag",
            "second_tag",
            "third_tag",
            "third_tag_prefix",
            "custom_tag",
            "custom_tag_prefix",
            "first_invalid_tag",
            "second_invalid_tag",
        ),
        VistaSDKTestData.add_tag_data(),
    )
    def test_create_tag(
        self,
        first_tag: str,
        second_tag: str,
        third_tag: str,
        third_tag_prefix: str,
        custom_tag: str,
        custom_tag_prefix: str,
        first_invalid_tag: str,
        second_invalid_tag: str,
    ) -> None:
        """Test creating metadata tags."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        codebook_type = codebooks[CodebookName.Position]

        metadata_tag1 = codebook_type.create_tag(first_tag)
        assert first_tag == str(metadata_tag1), (
            f"Expected tag value '{first_tag}', got '{metadata_tag1}'"
        )
        assert not metadata_tag1.is_custom, "First tag should not be custom"
        assert first_tag == metadata_tag1.value, (
            f"Expected tag value '{first_tag}', got '{metadata_tag1.value}'"
        )

        metadata_tag2 = codebook_type.create_tag(second_tag)
        assert second_tag == str(metadata_tag2), (
            f"Expected tag value '{second_tag}', got '{metadata_tag2}'"
        )
        assert not metadata_tag2.is_custom, "Second tag should not be custom"

        metadata_tag3 = codebook_type.create_tag(third_tag)
        assert third_tag == str(metadata_tag3), (
            f"Expected tag value '{third_tag}', got '{metadata_tag3}'"
        )
        assert not metadata_tag3.is_custom, "Third tag should not be custom"
        assert third_tag_prefix == metadata_tag3.prefix, (
            f"Expected tag prefix '{third_tag_prefix}', got '{metadata_tag3.prefix}'"
        )

        metadata_tag4 = codebook_type.create_tag(custom_tag)
        assert custom_tag == str(metadata_tag4), (
            f"Expected tag value '{custom_tag}', got '{metadata_tag4}'"
        )
        assert metadata_tag4.is_custom, "Custom tag should be custom"
        assert custom_tag_prefix == metadata_tag4.prefix, (
            f"Expected tag prefix '{custom_tag_prefix}', got '{metadata_tag4.prefix}'"
        )

        with pytest.raises(ValueError, match=".*"):
            codebook_type.create_tag(first_invalid_tag)
        assert codebook_type.try_create_tag(first_invalid_tag) is None, (
            f"Invalid tag '{first_invalid_tag}' should not be creatable"
        )

        with pytest.raises(ValueError, match=".*"):
            codebook_type.create_tag(second_invalid_tag)
        assert codebook_type.try_create_tag(second_invalid_tag) is None, (
            f"Invalid tag '{second_invalid_tag}' should not be creatable"
        )

    def test_get_groups(self) -> None:
        """Test retrieving codebook groups."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        groups = codebooks[CodebookName.Position].groups
        assert len(groups) > 1, "Should have more than one group"

        assert "Vertical" in groups, "'Vertical' should be in groups"
        raw_data = codebooks[CodebookName.Position].raw_data

        # -1 because we drop <number> as a group, like in C# implementation
        assert len(groups) == len(raw_data) - 1, (
            f"Groups length ({len(groups)})"
            f" should be raw data length ({len(raw_data)}) - 1"
        )
        assert "Vertical" in raw_data, "'Vertical' should be in raw data"

    def test_iterate_groups(self) -> None:
        """Test iterating through codebook groups."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_4a)
        groups = codebooks[CodebookName.Position].groups
        count = 0
        for _ in groups:
            count += 1

        assert count == 11, f"Expected 11 groups, got {count}"

    def test_iterate_values(self) -> None:
        """Test iterating through standard values."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_4a)
        values = codebooks[CodebookName.Position].standard_values
        count = 0
        for _ in values:
            count += 1

        assert count == 28, f"Expected 28 standard values, got {count}"

    @pytest.mark.parametrize(
        ("valid_custom_tag", "first_invalid_custom_tag", "second_invalid_custom_tag"),
        VistaSDKTestData.add_detail_tag_data(),
    )
    def test_detail_tag(
        self,
        valid_custom_tag: str,
        first_invalid_custom_tag: str,
        second_invalid_custom_tag: str,
    ) -> None:
        """Test detail tag handling."""
        codebooks = self.vis.get_codebooks(VisVersion.v3_5a)

        codebook = codebooks[CodebookName.Detail]
        assert codebook is not None, "Detail codebook should not be None"

        assert codebook.try_create_tag(valid_custom_tag) is not None, (
            f"Valid custom tag '{valid_custom_tag}' should be creatable"
        )
        assert codebook.try_create_tag(first_invalid_custom_tag) is None, (
            f"Invalid custom tag '{first_invalid_custom_tag}' should not be creatable"
        )
        assert codebook.try_create_tag(second_invalid_custom_tag) is None, (
            f"Invalid custom tag '{second_invalid_custom_tag}' should not be creatable"
        )

        with pytest.raises(ValueError, match=".*"):
            codebook.create_tag(first_invalid_custom_tag)
        with pytest.raises(ValueError, match=".*"):
            codebook.create_tag(second_invalid_custom_tag)

    def _position_validation_from_string(self, value: str):  # noqa: ANN202
        """Convert string representation to PositionValidationResult enum."""
        from vista_sdk.codebook import PositionValidationResult  # noqa: PLC0415

        mapping = {
            "Invalid": PositionValidationResult.Invalid,
            "InvalidOrder": PositionValidationResult.InvalidOrder,
            "InvalidGrouping": PositionValidationResult.InvalidGrouping,
            "Valid": PositionValidationResult.Valid,
            "Custom": PositionValidationResult.Custom,
        }
        return mapping.get(value, PositionValidationResult.Invalid)
