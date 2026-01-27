"""This module provides test data models and a utility class for the Vista SDK.

This file defines data models and a utility class for handling test data
for various components of the Vista SDK, including local IDs, GMOD paths,
codebooks, locations, and individualizable sets.
It provides methods to load and validate test data from JSON files.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import TypeVar

from pydantic import BaseModel, Field, ValidationError

T = TypeVar("T", bound=BaseModel)


class InvalidLocalIds(BaseModel):
    """Data model for invalid local IDs in test data."""

    local_id_str: str = Field(..., alias="input")
    expected_error_messages: list[str] = Field(..., alias="expectedErrorMessages")


class LocalIdTestData(BaseModel):
    """Data model for local ID test data."""

    invalid_local_ids: list[InvalidLocalIds] = Field(..., alias="InvalidLocalIds")


class GmodPathTestItem(BaseModel):
    """Data model for GMOD path test items."""

    path: str = Field(..., alias="path")
    vis_version: str = Field(..., alias="visVersion")


class GmodPathTestData(BaseModel):
    """Data model for GMOD path test data."""

    valid: list[GmodPathTestItem] = Field(..., alias="Valid")
    invalid: list[GmodPathTestItem] = Field(..., alias="Invalid")


class CodebookTestData(BaseModel):
    """Data model for codebook test data."""

    valid_position: list[list[str]] = Field(..., alias="ValidPosition")
    positions: list[list[str]] = Field(..., alias="Positions")
    states: list[list[str]] = Field(..., alias="States")
    tag: list[list[str]] = Field(..., alias="Tag")
    detail_tag: list[list[str]] = Field(..., alias="DetailTag")


class LocationsTestDataItem(BaseModel):
    """Data model for individual location test items."""

    value: str | None = Field(..., alias="value")
    success: bool = Field(..., alias="success")
    output: str | None = Field(..., alias="output")
    expected_error_messages: list[str] = Field(..., alias="expectedErrorMessages")


class LocationsTestData(BaseModel):
    """Data model for locations test data."""

    locations: list[LocationsTestDataItem] = Field(..., alias="locations")


class IndividualizableSetData(BaseModel):
    """Data model for individualizable set data."""

    is_full_path: bool = Field(..., alias="isFullPath")
    vis_version: str = Field(..., alias="visVersion")
    path: str = Field(..., alias="path")
    expected: list[list[str]] | None = Field(..., alias="expected")


class IndividualizableSetDatalist(BaseModel):
    """Data model for a list of individualizable set data."""

    data: list[IndividualizableSetData] = Field(..., alias="data")


class TestData:
    """Utility class for loading and validating test data from JSON files."""

    @staticmethod
    def get_data(test_name: str, cls: type[T]) -> T:
        """Load and validate test data from a JSON file."""
        path = Path(__file__).parent / "testdata" / f"{test_name}.json"
        if path:
            with path.open() as file:
                test_data_json = file.read()
                test_data_json = test_data_json.replace("\n", "")
                if test_name == "IndividualizableSets":
                    test_data_json = '{"data":' + test_data_json + "}"
        else:
            FileExistsError(f"File not found: {path}")
        try:
            data_dict = json.loads(test_data_json)
            return cls.model_validate(data_dict)

        except ValidationError as e:
            raise Exception(f"Couldn't deserialize: {cls.__name__}") from e

    @staticmethod
    def get_gmodpath_data() -> GmodPathTestData:
        """Load and validate GMOD path test data."""
        return TestData.get_data("GmodPaths", GmodPathTestData)

    @staticmethod
    def get_valid_gmod_path_data() -> list[GmodPathTestItem]:
        """Load valid GMOD path test data."""
        data = TestData.get_gmodpath_data()
        return data.valid

    @staticmethod
    def get_local_id_data(test_name: str) -> LocalIdTestData:
        """Load and validate local ID test data."""
        return TestData.get_data(test_name, LocalIdTestData)

    @staticmethod
    def get_universal_id_test_data() -> list[str]:
        """Load universal ID test data as a list of strings."""
        return [
            "data.dnv.com/IMO1234567/dnv-v2/vis-3-4a/621.21/S90/sec/411.1/C101/meta/qty-mass/cnt-fuel.oil/pos-inlet",
            "data.dnv.com/IMO1234567/dnv-v2/vis-3-7a/612.21/C701.23/C633/meta/calc~accumulate",
        ]

    @staticmethod
    def get_codebook_data(test_name: str) -> CodebookTestData:
        """Load and validate codebook test data."""
        return TestData.get_data(test_name, CodebookTestData)

    @staticmethod
    def get_locations_data(test_name: str) -> LocationsTestData:
        """Load and validate locations test data."""
        return TestData.get_data(test_name, LocationsTestData)

    @staticmethod
    def get_individualizable_sets_data(test_name: str) -> IndividualizableSetDatalist:
        """Load and validate individualizable sets test data."""
        return TestData.get_data(test_name, IndividualizableSetDatalist)

    @staticmethod
    def load_invalid_local_ids() -> list[tuple[str, list[str]]]:
        """Load invalid local ID test data from the testdata directory."""
        data = TestData.get_local_id_data("InvalidLocalIds")
        return [
            (item.local_id_str, item.expected_error_messages)
            for item in data.invalid_local_ids
        ]

    # Codebook test data helpers
    @staticmethod
    def _add_codebook_data(data: list[list[str]]) -> list[tuple[str, ...]]:
        """Convert codebook data into test parameters."""
        return [tuple(item) for item in data]

    @classmethod
    def add_valid_position_data(cls) -> list[tuple[str, ...]]:
        """Get valid position test data."""
        codebook_data = cls.get_codebook_data("Codebook")
        return cls._add_codebook_data(codebook_data.valid_position)

    @classmethod
    def add_states_data(cls) -> list[tuple[str, ...]]:
        """Get states test data."""
        codebook_data = cls.get_codebook_data("Codebook")
        return cls._add_codebook_data(codebook_data.states)

    @classmethod
    def add_positions_data(cls) -> list[tuple[str, ...]]:
        """Get positions test data."""
        codebook_data = cls.get_codebook_data("Codebook")
        return cls._add_codebook_data(codebook_data.positions)

    @classmethod
    def add_tag_data(cls) -> list[tuple[str, ...]]:
        """Get tag test data."""
        codebook_data = cls.get_codebook_data("Codebook")
        return cls._add_codebook_data(codebook_data.tag)

    @classmethod
    def add_detail_tag_data(cls) -> list[tuple[str, ...]]:
        """Get detail tag test data."""
        codebook_data = cls.get_codebook_data("Codebook")
        return cls._add_codebook_data(codebook_data.detail_tag)
