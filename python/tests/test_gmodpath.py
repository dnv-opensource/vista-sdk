"""Unit tests for GmodPath and related VIS SDK functionality."""

import os
import unittest

from dotenv import load_dotenv
from pydantic import ValidationError

from tests.testdata import TestData
from vista_sdk.client import Client
from vista_sdk.gmod_path import GmodPath
from vista_sdk.vis_version import VisVersion, VisVersionExtension, VisVersions

from .test_vis import TestVis

load_dotenv()
environment = os.getenv("ENVIRONMENT")


class TestGmodPath(unittest.TestCase):
    """Unit tests for GmodPath functionality."""

    def setUp(self) -> None:
        """Set up the test environment."""
        try:
            self.gmod_test_data = TestData.get_gmodpath_data("GmodPaths")
            self.test_individualizable_sets_data = (
                TestData.get_individualizable_sets_data("IndividualizableSets")
            )
        except ValidationError as e:
            raise Exception("Couldn't load test data") from e
        self.vis = TestVis.get_vis()
        if environment == "local":
            Client.get_gmod_test(
                VisVersionExtension.to_version_string(VisVersion.v3_4a)
            )
        else:
            Client.get_gmod(VisVersionExtension.to_version_string(VisVersion.v3_4a))

    def test_gmodpath_parse(self) -> None:
        """Test parsing of valid GMOD paths."""
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path: str = item.path
                path = GmodPath.try_parse(input_path, vis_version)
                assert path[0], "Path parsing failed for valid input"
                assert path[1] is not None, "Parsed path is None for valid input"
                assert input_path == path[1].__str__(), (
                    "Parsed path does not match input path"
                )

    def test_gmodpath_parse_invalid(self) -> None:
        """Test parsing of invalid GMOD paths."""
        # Test with invalid paths
        for item in self.gmod_test_data.invalid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path: str = item.path
                path = GmodPath.try_parse(input_path, vis_version)
                assert not path[0], "Path parsing should fail for invalid input"
                assert path[1] is None, "Parsed path should be None for invalid input"

    def test_get_full_path(self) -> None:
        """Test retrieval of the full path from a GMOD path."""
        # Test with a valid path
        path_str = "411.1/C101.72/I101"
        expectation = {
            0: "VE",
            1: "400a",
            2: "410",
            3: "411",
            4: "411i",
            5: "411.1",
            6: "CS1",
            7: "C101",
            8: "C101.7",
            9: "C101.72",
            10: "I101",
        }

        seen = set()
        path = GmodPath.parse(path_str, arg=VisVersion.v3_4a)

        for depth, node in path.get_full_path():
            if depth in seen:
                self.fail("Got same depth twice")
            seen.add(depth)
            if len(seen) == 1:
                assert depth == 0, "First depth should be 0"
            assert expectation[depth] == node.code, (
                f"Expected {expectation[depth]} at depth {depth}, got {node.code}"
            )

        assert sorted(expectation.keys()) == sorted(seen), (
            "Seen depths do not match expected depths"
        )

    def test_get_full_path_from(self) -> None:
        """Test retrieval of the full path from a specific depth in a GMOD path."""
        path_str = "411.1/C101.72/I101"
        expectation = {
            4: "411i",
            5: "411.1",
            6: "CS1",
            7: "C101",
            8: "C101.7",
            9: "C101.72",
            10: "I101",
        }

        seen = set()
        path = GmodPath.parse(path_str, arg=VisVersion.v3_4a)

        for depth, node in path.get_full_path_from(4):
            if depth in seen:
                self.fail("Got same depth twice")
            seen.add(depth)
            if len(seen) == 1:
                assert depth == 4, "First depth should be 4"
            assert expectation[depth] == node.code, (
                f"Expected {expectation[depth]} at depth {depth}, got {node.code}"
            )

        assert sorted(expectation.keys()) == sorted(seen), (
            "Seen depths do not match expected depths"
        )

    def test_full_path_parsing(self) -> None:
        """Test parsing of full paths into GmodPath objects."""
        version = VisVersion.v3_4a
        short_path_strs: list[str] = ["612.21-1/C701.13/S93"]
        expected_full_path_strs: list[str] = [
            "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93"
        ]
        for i in range(len(short_path_strs)):
            path = GmodPath.parse(short_path_strs[i], arg=version)
            full_string = path.to_full_path_string()
            assert expected_full_path_strs[i] == full_string, (
                f"Expected full path string '{expected_full_path_strs[i]}', "
                f"got '{full_string}'"
            )

            parsed, parsed_path = GmodPath.try_parse_full_path(full_string, arg=version)
            assert parsed_path is not None, "Parsed path should not be None"
            assert parsed, "Expected parsing to succeed for valid full path"

            assert path == parsed_path, "Parsed path does not match original path"
            assert full_string == path.to_full_path_string(), (
                "Full path string does not match original path"
            )
            assert full_string == parsed_path.to_full_path_string(), (
                "Full path string does not match parsed path"
            )
            assert short_path_strs[i] == str(path), (
                f"Short path string '{short_path_strs[i]}' does not match path string '{path!s}'"  # noqa: E501
            )
            assert short_path_strs[i] == str(parsed_path), (
                f"Short path string '{short_path_strs[i]}' does not match parsed path string '{parsed_path!s}'"  # noqa: E501
            )

            parsed_path = GmodPath.parse_full_path(full_string, version)
            assert parsed_path is not None, "Parsed path should not be None"
            assert path == parsed_path, "Parsed path does not match original path"
            assert full_string == path.to_full_path_string(), (
                "Full path string does not match original path"
            )
            assert full_string == parsed_path.to_full_path_string(), (
                "Full path string does not match parsed path"
            )
            assert short_path_strs[i] == str(path), (
                f"Short path string '{short_path_strs[i]}' does not match path string '{path!s}'"  # noqa: E501
            )
            assert short_path_strs[i] == str(parsed_path), (
                f"Short path string '{short_path_strs[i]}' does not match parsed path string '{parsed_path!s}'"  # noqa: E501
            )

    def test_individualizable_sets(self) -> None:
        """Test individualizable sets in GMOD paths."""
        for item in self.test_individualizable_sets_data.data:
            with self.subTest(item=item):
                is_full_path: bool = item.is_full_path
                version = VisVersions.parse(item.vis_version)
                gmod = self.vis.get_gmod(version)

                if item.expected is None:
                    result = (
                        gmod.try_parse_from_full_path(item.path)
                        if is_full_path
                        else gmod.try_parse_path(item.path)
                    )
                    assert result is not None, "Parsing result should not be None"
                    assert not result[0], "Expected parsing to fail for invalid path"
                    assert result[1] is None, (
                        "Parsed path should be None for invalid path"
                    )
                    return
                path = (
                    gmod.parse_from_full_path(item.path)
                    if is_full_path
                    else gmod.parse_path(item.path)
                )
                sets = path.individualizable_sets
                assert len(item.expected) == len(sets), (
                    f"Expected {len(item.expected)} sets, got {len(sets)}"
                )
                for i in range(len(item.expected)):
                    assert item.expected[i] == [n.code for n in sets[i].nodes], (
                        f"Expected {item.expected[i]} at index {i}, got {[n.code for n in sets[i].nodes]}"  # noqa: E501
                    )

    def test_individualizable_sets_full_path(self) -> None:
        """Test individualizable sets in full paths."""
        for item in self.test_individualizable_sets_data.data:
            with self.subTest(item=item):
                is_full_path: bool = item.is_full_path
                version = VisVersions.parse(item.vis_version)
                gmod = self.vis.get_gmod(version)

                if is_full_path:
                    return

                if item.expected is None:
                    result_tuple = gmod.try_parse_path(item.path)
                    assert result_tuple is not None, "gmod.try_parse_path returned None"
                    result, parsed = result_tuple
                    assert not result, "Expected parsing to fail for invalid path"
                    assert parsed is None, "Parsed path should be None for invalid path"
                    return

                path = GmodPath.parse_full_path(
                    gmod.parse_path(item.path).to_full_path_string(), version
                )
                sets = path.individualizable_sets
                assert len(item.expected) == len(sets), (
                    f"Expected {len(item.expected)} sets, got {len(sets)}"
                )
                for i in range(len(item.expected)):
                    assert item.expected[i] == [node.code for node in sets[i].nodes], (
                        f"Expected {item.expected[i]} at index {i}, got {[node.code for node in sets[i].nodes]}"  # noqa: E501
                    )

    def test_gmod_path_does_not_individualize(self) -> None:
        """Test that GMOD paths that do not individualize return None."""
        version = VisVersion.v3_7a
        gmod = self.vis.get_gmod(version)
        result = gmod.try_parse_path("500a-1")
        assert result is not None, "gmod.try_parse_path returned None"
        parsed, path = result
        assert not parsed, "Parsed path should not be individualizable"
        assert path is None, (
            "Parsed path should be None for non-individualizable GMOD path"
        )

    def test_to_full_path_string(self) -> None:
        """Test conversion of GMOD paths to full path strings."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)
        path = gmod.parse_path("511.11-1/C101.663i-1/C663")
        assert (
            path.to_full_path_string()
            == "VE/500a/510/511/511.1/511.1i-1/511.11-1/CS1/C101/C101.6/C101.66/C101.663/C101.663i-1/C663"  # noqa: E501
        ), "Full path string does not match expected value"

        path = gmod.parse_path("846/G203.32-2/S110.2-1/E31")

        assert (
            path.to_full_path_string()
            == "VE/800a/840/846/G203/G203.3-2/G203.32-2/S110/S110.2-1/CS1/E31"
        ), "Full path string does not match expected value"

    def test_valid_gmod_path_individualizable_sets(self) -> None:
        """Test that all individualizable sets in valid GMOD paths are correctly identified."""  # noqa: E501
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path = item.path

                gmod = self.vis.get_gmod(vis_version=vis_version)
                path = gmod.parse_path(input_path)
                sets = path.individualizable_sets

                unique_codes = set()
                for individual_set in sets:
                    for node in individual_set.nodes:
                        assert node.code not in unique_codes, (
                            f"Node {node.code} should not be duplicated in individualizable sets"  # noqa: E501
                        )
                        unique_codes.add(node.code)

                        assert node.code in unique_codes, (
                            f"Node {node.code} should be unique in individualizable sets"  # noqa: E501
                        )

    def test_valid_gmod_path_individualizable_sets_full_path(self) -> None:
        """Test that all individualizable sets in valid GMOD paths are correctly identified."""  # noqa: E501
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path = item.path

                gmod = self.vis.get_gmod(vis_version=vis_version)

                full_path_str = gmod.parse_path(input_path).to_full_path_string()
                path = GmodPath.parse_full_path(full_path_str, vis_version)
                sets = path.individualizable_sets

                unique_codes = set()
                for individual_set in sets:
                    print(f"Individual set: {individual_set}")
                    for node in individual_set.nodes:
                        assert node.code not in unique_codes, (
                            f"Node {node.code} should not be duplicated in individualizable sets"  # noqa: E501
                        )
                        unique_codes.add(node.code)

                        assert node.code in unique_codes, (
                            f"Node {node.code} should be unique in individualizable sets"  # noqa: E501
                        )

    def test_common_names(self) -> None:
        """Test that all individualizable nodes have common names."""
        for item in self.gmod_test_data.valid:
            with self.subTest(item=item):
                vis_version = VisVersions.parse(item.vis_version)
                input_path = item.path

                gmod = self.vis.get_gmod(vis_version=vis_version)
                path = gmod.parse_path(input_path)
                sets = path.individualizable_sets

                unique_codes = set()
                for individual_set in sets:
                    for node in individual_set.nodes:
                        assert node.code not in unique_codes, (
                            f"Node {node.code} should not be duplicated in individualizable sets"  # noqa: E501
                        )
                        unique_codes.add(node.code)

                        assert node.code in unique_codes, (
                            f"Node {node.code} should be unique in individualizable sets"  # noqa: E501
                        )
                        assert node.metadata.common_name is not None, (
                            f"Node {node.code} should have a common name"
                        )
                        assert node.metadata.common_name != "", (
                            f"Node {node.code} common name should not be empty"
                        )

    def test_verbose_path(self) -> None:
        """Test conversion of GMOD paths to verbose strings."""
        path_strs = ["411.1-1/C101.71/I101", "411.1/C102.321/C502", "1000.1/F401.2"]
        target_strs = [
            "Propulsion engine 1/Control monitoring and alarm system",
            "Propulsion steam turbine/Intermediate "
            "pressure turbine rotor blade arrangement",
            "Cargo data/Deadweight carried",
        ]
        for i, path_str in enumerate(path_strs):
            with self.subTest(path_str=path_str):
                path = GmodPath.parse(path_str, arg=VisVersion.v3_7a)
                verbose_path = path.to_verbose_string()
                assert target_strs[i] == verbose_path, (
                    f"Expected '{target_strs[i]}', got '{verbose_path}'"
                )
