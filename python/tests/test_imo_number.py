"""Tests for the ImoNumber class in the Vista SDK."""

import json
import sys
from pathlib import Path

import pytest

# Add src to path for imports
src_path = Path(__file__).parent.parent / "src"
sys.path.insert(0, str(src_path))

from vista_sdk.imo_number import ImoNumber


class ImoTestDataItem:
    """Represents a test data item for IMO number validation."""

    def __init__(self, value: str, success: bool, output: str | None) -> None:
        """Initialize a test data item.

        Args:
            value: The input value to test
            success: Whether parsing should succeed
            output: Expected output string representation
        """
        self.value = value
        self.success = success
        self.output = output


class ImoTestData:
    """Represents the complete test data structure."""

    def __init__(self, imo_numbers: list[ImoTestDataItem]) -> None:
        """Initialize test data.

        Args:
            imo_numbers: List of test data items
        """
        self.imo_numbers = imo_numbers


class TestImoNumber:
    """Unit tests for the ImoNumber class."""

    def test_validation(self) -> None:
        """Test IMO number validation using test data from JSON file.

        This test mirrors the C# Test_Validation method, loading test data
        from testdata/ImoNumbers.json and validating parsing behavior.
        """
        # Read test data from JSON file
        testdata_path = (
            Path(__file__).parent.parent.parent / "testdata" / "ImoNumbers.json"
        )

        with testdata_path.open(encoding="utf-8") as file:
            text = file.read()

        # Parse JSON data
        json_data: dict[str, list[dict[str, str | bool | None]]] = json.loads(text)

        # Convert to test data objects
        test_items = []
        for item_data in json_data["imoNumbers"]:
            output_value = (
                str(item_data["output"]) if item_data["output"] is not None else None
            )
            test_items.append(
                ImoTestDataItem(
                    value=str(item_data["value"]),
                    success=bool(item_data["success"]),
                    output=output_value,
                )
            )

        data = ImoTestData(test_items)

        # Test each item
        for item in data.imo_numbers:
            # Test try_parse method
            parsed_imo = ImoNumber.try_parse(item.value)

            if item.success:
                # Should parse successfully
                assert parsed_imo is not None, (
                    f"Expected successful parse for: {item.value}"
                )

                # Should not be default/None
                assert parsed_imo is not None, (
                    f"Parsed IMO should not be None for: {item.value}"
                )

                # Test string representation if expected output is provided
                if item.output is not None:
                    assert str(parsed_imo) == item.output, (
                        f"Expected output '{item.output}' for input '{item.value}', "
                        f"but got '{parsed_imo!s}'"
                    )
            else:
                # Should fail to parse
                assert parsed_imo is None, f"Expected parse failure for: {item.value}"

    def test_parsing_methods(self) -> None:
        """Test different IMO number parsing methods."""
        # Test valid IMO numbers
        valid_numbers = ["9074729", "IMO9074729", "1234567"]

        for value in valid_numbers:
            # Test parse method (should not raise exception)
            imo = ImoNumber.parse(value)
            assert imo is not None

            # Test try_parse method
            parsed_imo = ImoNumber.try_parse(value)
            assert parsed_imo is not None
            assert parsed_imo == imo

        # Test invalid IMO numbers
        invalid_numbers = ["-1", "0", "123412034", "IM9074729"]

        for value in invalid_numbers:
            # Test parse method (should raise exception)
            with pytest.raises(ValueError, match="Invalid IMO number"):
                ImoNumber.parse(value)

            # Test try_parse method (should return None)
            parsed_imo = ImoNumber.try_parse(value)
            assert parsed_imo is None

    def test_imo_number_properties(self) -> None:
        """Test IMO number properties and methods."""
        # Create IMO number from integer
        imo1 = ImoNumber(9074729)
        assert imo1.value == 9074729
        assert str(imo1) == "IMO9074729"
        assert int(imo1) == 9074729

        # Create IMO number from string
        imo2 = ImoNumber("9074729")
        assert imo2.value == 9074729
        assert str(imo2) == "IMO9074729"

        # Test equality
        assert imo1 == imo2
        assert hash(imo1) == hash(imo2)

        # Test with IMO prefix
        imo3 = ImoNumber("IMO9074729")
        assert imo3.value == 9074729
        assert imo3 == imo1

    def test_imo_number_validation_algorithm(self) -> None:
        """Test the IMO number validation algorithm."""
        # Test known valid IMO numbers
        valid_imos = [
            9074729,  # Example from IMO documentation
            9785811,
            9704611,
            1234567,  # Test case from the data
        ]

        for imo_value in valid_imos:
            assert ImoNumber.is_valid(imo_value), f"IMO {imo_value} should be valid"
            imo = ImoNumber(imo_value)
            assert imo.value == imo_value

        # Test known invalid IMO numbers
        invalid_imos = [
            -1,
            0,
            1,
            123412034,  # Too many digits
            1234507,  # Invalid check digit
        ]

        for imo_value in invalid_imos:
            assert not ImoNumber.is_valid(imo_value), (
                f"IMO {imo_value} should be invalid"
            )
            with pytest.raises(ValueError, match="Invalid IMO number"):
                ImoNumber(imo_value)

    def test_edge_cases(self) -> None:
        """Test edge cases and error conditions."""
        # Test with None
        assert ImoNumber.try_parse("") is None

        # Test with non-numeric strings
        assert ImoNumber.try_parse("abc") is None
        assert ImoNumber.try_parse("IMOabc") is None

        # Test with malformed IMO prefix
        assert ImoNumber.try_parse("IM9074729") is None
        assert ImoNumber.try_parse("IMO") is None

        # Test boundary values
        assert not ImoNumber.is_valid(999999)  # Too small
        assert not ImoNumber.is_valid(10000000)  # Too large

        # Test whitespace handling
        imo = ImoNumber.try_parse("  9074729  ")
        assert imo is not None
        assert imo.value == 9074729

        imo_with_prefix = ImoNumber.try_parse("  IMO9074729  ")
        assert imo_with_prefix is not None
        assert imo_with_prefix.value == 9074729

    def test_repr_method(self) -> None:
        """Test the __repr__ method."""
        imo = ImoNumber(9074729)
        repr_str = repr(imo)
        assert "ImoNumber" in repr_str
        assert "9074729" in repr_str
