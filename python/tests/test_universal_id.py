"""Tests for Universal ID functionality."""

import pytest

from tests.testdata import TestData  # type: ignore
from vista_sdk.imo_number import ImoNumber  # type: ignore
from vista_sdk.universal_id import UniversalId  # type: ignore
from vista_sdk.universal_id_builder import UniversalIdBuilder


class TestUniversalId:
    """Test class for Universal ID functionality."""

    @pytest.mark.parametrize("test_case", TestData.get_universal_id_test_data())
    def test_try_parsing(self, test_case: str) -> None:
        """Test UniversalIdBuilder.try_parse_with_errors method.

        Equivalent to C# Test_TryParsing.
        """
        success, errors, builder = UniversalIdBuilder.try_parse_with_errors(test_case)
        assert success, f"Parsing failed for: {test_case}. Errors: {errors}"
        assert builder is not None, f"Builder should not be None for: {test_case}"

    @pytest.mark.parametrize("test_case", TestData.get_universal_id_test_data())
    def test_parsing(self, test_case: str) -> None:
        """Test UniversalId.parse method.

        Equivalent to C# Test_Parsing.
        """
        universal_id = UniversalId.parse(test_case)

        assert universal_id is not None, (
            f"UniversalId should not be None for: {test_case}"
        )

        # Check that IMO number equals the expected value (1234567)
        expected_imo = ImoNumber(1234567)
        assert universal_id.imo_number == expected_imo, (
            f"IMO number should be {expected_imo}, got {universal_id.imo_number}"
        )

    @pytest.mark.parametrize(
        "test_case",
        TestData.get_universal_id_test_data(),
    )
    def test_to_string(self, test_case: str) -> None:
        """Test string representation of UniversalId.

        Equivalent to C# Test_ToString.
        """
        universal_id_builder = UniversalIdBuilder.parse(test_case)

        universal_id_string = str(universal_id_builder)
        assert universal_id_builder is not None, (
            f"UniversalIdBuilder should not be None for: {test_case}"
        )
        assert test_case == universal_id_string, (
            f"String representation should match original."
            f"Expected: {test_case}, Got: {universal_id_string}"
        )

    @pytest.mark.parametrize(
        "test_case",
        TestData.get_universal_id_test_data(),
    )
    def test_universal_builder_add_and_remove_all(self, test_case: str) -> None:
        """Test adding and removing components from UniversalIdBuilder.

        Equivalent to C# Test_UniversalBuilder_Add_And_RemoveAll.
        """
        success, universal_id_builder = UniversalIdBuilder.try_parse(test_case)

        assert success, f"Parsing should succeed for: {test_case}"
        assert universal_id_builder is not None, "UniversalIdBuilder should not be None"
        assert universal_id_builder.local_id is not None, "LocalId should not be None"
        assert universal_id_builder.imo_number is not None, (
            "ImoNumber should not be None"
        )

        # Test removing components (equivalent to WithoutImoNumber().WithoutLocalId())
        id_without_components = (
            universal_id_builder.without_imo_number().without_local_id()
        )

        assert id_without_components.local_id is None, (
            "LocalId should be None after removal"
        )
        assert id_without_components.imo_number is None, (
            "ImoNumber should be None after removal"
        )

    def test_universal_builder_try_with(self) -> None:
        """Test UniversalIdBuilder try_with methods with None values.

        Equivalent to C# Test_UniversalBuilder_TryWith.
        """
        universal_builder = UniversalIdBuilder()

        universal_builder, _ = universal_builder.try_with_local_id(None)
        universal_builder, _ = universal_builder.try_with_imo_number(None)

        assert universal_builder.local_id is None, "LocalId should be None"
        assert universal_builder.imo_number is None, "ImoNumber should be None"

    def test_invalid_input_handling(self) -> None:
        """Test handling of invalid input strings."""
        invalid_inputs = [
            "",
            "invalid_string",
            "data.dnv.com/invalid_imo/dnv-v2/vis-3-4a/test",
            "data.dnv.com/9999999/dnv-v2/vis-3-4a/invalid_local_id",
        ]

        for invalid_input in invalid_inputs:
            success, errors, builder = UniversalIdBuilder.try_parse_with_errors(
                invalid_input
            )
            assert not success, (
                f"Parsing should fail for invalid input: {invalid_input}"
            )
            assert errors is not None, (
                f"Errors should be provided for invalid input: {invalid_input}"
            )
            assert builder is None, (
                f"Builder should be None for invalid input: {invalid_input}"
            )

    def test_try_parse_simple_success(self) -> None:
        """Test successful parsing with try_parse_simple."""
        test_case = "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet"  # noqa: E501

        builder = UniversalIdBuilder.try_parse_simple(test_case)
        assert builder is not None, "Builder should not be None for valid input"
        assert builder.imo_number is not None, "IMO number should be set"
        assert builder.local_id is not None, "Local ID should be set"

    def test_try_parse_simple_failure(self) -> None:
        """Test failed parsing with try_parse_simple."""
        invalid_input = "invalid_string"

        builder = UniversalIdBuilder.try_parse_simple(invalid_input)
        assert builder is None, "Builder should be None for invalid input"

    def test_universal_id_try_parse_success(self) -> None:
        """Test successful UniversalId.try_parse."""
        test_case = "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet"  # noqa: E501

        success, universal_id = UniversalId.try_parse(test_case)
        assert success, "Parsing should succeed"
        assert universal_id is not None, "UniversalId should not be None"
        assert universal_id.imo_number is not None, "IMO number should be set"
        assert universal_id.local_id is not None, "Local ID should be set"

    def test_universal_id_try_parse_failure(self) -> None:
        """Test failed UniversalId.try_parse."""
        invalid_input = "invalid_string"

        success, universal_id = UniversalId.try_parse(invalid_input)
        assert not success, "Parsing should fail"
        assert universal_id is None, "UniversalId should be None"

    def test_universal_id_try_parse_with_errors(self) -> None:
        """Test UniversalId.try_parse_with_errors method."""
        # Test success case
        valid_input = "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet"  # noqa: E501
        success, errors, universal_id = UniversalId.try_parse_with_errors(valid_input)

        assert success, "Parsing should succeed"
        assert universal_id is not None, "UniversalId should not be None"

        # Test failure case
        invalid_input = "invalid_string"
        success, errors, universal_id = UniversalId.try_parse_with_errors(invalid_input)

        assert not success, "Parsing should fail"
        assert errors is not None, "Errors should be provided"
        assert universal_id is None, "UniversalId should be None"
