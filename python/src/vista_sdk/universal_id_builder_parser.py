"""This module defines the UniversalIdBuilder parsing functionality."""

from __future__ import annotations

from vista_sdk.imo_number import ImoNumber
from vista_sdk.internal.local_id_parsing_error_builder import LocalIdParsingErrorBuilder
from vista_sdk.internal.local_id_parsing_state import LocalIdParsingState
from vista_sdk.local_id_builder_parsing import LocalIdBuilderParsing
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.universal_id_builder import UniversalIdBuilder


class UniversalIdBuilderParser:
    """Parser for Universal ID strings."""

    @staticmethod
    def parse(universal_id_str: str) -> UniversalIdBuilder:
        """Parse a string into a UniversalIdBuilder.

        Args:
            universal_id_str: String representation of the Universal ID

        Returns:
            UniversalIdBuilder instance

        Raises:
            ValueError: If the string cannot be parsed as a valid Universal ID
        """
        success, errors, builder = UniversalIdBuilderParser.try_parse_with_errors(
            universal_id_str
        )
        if not success or builder is None:
            raise ValueError(
                f"Couldn't parse universal ID from: '{universal_id_str}'. {errors}"
            )
        return builder

    @staticmethod
    def try_parse(universal_id: str) -> tuple[bool, UniversalIdBuilder | None]:
        """Try to parse a string into a UniversalIdBuilder.

        Args:
            universal_id: String representation of the Universal ID

        Returns:
            A tuple of (success, builder)
        """
        success, _, builder = UniversalIdBuilderParser.try_parse_with_errors(
            universal_id
        )
        return success, builder

    @staticmethod
    def try_parse_with_errors(
        universal_id: str,
    ) -> tuple[bool, ParsingErrors, UniversalIdBuilder | None]:
        """Try to parse a string with detailed error information.

        Args:
            universal_id: String representation of the Universal ID

        Returns:
            A tuple of (success, errors, builder)
        """
        error_builder = LocalIdParsingErrorBuilder.empty()

        # Basic validation
        if universal_id is None or len(universal_id) == 0:
            UniversalIdBuilderParser._add_error(
                error_builder,
                LocalIdParsingState.NAMING_RULE,
                "Failed to find localId start segment",
            )
            return False, error_builder.build(), None

        # Find the local ID start segment
        local_id_start_index = universal_id.find("/dnv-v")
        if local_id_start_index == -1:
            UniversalIdBuilderParser._add_error(
                error_builder,
                LocalIdParsingState.NAMING_RULE,
                "Failed to find localId start segment",
            )
            return False, error_builder.build(), None

        universal_id_segment = universal_id[:local_id_start_index]
        local_id_segment = universal_id[local_id_start_index:]

        # Parse local ID
        parser = LocalIdBuilderParsing()
        try:
            success, local_id_builder = parser._try_parse_internal(
                local_id_segment, error_builder
            )
        except Exception:
            local_id_builder = None
            success = False

        if not success or local_id_builder is None:
            return False, error_builder.build(), None

        # Parse universal segments
        imo_number = UniversalIdBuilderParser._parse_universal_id_segments(
            universal_id_segment, error_builder
        )

        # Build result
        try:
            vis_version = local_id_builder.vis_version
            if vis_version is None:
                UniversalIdBuilderParser._add_error(
                    error_builder,
                    LocalIdParsingState.VIS_VERSION,
                    "VIS version not found in local ID",
                )
                return False, error_builder.build(), None

            builder = UniversalIdBuilder.create(vis_version)

            if imo_number is not None:
                builder = builder.try_with_imo_number_simple(imo_number)

            builder = builder.try_with_local_id_simple(local_id_builder)

            return True, error_builder.build(), builder
        except Exception as e:
            UniversalIdBuilderParser._add_error(
                error_builder,
                LocalIdParsingState.COMPLETENESS,
                f"Failed to build UniversalIdBuilder: {e}",
            )
            return False, error_builder.build(), None

    @staticmethod
    def _parse_universal_id_segments(
        universal_id_segment: str, error_builder: LocalIdParsingErrorBuilder
    ) -> ImoNumber | None:
        """Parse universal ID segments using state machine."""
        state = LocalIdParsingState.NAMING_ENTITY
        i = 0
        imo_number: ImoNumber | None = None

        while state <= LocalIdParsingState.IMO_NUMBER:
            if i >= len(universal_id_segment):
                break  # We've consumed the string

            # Find next slash or take rest of string
            next_slash = universal_id_segment.find("/", i)
            if next_slash == -1:
                segment = universal_id_segment[i:]
            else:
                segment = universal_id_segment[i:next_slash]

            if state == LocalIdParsingState.NAMING_ENTITY:
                if segment != UniversalIdBuilder.NAMING_ENTITY:
                    UniversalIdBuilderParser._add_error(
                        error_builder,
                        state,
                        f"Naming entity segment didn't match. Found: {segment}",
                    )
                    return None
                state = LocalIdParsingState.IMO_NUMBER
            elif state == LocalIdParsingState.IMO_NUMBER:
                imo_parsed = ImoNumber.try_parse(segment)
                if imo_parsed is None:
                    UniversalIdBuilderParser._add_error(
                        error_builder, state, "Invalid IMO number segment"
                    )
                    return None
                imo_number = imo_parsed
                break  # Done parsing

            i += len(segment) + 1  # +1 for the slash

        return imo_number

    @staticmethod
    def _add_error(
        error_builder: LocalIdParsingErrorBuilder,
        state: LocalIdParsingState,
        message: str | None,
    ) -> None:
        """Add an error to the error builder.

        Args:
            error_builder: The error builder to add to
            state: The parsing state where the error occurred
            message: The error message
        """
        # Note: In Python we modify the error_builder in place, unlike C# ref parameter
        error_builder.add_error(state, message)
