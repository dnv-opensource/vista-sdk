"""Tests for GMOD versioning functionality."""

from typing import Any

import pytest

from tests.testdata import GmodPathTestItem, TestData
from vista_sdk.gmod_node import GmodNode
from vista_sdk.gmod_path import GmodPath
from vista_sdk.local_id_builder import LocalIdBuilder
from vista_sdk.vis_version import VisVersion, VisVersions


class TestGmodVersioning:
    """Tests for GmodVersioning functionality."""

    @pytest.fixture(autouse=True)
    def setup(self) -> None:
        """Set up the test environment."""
        from vista_sdk.vis import VIS

        self.vis = VIS()

    @staticmethod
    def valid_test_data_path() -> list[list[Any]]:
        """Get test data for valid paths."""
        return [
            ["411.1/C101.72/I101", "411.1/C101.72/I101"],
            ["323.51/H362.1", "323.61/H362.1"],
            ["321.38/C906", "321.39/C906"],
            ["511.331/C221", "511.31/C121.31/C221"],
            ["511.11/C101.663i/C663.5/CS6d", "511.11/C101.663i/C663.6/CS6d"],
            ["511.11-1/C101.663i/C663.5/CS6d", "511.11-1/C101.663i/C663.6/CS6d"],
            ["1012.21/C1147.221/C1051.7/C101.22", "1012.21/C1147.221/C1051.7/C101.93"],
            [
                "1012.21/C1147.221/C1051.7/C101.61/S203.6",
                "1012.21/C1147.221/C1051.7/C101.311/C467.5",
            ],
            ["001", "001"],
            ["038.7/F101.2/F71", "038.7/F101.2/F71"],
            [
                "1012.21/C1147.221/C1051.7/C101.61/S203.6/S61",
                "1012.21/C1147.221/C1051.7/C101.311/C467.5/S61",
            ],
            ["000a", "000a"],
            [
                "1012.21/C1147.221/C1051.7/C101.61/S203.2/S101",
                "1012.21/C1147.221/C1051.7/C101.61/S203.3/S110.1/S101",
            ],
            [
                "1012.21/C1147.221/C1051.7/C101.661i/C624",
                "1012.21/C1147.221/C1051.7/C101.661i/C621",
            ],
            [
                "1012.22/S201.1/C151.2/S110.2/C101.64i",
                "1012.22/S201.1/C151.2/S110.2/C101.64",
            ],
            [
                "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
                "632.32i/S110.2/C111.42/G203.31/S90.5/C401",
            ],
            [
                "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
                "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.42/G204.41/S90.2/S51",
            ],
            [
                "864.11/G71.21/C101.64i/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
                "864.11/G71.21/C101.64/S201.1/C151.31/S110.2/C111.41/G240.1/G242.2/S90.5/C401",
            ],
            ["221.31/C1141.41/C664.2/C471", "221.31/C1141.41/C664.2/C471"],
            ["514/E15", "514"],
            [
                "1346/S201.1/C151.31/S110.2/C111.1/C109.16/C509",
                "1346/S201.1/C151.31/S110.2/C111.1/C109.126/C509",
                VisVersion.v3_7a,
                VisVersion.v3_8a,
            ],
        ]

    @staticmethod
    def valid_test_data_full_path() -> list[list[str]]:
        """Get test data for full paths."""
        return [
            [
                "VE/600a/630/632/632.3/632.32/632.32i-2/S110",
                "VE/600a/630/632/632.3/632.32/632.32i-2/SS5/S110",
            ]
        ]

    @staticmethod
    def valid_test_data_node() -> list[list[Any]]:
        """Get test data for nodes."""
        return [["1014.211", None, "1014.211"], ["323.5", None, "323.6"]]

    @pytest.mark.parametrize(
        ("input_path", "expected_path", "source_version", "target_version"),
        [
            [*path, VisVersion.v3_4a, VisVersion.v3_6a] if len(path) == 2 else path
            for path in valid_test_data_path()
        ],
    )
    def test_gmod_versioning_convert_path(
        self,
        input_path: str,
        expected_path: str,
        source_version: VisVersion,
        target_version: VisVersion,
    ) -> None:
        """Test GmodVersioning.convert_path."""
        target_gmod = self.vis.get_gmod(target_version)

        # Parse the source path with better error handling
        try:
            source_path = GmodPath.parse(input_path, source_version)
        except ValueError as e:
            pytest.fail(
                f"Failed to parse source path '{input_path}'"
                f" with version {source_version}: {e}"
            )

        # Parse the expected target path
        try:
            parsed_target_path = GmodPath.parse(expected_path, target_gmod.vis_version)
        except Exception as e:
            pytest.fail(
                f"Failed to parse expected path '{expected_path}'"
                f" with version {target_version}: {e}"
            )

        # Convert the path
        target_path = self.vis.convert_path(
            source_version, source_path, target_gmod.vis_version
        )
        assert target_path is not None, "Target path should not be None"

        # Check that the converted path matches the expected path
        assert expected_path == str(target_path), (
            f"Expected path {expected_path} does not match target path {target_path}"
        )

        # Verify the original source path was parsed correctly
        assert input_path == str(source_path), (
            f"Input path {input_path} does not match parsed source path {source_path}"
        )

        # Check that target path was parsed correctly
        assert expected_path == str(parsed_target_path), (
            f"Expected path {expected_path}"
            f" does not match parsed target path {parsed_target_path}"
        )

    @pytest.mark.parametrize(
        ("input_path", "expected_path"), valid_test_data_full_path()
    )
    def test_gmod_versioning_convert_full_path(
        self, input_path: str, expected_path: str
    ) -> None:
        """Test GmodVersioning.convert_full_path."""
        source_version = VisVersion.v3_4a
        target_version = VisVersion.v3_6a

        target_gmod = self.vis.get_gmod(target_version)
        source_path = GmodPath.parse_full_path(input_path, source_version)

        parsed_target_path = GmodPath.parse_full_path(
            expected_path, target_gmod.vis_version
        )

        target_path = self.vis.convert_path(
            source_version, source_path, target_gmod.vis_version
        )

        # Assertions
        assert source_path is not None, "Source path should not be None"
        assert input_path == source_path.to_full_path_string(), (
            f"Input path {input_path} does not match parsed source path "
            f"{source_path.to_full_path_string()}"
        )

        assert parsed_target_path is not None, "Parsed target path should not be None"
        assert expected_path == parsed_target_path.to_full_path_string(), (
            f"Expected path {expected_path} does not match parsed target path "
            f"{parsed_target_path.to_full_path_string()}"
        )

        assert target_path is not None, "Target path should not be None"
        assert expected_path == target_path.to_full_path_string(), (
            f"Expected path {expected_path} does not match target path "
            f"{target_path.to_full_path_string()}"
        )

    @pytest.mark.parametrize(
        ("source", "target", "source_version", "target_version"),
        [
            (
                "244.1i/H101.111/H401",
                "244.1i/H101.11/H407.1/H401",
                VisVersion.v3_7a,
                VisVersion.v3_8a,
            )
        ],
    )
    def test_gmod_versioning_throw_exception(
        self,
        source: str,
        target: str,  # noqa: ARG002
        source_version: VisVersion,
        target_version: VisVersion,
    ) -> None:
        """This test expects a ValueError to be raised during conversion.

        This is currently to assert that this case is not allowed,
        due to the VIS team not handling this case:
            A merged node that previously had a normal assigment,
            but its target has a different normal assignment
        """
        source_path = GmodPath.parse(source, source_version)
        assert source_path is not None, "Source path should not be None"
        with pytest.raises(Exception):  # noqa: B017, PT011
            self.vis.convert_path(source_version, source_path, target_version)

    @pytest.mark.parametrize(
        ("input_code", "location", "expected_code"),
        [
            ("1014.211", None, "1014.211"),
            ("323.5", None, "323.6"),
            ("412.72", None, "412.7i"),
            ("323.4", None, "323.5"),
            ("323.51", None, "323.61"),
            ("323.6", None, "323.7"),
            ("C101.212", None, "C101.22"),
            ("C101.22", None, "C101.93"),
            ("511.31", None, "C121.1"),
            ("C101.31", "5", "C101.31"),
        ],
    )
    def test_gmod_versioning_convert_node(
        self, input_code: str, location: str | None, expected_code: str
    ) -> None:
        """Test GmodVersioning.convert_node."""
        gmod = self.vis.get_gmod(VisVersion.v3_4a)
        target_gmod = self.vis.get_gmod(VisVersion.v3_6a)

        source_node: GmodNode = gmod[input_code]
        if location:
            source_node = source_node.with_location(location)

        expected_node = target_gmod[expected_code]
        if location:
            expected_node = expected_node.with_location(location)

        target_node: GmodNode | None = self.vis.convert_node(
            gmod.vis_version, source_node, target_gmod.vis_version
        )

        # Assertions
        assert source_node is not None, "Source node should not be None"
        assert expected_node is not None, "Expected node should not be None"
        assert target_node is not None, "Target node should not be None"

        assert expected_node.code == target_node.code, (
            f"Expected code {expected_node.code} but got {target_node.code}"
        )
        assert expected_node.location == target_node.location, (
            f"Expected location {expected_node.location} but got {target_node.location}"
        )
        assert expected_node == target_node, (
            f"Expected node {expected_node} does not match target node {target_node}"
        )

    @pytest.mark.parametrize(
        ("source_local_id_str", "target_local_id_str"),
        [
            (
                "/dnv-v2/vis-3-4a/411.1/C101/sec/411.1/C101.64i/S201/meta/cnt-condensate",
                "/dnv-v2/vis-3-5a/411.1/C101/sec/411.1/C101.64/S201/meta/cnt-condensate",
            ),
            (
                "/dnv-v2/vis-3-5a/511.11/C101/sec/621.3/I101/meta/qty-time/detail-hfo.to.gas",
                "/dnv-v2/vis-3-6a/511.11/C101/sec/621.3/I101/meta/qty-time/detail-hfo.to.gas",
            ),
        ],
    )
    def test_convert_local_id(
        self, source_local_id_str: str, target_local_id_str: str
    ) -> None:
        """Test converting local IDs between different VIS versions."""
        source_local_id = LocalIdBuilder.parse(source_local_id_str).build()
        target_local_id = LocalIdBuilder.parse(target_local_id_str).build()

        target_version = target_local_id.vis_version
        if target_version is None:
            pytest.fail("Target local ID must have a valid VIS version")
            return  # This should never be reached, but helps type checker

        # Convert the source LocalId to the target version
        converted_local_id = self.vis.convert_local_id(source_local_id, target_version)

        assert converted_local_id is not None, "Converted local ID should not be None"
        assert converted_local_id == target_local_id, (
            "Converted local ID does not match expected target local ID"
        )

    @pytest.mark.parametrize("item", TestData.get_valid_gmod_path_data())
    def test_valid_gmod_path_to_latest(self, item: GmodPathTestItem) -> None:
        """Test converting valid GMOD paths to the latest version."""
        source_version = VisVersions.parse(item.vis_version)
        source_path = GmodPath.parse(item.path, source_version)
        target_version = self.vis.latest_vis_version
        target_path = self.vis.convert_path(source_version, source_path, target_version)
        assert target_path is not None, "Target path should not be None"

    @pytest.mark.parametrize(
        ("source", "target", "source_version", "target_version"),
        [
            (
                "691.811i-A/H101.11-1",
                "691.83111i-A/H101.11-1",
                VisVersion.v3_7a,
                VisVersion.v3_9a,
            )
        ],
    )
    def test_convert_gmod_path_with_location(
        self,
        source: str,
        target: str,
        source_version: VisVersion,
        target_version: VisVersion,
    ) -> None:
        """Test converting GMOD paths with locations."""
        source_path = GmodPath.parse(source, source_version)
        converted_path = self.vis.convert_path(
            source_version, source_path, target_version
        )
        assert converted_path is not None, "Target path should not be None"
        assert str(converted_path) == target, (
            f"Expected target path {target}, got {converted_path}"
        )

    def test_one_path_to_root_for_asset_functions(self) -> None:
        """Test that all asset function nodes have exactly one path to root."""

        def one_path_to_root(node) -> bool:  # noqa : ANN001
            return node[1].is_root or (
                len(node.parents) == 1 and one_path_to_root(node.parents[0])
            )

        for version in VisVersions.all_versions():
            gmod = self.vis.get_gmod(version)
            for node in gmod._node_map:
                if (
                    not hasattr(node[1], "is_asset_function_node")
                    or not node[1].is_asset_function_node()
                ):
                    continue

                assert one_path_to_root(node), (
                    f"Node {node[1].code} in {version} doesn't have one path to root",
                )

    def test_c101_22_to_c101_93_conversion(self) -> None:
        """Test the specific conversion from C101.22 to C101.93."""
        source_version = VisVersion.v3_4a
        target_version = VisVersion.v3_6a

        # Get the source and target nodes
        source_gmod = self.vis.get_gmod(source_version)

        source_node = source_gmod["C101.22"]

        # Convert the node
        target_node = self.vis.convert_node(source_version, source_node, target_version)

        # Assert the conversion was successful
        assert target_node is not None, "Target node should not be None"
        expected_code = "C101.93"
        assert target_node.code == expected_code, (
            f"Expected {expected_code}, got {target_node.code}"
        )
