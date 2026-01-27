"""Unit tests for the VisVersion, VisVersionExtension, and VisVersions classes."""

from pathlib import Path

import pytest

from vista_sdk.source_generator.vis_versions_generator import (
    generate_vis_version_script,
)
from vista_sdk.vis_version import VisVersion, VisVersionExtension, VisVersions


class TestVisVersions:
    """Unit tests for the VisVersion, VisVersionExtension, and VisVersions classes."""

    def test_to_string(self) -> None:
        """Test the to_string method of VisVersionExtension."""
        result = str(VisVersion.v3_5a)
        assert result == "3-5a"

        result = f"{VisVersion.v3_5a}"
        assert result == "3-5a"

    def test_is_valid(self) -> None:
        """Test the is_valid method of VisVersionExtension."""
        assert not VisVersionExtension.is_valid("3-8a")
        assert VisVersionExtension.is_valid(VisVersion.v3_8a)

    def test_all_versions(self) -> None:
        """Test the all_versions method of VisVersions."""
        versions = VisVersions.all_versions()
        assert VisVersion.v3_7a in versions
        assert VisVersion.v3_8a in versions
        assert VisVersion.v3_9a in versions
        assert VisVersion.v3_10a in versions
        assert len(VisVersions.all_versions()) == 7, "There should be 7 versions"

    def test_try_parse(self) -> None:
        """Test the try_parse method of VisVersions."""
        assert VisVersions.try_parse("3-4a") == VisVersion.v3_4a
        with pytest.raises(ValueError, match="invalid-version"):
            VisVersions.try_parse("invalid-version")

    def test_parse(self) -> None:
        """Test the parse method of VisVersions."""
        with pytest.raises(ValueError, match="invalid-version"):
            VisVersions.parse("invalid-version")

    def test_vis_generation(self) -> None:
        """Test the generation of the VisVersions script."""
        root_dir = Path(__file__).parent.parent.parent.resolve()
        resources_dir = root_dir / "resources"
        output_file = root_dir / "python" / "src" / "vista_sdk" / "vis_versions.py"

        generate_vis_version_script(str(resources_dir), str(output_file))

    def test_ordering(self) -> None:
        """Test ordering of VisVersion enums."""
        versions = VisVersions.all_versions()
        sorted_versions = sorted(versions, key=lambda v: v.value)
        index34a = sorted_versions.index(VisVersion.v3_4a)
        index310a = sorted_versions.index(VisVersion.v3_10a)
        assert index34a < index310a, "VisVersion ordering is incorrect"
        assert versions == sorted_versions, "VisVersions are not in the correct order"

    def test_comparison(self) -> None:
        """Test comparison between VisVersion enums."""
        assert VisVersion.v3_4a != VisVersion.v3_5a
        assert VisVersion.v3_6a == VisVersion.v3_6a
        assert VisVersion.v3_7a.value < VisVersion.v3_8a.value
        assert VisVersion.v3_9a.value <= VisVersion.v3_9a.value
        assert VisVersion.v3_10a.value > VisVersion.v3_8a.value
