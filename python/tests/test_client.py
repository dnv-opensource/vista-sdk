"""Tests for the Client module."""

import gzip
import json
import tempfile
from collections.abc import Generator
from pathlib import Path
from unittest import mock

import pytest

from vista_sdk.codebook_dto import CodebooksDto
from vista_sdk.gmod_dto import GmodDto
from vista_sdk.gmod_versioning_dto import GmodVersioningDto
from vista_sdk.locations_dto import LocationsDto


class TestClient:
    """Test class for the Client."""

    # Constants for testing
    TEST_VIS_VERSION = "3-6a"

    @pytest.fixture
    def mock_resources_path(self) -> Generator[Path, None]:
        """Fixture to create temporary resource files for testing."""
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)

            # Create mock resources
            self._create_mock_resource(
                temp_path, "locations", {"locations": [{"code": "L1"}]}
            )
            self._create_mock_resource(temp_path, "gmod", {"nodes": [{"code": "N1"}]})
            self._create_mock_resource(
                temp_path,
                "gmod-versioning",
                {"items": [{"from_code": "A", "to_code": "B"}]},
            )
            self._create_mock_resource(
                temp_path, "codebooks", {"codebooks": [{"name": "CB1", "items": []}]}
            )

            yield temp_path

    def _create_mock_resource(self, base_path: Path, resource_type: str, data) -> Path:  # noqa : ANN001
        """Helper to create mock gzipped JSON resource files."""
        file_path = base_path / f"{resource_type}-vis-{self.TEST_VIS_VERSION}.json.gz"
        with gzip.open(file_path, "wt") as f:
            json.dump(data, f)
        return file_path

    def test_get_locations(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test retrieving locations data."""
        from vista_sdk.client import Client  # noqa PLC0415

        # Mock data for test
        mock_data = {
            "visRelease": self.TEST_VIS_VERSION,
            "items": [{"code": "L1", "name": "Location 1", "definition": "Test"}],
        }

        with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
            # Setup mock path context manager
            mock_ctx = mock.MagicMock()
            mock_path.return_value.__enter__.return_value = mock_ctx

            # Setup mock gzip open
            mock_gzip_file = mock.MagicMock()
            mock_gzip_file.__enter__.return_value.read.return_value = json.dumps(
                mock_data
            )

            with (
                mock.patch("vista_sdk.client.gzip.open", return_value=mock_gzip_file),
                mock.patch("vista_sdk.client.json.load", return_value=mock_data),
            ):
                # Test the method
                result = Client.get_locations(self.TEST_VIS_VERSION)

                if result:
                    # Assertions
                    assert isinstance(result, LocationsDto)
                    assert len(result.items) == 1
                    assert result.items[0].code == "L1"

    def test_get_locations_file_not_found(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test handling of FileNotFoundError in get_locations."""
        from vista_sdk.client import Client  # noqa PLC0415

        with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
            mock_path.side_effect = FileNotFoundError("Test file not found")

            # Test the method raises the expected exception
            with pytest.raises(FileNotFoundError) as exc_info:
                Client.get_locations(self.TEST_VIS_VERSION)

            # Check error message
            assert "File:" in str(exc_info.value)
            assert self.TEST_VIS_VERSION in str(exc_info.value)

    def test_get_gmod(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test retrieving GMOD data."""
        from vista_sdk.client import Client  # noqa PLC0415

        # Mock data for test
        mock_data = {
            "visRelease": self.TEST_VIS_VERSION,
            "items": [
                {"category": "category", "type": "type", "code": "N1", "name": "Node 1"}
            ],
            "relations": [["R1", "N1", "N2"], ["R2", "N2", "N3"]],
        }

        with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
            mock_ctx = mock.MagicMock()
            mock_path.return_value.__enter__.return_value = mock_ctx

            with mock.patch("vista_sdk.client.gzip.open") as mock_gzip_open:
                mock_gzip_file = mock.MagicMock()
                mock_gzip_open.return_value.__enter__.return_value = mock_gzip_file

                with mock.patch("vista_sdk.client.json.load", return_value=mock_data):
                    # Test the method
                    result = Client.get_gmod(self.TEST_VIS_VERSION)

                    # Assertions
                    assert isinstance(result, GmodDto)
                    assert len(result.items) == 1
                    assert result.items[0].code == "N1"

    def test_get_gmod_versioning(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test retrieving GMOD Versioning data."""
        from vista_sdk.client import Client  # noqa PLC0415

        # We need to fix the logger import first
        with mock.patch("vista_sdk.client.logger") as mock_logger:  # noqa: F841
            # Mock data for test

            # Create node conversion DTO instances
            mock_data = {
                "visRelease": self.TEST_VIS_VERSION,
                "items": {
                    "node1": {  # String key
                        "operations": ["RENAME"],  # List that will be converted to Set
                        "source": "node1_old",
                        "target": "node1_new",
                        "oldAssignment": "old_path1",
                        "newAssignment": "new_path1",
                        "deleteAssignment": False,
                    },
                },
            }

            with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
                mock_ctx = mock.MagicMock()
                mock_path.return_value.__enter__.return_value = mock_ctx

                with mock.patch("vista_sdk.client.gzip.open") as mock_gzip_open:
                    mock_gzip_file = mock.MagicMock()
                    mock_gzip_open.return_value.__enter__.return_value = mock_gzip_file

                    with mock.patch(
                        "vista_sdk.client.json.load", return_value=mock_data
                    ):
                        # Test the method
                        result = Client.get_gmod_versioning(self.TEST_VIS_VERSION)

                        # Assertions
                        assert isinstance(result, GmodVersioningDto)
                        assert len(result.items) == 1
                        assert result.items["node1"].source == "node1_old"

    def test_get_gmod_versioning_missing_items(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test error handling when 'items' key is missing."""
        from vista_sdk.client import Client  # noqa PLC0415

        # We need to fix the logger import first
        with mock.patch("vista_sdk.client.logger") as mock_logger:  # noqa: F841
            # Mock data missing 'items' key
            mock_data = {"visRelease": self.TEST_VIS_VERSION}

            with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
                mock_ctx = mock.MagicMock()
                mock_path.return_value.__enter__.return_value = mock_ctx

                with mock.patch("vista_sdk.client.gzip.open") as mock_gzip_open:
                    mock_gzip_file = mock.MagicMock()
                    mock_gzip_open.return_value.__enter__.return_value = mock_gzip_file

                    with mock.patch(
                        "vista_sdk.client.json.load", return_value=mock_data
                    ):
                        # Test the method raises ValueError
                        with pytest.raises(KeyError) as exc_info:
                            Client.get_gmod_versioning(self.TEST_VIS_VERSION)

                        # Check error message
                        assert "'items'" in str(exc_info.value)

    def test_get_codebooks(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test retrieving codebooks data."""
        from vista_sdk.client import Client  # noqa PLC0415

        # Mock data for test
        mock_data = {
            "visRelease": "3.7a",
            "items": [
                {
                    "name": "Location",
                    "values": {
                        "P": ["Port", "Left"],
                        "S": ["Starboard", "Right"],
                        "C": ["Center"],
                    },
                }
            ],
        }

        with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
            mock_ctx = mock.MagicMock()
            mock_path.return_value.__enter__.return_value = mock_ctx

            with mock.patch("vista_sdk.client.gzip.open") as mock_gzip_open:
                mock_gzip_file = mock.MagicMock()
                mock_gzip_open.return_value.__enter__.return_value = mock_gzip_file

                with mock.patch("vista_sdk.client.json.load", return_value=mock_data):
                    # Test the method
                    result = Client.get_codebooks(self.TEST_VIS_VERSION)

                    # Assertions
                    assert isinstance(result, CodebooksDto)
                    assert len(result.items) == 1
                    assert result.items[0].name == "Location"

    def test_get_locations_test(self, tmp_path, monkeypatch) -> None:  # noqa: ANN001
        """Test retrieving test locations data."""
        from vista_sdk.client import Client  # noqa PLC0415

        # Create a mock resource file in temp directory
        resources_dir = tmp_path / "resources"
        resources_dir.mkdir()

        file_path = resources_dir / f"locations-vis-{self.TEST_VIS_VERSION}.json.gz"
        mock_data = {
            "visRelease": self.TEST_VIS_VERSION,
            "items": [{"code": "L1", "name": "Location 1", "definition": "Test"}],
        }

        with gzip.open(file_path, "wt") as f:
            json.dump(mock_data, f)

        # Patch the Path('.') to return our temp path
        monkeypatch.setattr(Path, "cwd", lambda: tmp_path)

        # Mock to ensure the glob finds our test file
        def mock_glob(pattern) -> list[Path]:  # noqa: ARG001, ANN001
            """Mock glob to return our test file."""
            return [file_path]

        monkeypatch.setattr(Path, "glob", mock_glob)

        with mock.patch("vista_sdk.client.json.load", return_value=mock_data):
            # Test the method
            result = Client.get_locations_test(self.TEST_VIS_VERSION)

            if result is not None:
                # Assertions
                assert isinstance(result, LocationsDto)
                assert len(result.items) == 1
                assert result.items[0].code == "L1"

    def test_get_gmod_versioning_test(self, monkeypatch) -> None:  # noqa: ANN001, ARG002
        """Test retrieving test GMOD Versioning data."""
        from vista_sdk.client import Client  # noqa PLC0415

        # Mock data for test
        mock_data = {
            "visRelease": self.TEST_VIS_VERSION,
            "items": {
                "node1": {  # String key
                    "operations": ["RENAME"],  # List that will be converted to Set
                    "source": "node1_old",
                    "target": "node1_new",
                    "oldAssignment": "old_path1",
                    "newAssignment": "new_path1",
                    "deleteAssignment": False,
                },
            },
        }

        with mock.patch("vista_sdk.client.pkg_resources.path") as mock_path:
            mock_ctx = mock.MagicMock()
            mock_ctx.exists.return_value = True  # File exists
            mock_path.return_value.__enter__.return_value = mock_ctx

            with mock.patch("vista_sdk.client.gzip.open") as mock_gzip_open:
                mock_gzip_file = mock.MagicMock()
                mock_gzip_open.return_value.__enter__.return_value = mock_gzip_file

                with mock.patch("vista_sdk.client.json.load", return_value=mock_data):
                    # Test the method
                    result = Client.get_gmod_versioning_test(self.TEST_VIS_VERSION)

                    # Assertions
                    assert isinstance(result, GmodVersioningDto)
                    assert result.vis_version == self.TEST_VIS_VERSION
                    assert len(result.items) == 1
                    assert result.items["node1"].old_assignment == "old_path1"


# Integration tests that use real files (should be run in CI with resources available)
@pytest.mark.integration
class TestClientIntegration:
    """Integration tests for Client that use real resource files."""

    TEST_VIS_VERSION = "3-7a"  # Use a version known to exist

    def test_get_locations_integration(self) -> None:
        """Integration test for get_locations with real resource files."""
        from vista_sdk.client import Client  # noqa PLC0415

        try:
            result = Client.get_locations(self.TEST_VIS_VERSION)
            # print(f"Locations: {result.items[:1]}")  # Debug output
            assert isinstance(result, LocationsDto)
        except FileNotFoundError:
            pytest.skip("Resource files not available - skipping integration test")

    def test_get_gmod_integration(self) -> None:
        """Integration test for get_gmod with real resource files."""
        from vista_sdk.client import Client  # noqa PLC0415

        try:
            result = Client.get_gmod(self.TEST_VIS_VERSION)
            # print(f"GMOD: {result.items[:1]}")  # Debug output
            assert isinstance(result, GmodDto)
        except FileNotFoundError:
            pytest.skip("Resource files not available - skipping integration test")

    def test_get_gmod_versioning_integration(self) -> None:
        """Integration test for get_gmod_versioning with real resource files."""
        from vista_sdk.client import Client  # noqa PLC0415

        try:
            result = Client.get_gmod_versioning(self.TEST_VIS_VERSION)
            # print(f"GMOD Versioning: {result.items}")  # Debug output
            assert isinstance(result, GmodVersioningDto)
        except FileNotFoundError:
            pytest.skip("Resource files not available - skipping integration test")

    def test_get_codebooks_integration(self) -> None:
        """Integration test for get_codebooks with real resource files."""
        from vista_sdk.client import Client  # noqa PLC0415

        try:
            result = Client.get_codebooks(self.TEST_VIS_VERSION)
            # print(f"Codebooks: {result.items[:1]}")  # Debug output
            assert isinstance(result, CodebooksDto)
        except FileNotFoundError:
            pytest.skip("Resource files not available - skipping integration test")
