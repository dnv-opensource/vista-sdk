"""Test data utilities for Vista SDK tests."""

import json
from dataclasses import dataclass
from pathlib import Path

from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis_version import VisVersion, VisVersions


@dataclass
class InvalidLocalIds:
    """Model for invalid local IDs."""

    local_id_str: str
    expected_error_messages: list[str]


@dataclass
class LocalIdTestData:
    """Collection of invalid local IDs."""

    invalid_local_ids: list[InvalidLocalIds]


@dataclass
class GmodPathTestItem:
    """Model for GMOD path test items."""

    path: str
    vis_version: str


@dataclass
class GmodPathTestData:
    """Collection of valid and invalid GMOD paths."""

    valid: list[GmodPathTestItem]
    invalid: list[GmodPathTestItem]


@dataclass
class CodebookTestData:
    """Test data for codebook tests."""

    valid_position: list[list[str]]
    positions: list[list[str]]
    states: list[list[str]]
    tag: list[list[str]]
    detail_tag: list[list[str]]


@dataclass
class LocationsTestDataItem:
    """Model for location test items."""

    value: str
    success: bool
    output: str | None
    expected_error_messages: list[str]


@dataclass
class LocationsTestData:
    """Collection of location test data."""

    locations: list[LocationsTestDataItem]


@dataclass
class IndividualizableSetData:
    """Model for individualizable set data."""

    is_full_path: bool
    vis_version: str
    path: str
    expected: list[list[str]] | None


@dataclass
class VersioningFullPathTestItem:
    """Model for versioning full path test items."""

    from_version: str
    to_version: str
    source_full_path: str
    target_full_path: str


@dataclass
class VersioningTestItem:
    """Model for versioning test items."""

    source_version: VisVersion
    target_version: VisVersion
    source_path: GmodPath
    target_path: GmodPath
    conversion_code: str


class VistaSDKTestData:
    """Test data utilities for Vista SDK tests."""

    @staticmethod
    def get_data(test_name: str):  # noqa : ANN205
        """Load test data from JSON file."""
        path = Path(__file__).parent / "testdata" / f"{test_name}.json"
        try:
            if not path.exists():
                raise FileNotFoundError(f"Test data file not found: {path}")
            with path.open() as f:
                return json.load(f)
        except Exception as e:
            print(f"Error loading test data from {path}: {e!s}")
            raise

    @staticmethod
    def add_codebook_data(data: list[list[str]]) -> list[tuple]:
        """Convert codebook data into test parameters."""
        return [tuple(item) for item in data]

    @classmethod
    def add_valid_position_data(cls) -> list[tuple]:
        """Get valid position test data."""
        codebook_data = cls.codebook_test_data()
        return cls.add_codebook_data(codebook_data["ValidPosition"])

    @classmethod
    def add_states_data(cls) -> list[tuple]:
        """Get states test data."""
        codebook_data = cls.codebook_test_data()
        return cls.add_codebook_data(codebook_data["States"])

    @classmethod
    def add_positions_data(cls) -> list[tuple]:
        """Get positions test data."""
        codebook_data = cls.codebook_test_data()
        return cls.add_codebook_data(codebook_data["Positions"])

    @classmethod
    def add_tag_data(cls) -> list[tuple]:
        """Get tag test data."""
        codebook_data = cls.codebook_test_data()
        return cls.add_codebook_data(codebook_data["Tag"])

    @classmethod
    def add_detail_tag_data(cls) -> list[tuple]:
        """Get detail tag test data."""
        codebook_data = cls.codebook_test_data()
        return cls.add_codebook_data(codebook_data["DetailTag"])

    @classmethod
    def add_invalid_local_ids_data(cls) -> list[tuple[str, list[str]]]:
        """Get invalid local IDs test data."""
        data = cls.local_id_test_data()
        return [
            (item.local_id_str, item.expected_error_messages)
            for item in data.invalid_local_ids
        ]

    @classmethod
    def add_valid_gmod_paths_data(cls) -> list[tuple[GmodPathTestItem]]:
        """Get valid GMOD paths test data."""
        data = cls.gmod_path_test_data()
        return [(item,) for item in data.valid]

    @classmethod
    def add_invalid_gmod_paths_data(cls) -> list[tuple[GmodPathTestItem]]:
        """Get invalid GMOD paths test data."""
        data = cls.gmod_path_test_data()
        return [(item,) for item in data.invalid]

    @classmethod
    def add_locations_data(cls) -> list[tuple[str, bool, str | None, list[str]]]:
        """Get locations test data."""
        data = cls.locations_test_data()
        return [
            (item.value, item.success, item.output, item.expected_error_messages)
            for item in data.locations
        ]

    @classmethod
    def add_individualizable_sets_data(
        cls,
    ) -> list[tuple[bool, str, str, list[list[str]] | None]]:
        """Get individualizable sets test data."""
        data = cls.individualizable_sets_data()
        return [
            (item.is_full_path, item.vis_version, item.path, item.expected)
            for item in data
        ]

    @classmethod
    def add_versioning_data(cls) -> list[tuple[VersioningTestItem]]:
        """Get versioning test data."""
        data = cls.versioning_test_data()
        result = []

        for conversion_code, items in data.items():
            for item in items:
                source_version = VisVersions.parse(item["from"])
                target_version = VisVersions.parse(item["to"])

                source_path = GmodPath.parse_full_path(
                    item["sourceFullPath"], source_version
                )
                target_path = GmodPath.parse_full_path(
                    item["targetFullPath"], target_version
                )

                versioning_item = VersioningTestItem(
                    source_version=source_version,
                    target_version=target_version,
                    source_path=source_path,
                    target_path=target_path,
                    conversion_code=conversion_code,
                )
                result.append((versioning_item,))

        return result

    @classmethod
    def codebook_test_data(cls) -> dict:
        """Get codebook test data."""
        return cls.get_data("Codebook")

    @classmethod
    def local_id_test_data(cls) -> LocalIdTestData:
        """Get local ID test data."""
        data = cls.get_data("InvalidLocalIds")
        invalid_local_ids = [
            InvalidLocalIds(
                local_id_str=item["input"],
                expected_error_messages=item["expectedErrorMessages"],
            )
            for item in data["InvalidLocalIds"]
        ]
        return LocalIdTestData(invalid_local_ids=invalid_local_ids)

    @classmethod
    def gmod_path_test_data(cls) -> GmodPathTestData:
        """Get GMOD path test data."""
        data = cls.get_data("GmodPaths")

        valid = [
            GmodPathTestItem(path=item["path"], vis_version=item["visVersion"])
            for item in data["Valid"]
        ]

        invalid = [
            GmodPathTestItem(path=item["path"], vis_version=item["visVersion"])
            for item in data["Invalid"]
        ]

        return GmodPathTestData(valid=valid, invalid=invalid)

    @classmethod
    def locations_test_data(cls) -> LocationsTestData:
        """Get locations test data."""
        data = cls.get_data("Locations")
        locations = [
            LocationsTestDataItem(
                value=item["value"],
                success=item["success"],
                output=item.get("output"),
                expected_error_messages=item["expectedErrorMessages"],
            )
            for item in data["locations"]
        ]
        return LocationsTestData(locations=locations)

    @classmethod
    def individualizable_sets_data(cls) -> list[IndividualizableSetData]:
        """Get individualizable sets test data."""
        data = cls.get_data("IndividualizableSets")
        return [
            IndividualizableSetData(
                is_full_path=item["isFullPath"],
                vis_version=item["visVersion"],
                path=item["path"],
                expected=item.get("expected"),
            )
            for item in data
        ]

    @classmethod
    def versioning_test_data(cls) -> dict[str, list[dict]]:
        """Get versioning test data."""
        return cls.get_data("VersioningTestCases")
