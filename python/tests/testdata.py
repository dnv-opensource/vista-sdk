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

    value: str = Field(..., alias="value")
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
        path = Path("tests") / "testdata" / f"{test_name}.json"
        with path.open() as file:
            test_data_json = file.read()
            test_data_json = test_data_json.replace("\n", "")
            if test_name == "IndividualizableSets":
                test_data_json = '{"data":' + test_data_json + "}"
        try:
            data_dict = json.loads(test_data_json)
            return cls.model_validate(data_dict)

        except ValidationError as e:
            raise Exception(f"Couldn't deserialize: {cls.__name__}") from e

    @staticmethod
    def get_gmodpath_data(test_name: str) -> GmodPathTestData:
        """Load and validate GMOD path test data."""
        return TestData.get_data(test_name, GmodPathTestData)

    @staticmethod
    def get_local_id_data(test_name: str) -> LocalIdTestData:
        """Load and validate local ID test data."""
        return TestData.get_data(test_name, LocalIdTestData)

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
