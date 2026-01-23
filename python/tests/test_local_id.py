"""Tests for the LocalId class and related parsing functionality."""

from dataclasses import dataclass
from pathlib import Path

import pytest

from tests.testdata import TestData
from vista_sdk.codebook_names import CodebookName
from vista_sdk.internal.location_parsing_error_builder import LocationValidationResult
from vista_sdk.local_id import LocalId
from vista_sdk.local_id_builder import LocalIdBuilder
from vista_sdk.mqtt.mqtt_local_id import MqttLocalId
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.vis import VIS
from vista_sdk.vis_version import VisVersion


class TestParsingErrors:
    """Tests for the ParsingErrors class."""

    def test_comparisons(self) -> None:
        """Test comparison operators for ParsingErrors."""
        errors1 = [(LocationValidationResult.INVALID.name, "M1")]
        errors2 = [
            (LocationValidationResult.INVALID.name, "M1"),
            (LocationValidationResult.INVALID_CODE.name, "M1"),
        ]

        e1 = ParsingErrors(errors1)
        e2 = ParsingErrors(errors1)
        e3 = ParsingErrors(errors2)
        e4 = ParsingErrors.empty()

        assert e1 == e2
        assert e1 == e1
        assert e1 is not None
        assert e1 != e4

        assert e1 != e3
        assert e4 == ParsingErrors.empty()
        assert e4.__eq__(ParsingErrors.empty())
        assert e4.__eq__(ParsingErrors.empty())

    def test_enumerator(self) -> None:
        """Test enumeration of ParsingErrors."""
        errors1 = [(LocationValidationResult.INVALID.name, "M1")]
        errors2 = [
            (LocationValidationResult.INVALID.name, "M1"),
            (LocationValidationResult.INVALID_CODE.name, "M1"),
        ]

        e1 = ParsingErrors(errors1)
        e2 = ParsingErrors(errors2)
        e3 = ParsingErrors.empty()

        assert len(errors1) == len(list(e1))
        assert len(errors2) == len(list(e2))
        assert len(list(e3)) == 0


@dataclass
class Input:
    """Input data for local ID testing."""

    primary_item: str
    secondary_item: str | None = None
    quantity: str | None = None
    content: str | None = None
    position: str | None = None
    vis_version: VisVersion = VisVersion.v3_4a
    verbose: bool = False


class TestLocalId:
    """Tests for the LocalId class and related functionality."""

    @pytest.mark.parametrize(
        ("input_data", "expected_output"),
        [
            (Input("411.1/C101.31-2"), "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta"),
            (
                Input("411.1/C101.31-2", None, "temperature", "exhaust.gas", "inlet"),
                "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            ),
            (
                Input(
                    "411.1/C101.63/S206",
                    None,
                    "temperature",
                    "exhaust.gas",
                    "inlet",
                    verbose=True,
                ),
                "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            ),
            (
                Input(
                    "411.1/C101.63/S206",
                    "411.1/C101.31-5",
                    "temperature",
                    "exhaust.gas",
                    "inlet",
                    verbose=True,
                ),
                "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            ),
            (
                Input(
                    primary_item="511.11/C101.67/S208",
                    quantity="pressure",
                    position="inlet",
                    content="starting.air",
                    verbose=True,
                    vis_version=VisVersion.v3_6a,
                ),
                "/dnv-v2/vis-3-6a/511.11/C101.67/S208/~main.generator.engine/~starting.system.pneumatic/meta/qty-pressure/cnt-starting.air/pos-inlet",
            ),
        ],
    )
    def test_local_id_build_valid(
        self, input_data: Input, expected_output: str
    ) -> None:
        """Test building valid LocalId objects."""
        vis = VIS()

        vis_version = input_data.vis_version

        gmod = vis.get_gmod(vis_version)
        codebooks = vis.get_codebooks(vis_version)

        primary_item = gmod.parse_path(input_data.primary_item)
        secondary_item = (
            gmod.parse_path(input_data.secondary_item)
            if input_data.secondary_item is not None
            else None
        )

        local_id = (
            LocalIdBuilder.create(vis_version)
            .with_primary_item(primary_item)
            .try_with_secondary_item(secondary_item)
            .with_verbose_mode(input_data.verbose)
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Quantity, input_data.quantity)
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Content, input_data.content)
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Position, input_data.position)
            )
        )

        local_id_str = str(local_id)

        assert expected_output == local_id_str

    def test_local_id_build_all_without(self) -> None:
        """Test removing all components from a LocalId."""
        vis = VIS()

        vis_version = VisVersion.v3_4a

        gmod = vis.get_gmod(vis_version)
        codebooks = vis.get_codebooks(vis_version)

        primary_item = gmod.parse_path("411.1/C101.31-2")
        secondary_item = gmod.parse_path("411.1/C101.31-5")

        # Create the builder step by step to handle Python's tuple returns
        local_id = (
            LocalIdBuilder.create(vis_version)
            .with_primary_item(primary_item)
            .try_with_secondary_item(secondary_item)
            .with_verbose_mode(True)
            .try_with_metadata_tag(
                codebooks.create_tag(CodebookName.Quantity, "quantity")
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Content, "content")
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Position, "position")
            )
            .try_with_metadata_tag(codebooks.create_tag(CodebookName.State, "state"))
            .try_with_metadata_tag(
                codebooks.create_tag(CodebookName.Content, "content")
            )
            .try_with_metadata_tag(
                codebooks.create_tag(CodebookName.Calculation, "calculate")
            )
        )

        assert local_id.is_valid

        all_without = (
            local_id.without_primary_item()
            .without_secondary_item()
            .without_quantity()
            .without_position()
            .without_state()
            .without_content()
            .without_calculation()
        )

        assert all_without.is_empty

    @pytest.mark.parametrize(
        ("input_data", "expected_output"),
        [
            (
                Input("411.1/C101.31-2", None, "temperature", "exhaust.gas", "inlet"),
                "dnv-v2/vis-3-4a/411.1_C101.31-2/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            ),
            (
                Input(
                    "411.1/C101.63/S206", None, "temperature", "exhaust.gas", "inlet"
                ),
                "dnv-v2/vis-3-4a/411.1_C101.63_S206/_/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            ),
            (
                Input(
                    "411.1/C101.63/S206",
                    "411.1/C101.31-5",
                    "temperature",
                    "exhaust.gas",
                    "inlet",
                ),
                "dnv-v2/vis-3-4a/411.1_C101.63_S206/411.1_C101.31-5/qty-temperature/cnt-exhaust.gas/_/_/_/_/pos-inlet/_",
            ),
        ],
    )
    def test_mqtt_local_id_build_valid(
        self, input_data: Input, expected_output: str
    ) -> None:
        """Test building valid MQTT LocalId objects."""
        vis = VIS()

        vis_version = VisVersion.v3_4a

        gmod = vis.get_gmod(vis_version)
        codebooks = vis.get_codebooks(vis_version)

        primary_item = gmod.parse_path(input_data.primary_item)
        secondary_item = (
            gmod.parse_path(input_data.secondary_item)
            if input_data.secondary_item is not None
            else None
        )

        # Create the builder step by step to handle Python's tuple returns
        builder = (
            LocalIdBuilder.create(vis_version)
            .try_with_primary_item(primary_item)
            .try_with_secondary_item(secondary_item)
            .with_verbose_mode(input_data.verbose)
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Quantity, input_data.quantity)
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Content, input_data.content)
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Position, input_data.position)
            )
        )

        # Build an MQTT local ID
        mqtt_local_id = MqttLocalId(builder)
        local_id_str = str(mqtt_local_id)
        assert expected_output == local_id_str

    @pytest.mark.parametrize(
        "input_data",
        [
            Input("411.1/C101.31-2"),
            Input("411.1/C101.31-2", None, "temperature", "exhaust.gas", "inlet"),
            Input(
                "411.1/C101.63/S206",
                None,
                "temperature",
                "exhaust.gas",
                "inlet",
                verbose=True,
            ),
        ],
    )
    def test_local_id_equality(self, input_data: Input) -> None:
        """Test equality comparisons for LocalId objects."""
        vis = VIS()

        vis_version = VisVersion.v3_4a

        gmod = vis.get_gmod(vis_version)
        codebooks = vis.get_codebooks(vis_version)

        primary_item = gmod.parse_path(input_data.primary_item)
        secondary_item = (
            gmod.parse_path(input_data.secondary_item)
            if input_data.secondary_item is not None
            else None
        )

        # Create the builder step by step to handle Python's tuple returns
        local_id = (
            LocalIdBuilder.create(vis_version)
            .with_primary_item(primary_item)
            .try_with_secondary_item(secondary_item)
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Quantity, input_data.quantity)
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Content, input_data.content)
            )
            .try_with_metadata_tag(
                codebooks.try_create_tag(CodebookName.Position, input_data.position)
            )
        )

        other_local_id = local_id
        assert local_id == other_local_id
        assert local_id is other_local_id

        # Create a copy
        other_local_id = local_id.with_vis_version(vis_version)

        assert local_id == other_local_id
        assert local_id is not other_local_id

        # Modify the copy
        other_local_id = other_local_id.with_metadata_tag(
            codebooks.create_tag(CodebookName.Position, "eqtestvalue")
        )
        assert local_id != other_local_id
        assert local_id is not other_local_id

        # Create a structurally equivalent builder
        other_local_id = local_id.try_with_primary_item(
            local_id.primary_item
        ).try_with_metadata_tag(  # Extract just the builder from the tuple
            codebooks.try_create_tag(
                CodebookName.Position,
                local_id.position.value if local_id.position else None,
            )
        )

        assert local_id == other_local_id
        assert local_id is not other_local_id

    @pytest.mark.parametrize(
        "local_id_str",
        [
            "/dnv-v2/vis-3-4a/1031/meta/cnt-refrigerant/state-leaking",
            "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
            "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
            "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/411.1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
            "/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low",
        ],
    )
    def test_parsing(self, local_id_str: str) -> None:
        """Test parsing LocalId from strings."""
        parsed, _, local_id = LocalId.try_parse(local_id_str)

        assert parsed
        assert local_id_str == str(local_id)

    def test_simple_parse(self) -> None:
        """Test simple parsing of LocalId."""
        local_id_as_string = "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet"  # noqa: E501
        local_id = LocalId.parse(local_id_as_string)
        assert local_id is not None

    def test_smoke_test_parsing(self) -> None:
        """Test parsing a large number of LocalIds from a file."""
        path = Path(__file__).parent / "testdata" / "LocalIds.txt"
        with path.open() as file:
            errored: list[
                tuple[str, LocalId | None, Exception | None, ParsingErrors]
            ] = []

            for line in file:
                local_id_str = line.strip()
                if not local_id_str:
                    continue

                try:
                    # Skip invalid metadata tags "qty-content"
                    if "qty-content" in local_id_str:
                        continue

                    result = LocalId.try_parse(local_id_str)
                    parsed, error_builder, local_id = result

                    if not parsed or local_id is None:
                        errored.append((local_id_str, local_id, None, error_builder))
                except Exception as ex:
                    # Skip invalid location e.g. primaryItem 511.11-1SO
                    if "location" in str(ex):
                        continue
                    errored.append((local_id_str, None, ex, ParsingErrors.empty()))

            if any(e[3].has_errors for e in errored):
                # For debugging: print errors
                pass

            # Check no parsing errors
            assert not any(error for item in errored for error in item[3])
            assert not errored

    @pytest.mark.parametrize(
        ("local_id_str", "expected_error_messages"),
        [
            (item.local_id_str, item.expected_error_messages)
            for item in TestData.get_local_id_data("InvalidLocalIds").invalid_local_ids
        ],
    )
    def test_parsing_validation(
        self, local_id_str: str, expected_error_messages: list[str]
    ) -> None:
        """Test validation during parsing using test data from JSON file."""
        parsed, error_builder, _ = LocalId.try_parse(local_id_str)

        actual_error_messages = [message for _, message in error_builder]
        assert sorted(actual_error_messages) == sorted(expected_error_messages)

        assert not parsed
        assert error_builder is not None
