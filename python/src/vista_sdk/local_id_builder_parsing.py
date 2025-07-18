"""LocalId parsing functionality for the LocalIdBuilder class.

This module provides parsing functionality for the LocalIdBuilder class,
implementing the state machine logic for parsing LocalId strings.
"""

from __future__ import annotations

import contextlib

from vista_sdk.codebook_names import CodebookName
from vista_sdk.codebooks import Codebooks
from vista_sdk.internal.local_id_parsing_error_builder import LocalIdParsingErrorBuilder
from vista_sdk.internal.local_id_parsing_state import LocalIdParsingState
from vista_sdk.local_id_builder import LocalIdBuilder
from vista_sdk.metadata_tag import MetadataTag
from vista_sdk.parsing_errors import ParsingErrors
from vista_sdk.vis_version import VisVersions


class LocalIdBuilderParsing:
    """Class for parsing LocalId strings into LocalIdBuilder objects."""

    def __init__(self) -> None:
        """Initialize the LocalIdBuilderParsing class."""
        from vista_sdk.vis import VIS

        self.vis = VIS()

    def parse(self, local_id_str: str) -> LocalIdBuilder:  # noqa: C901
        """Parse a string into a LocalIdBuilder.

        Args:
            local_id_str: The string to parse

        Returns:
            A LocalIdBuilder containing the parsed information

        Raises:
            ValueError: If parsing fails
        """
        success, errors, builder = self.try_parse(local_id_str)
        if not success or builder is None:
            # First, check for explicit errors
            if errors.has_errors:
                error_message = str(errors)
            else:
                # Custom error analysis for the specific path
                segments = local_id_str.split("/")
                # Try to find metadata tags to analyze
                meta_idx = -1
                with contextlib.suppress(ValueError):
                    meta_idx = segments.index("meta")

                # Specific analysis
                if len(segments) < 2:
                    error_message = "Invalid path format, missing required segments"
                elif segments[0] != LocalIdBuilder.NAMING_RULE:
                    error_message = (
                        f"Invalid naming rule, expected '{LocalIdBuilder.NAMING_RULE}'"
                    )
                elif not segments[1].startswith("vis-"):
                    error_message = "Invalid VIS version format, must start with 'vis-'"
                elif meta_idx == -1:
                    error_message = "Missing 'meta' segment in path"
                elif meta_idx < len(segments) - 1:
                    # Extract the tag prefix and value
                    meta_tag = segments[meta_idx + 1]
                    if "-" in meta_tag:
                        prefix, value = meta_tag.split("-", 1)
                        error_message = (
                            f"Invalid metadata tag value '{value}'"
                            f"for prefix '{prefix}'."
                            " Tag may not exist in codebook or"
                            " is incompatible with the path"
                        )
                    else:
                        error_message = f"Invalid metadata tag format '{meta_tag}'"
                else:
                    # Check if any path elements might be invalid
                    for i, segment in enumerate(segments):
                        if i < meta_idx and segment.count(".") > 0:
                            # This could be an invalid path element
                            error_message = (
                                f"Path element '{segment}' may be invalid or not "
                                "defined in the GMOD hierarchy"
                            )
                            break
                    else:
                        # If we didn't find a specific issue
                        error_message = (
                            "Unknown parsing failure, possibly an invalid path "
                            "structure or incompatible metadata combination"
                        )

            raise ValueError(
                f"Couldn't parse local ID from: '{local_id_str}'. {error_message}"
            )
        return builder

    def try_parse(
        self, local_id_str: str
    ) -> tuple[bool, ParsingErrors, LocalIdBuilder | None]:
        """Try to parse a string into a LocalIdBuilder.

        Args:
            local_id_str: The string to parse

        Returns:
            A tuple containing:
            - A boolean indicating whether parsing succeeded
            - The parsing errors encountered
            - The resulting LocalIdBuilder, or None if parsing failed
        """
        error_builder = LocalIdParsingErrorBuilder.empty()
        success, local_id = self.try_parse_internal(local_id_str, error_builder)
        errors = error_builder.build()
        return success, errors, local_id

    def try_parse_internal(  # noqa : C901
        self, local_id_str: str, error_builder: LocalIdParsingErrorBuilder
    ) -> tuple[bool, LocalIdBuilder | None]:
        """Internal implementation of try_parse.

        Args:
            local_id_str: The string to parse
            error_builder: Builder to collect errors

        Returns:
            A tuple containing:
            - A boolean indicating whether parsing succeeded
            - The resulting LocalIdBuilder, or None if parsing failed
        """
        if local_id_str is None:
            raise ValueError("local_id_str cannot be None")

        if len(local_id_str) == 0:
            return False, None

        if local_id_str[0] != "/":
            self.add_error(
                error_builder,
                LocalIdParsingState.FORMATTING,
                "Invalid format: missing '/' as first character",
            )
            return False, None

        # Use Python equivalent of span
        span = local_id_str

        vis_version = None
        gmod = None
        codebooks = None
        primary_item = None
        secondary_item = None
        qty = None
        cnt = None
        calc = None
        state_tag = None
        cmd = None
        type_tag = None
        pos = None
        detail = None
        predefined_message = None
        invalid_secondary_item = False
        verbose = False  # For builder.with_verbose_mode

        primary_item_start = -1
        secondary_item_start = -1

        state = LocalIdParsingState.NAMING_RULE
        i = 1

        while state <= LocalIdParsingState.META_DETAIL:
            next_start = min(len(span), i)
            next_slash_pos = span.find("/", next_start)
            if next_slash_pos == -1:
                segment = span[next_start:]
            else:
                segment = span[next_start:next_slash_pos]

            if state == LocalIdParsingState.NAMING_RULE:
                if len(segment) == 0:
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.NAMING_RULE,
                        predefined_message,
                    )
                    state = self.advance_state(state)
                    continue

                if segment != LocalIdBuilder.NAMING_RULE:
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.NAMING_RULE,
                        predefined_message,
                    )
                    return False, None

                i, state = self.advance_parser(i, segment, state)

            elif state == LocalIdParsingState.VIS_VERSION:
                if len(segment) == 0:
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.VIS_VERSION,
                        predefined_message,
                    )
                    state = self.advance_state(state)
                    continue

                if not segment.startswith("vis-"):
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.VIS_VERSION,
                        predefined_message,
                    )
                    return False, None

                version_str = segment[4:]  # Remove 'vis-' prefix
                vis_version = VisVersions.try_parse(version_str)
                if vis_version is None:
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.VIS_VERSION,
                        predefined_message,
                    )
                    return False, None

                gmod = self.vis.get_gmod(vis_version)
                codebooks = self.vis.get_codebooks(vis_version)

                if gmod is None or codebooks is None:
                    return False, None

                i, state = self.advance_parser(i, segment, state)

            elif state == LocalIdParsingState.PRIMARY_ITEM:
                if len(segment) == 0:
                    if primary_item_start != -1:
                        if gmod is None:
                            return False, None

                        path = span[primary_item_start : i - 1]
                        res = gmod.try_parse_path(path)
                        if res is not None:
                            success, primary_item = res
                            if not success or primary_item is None:
                                self.add_error(
                                    error_builder,
                                    LocalIdParsingState.PRIMARY_ITEM,
                                    f"Invalid GmodPath in Primary item: {path}",
                                )
                    else:
                        self.add_error(
                            error_builder,
                            LocalIdParsingState.PRIMARY_ITEM,
                            predefined_message,
                        )

                    self.add_error(
                        error_builder,
                        LocalIdParsingState.PRIMARY_ITEM,
                        "Invalid or missing '/meta' prefix after Primary item",
                    )
                    state = self.advance_state(state)
                    continue

                dash_index = segment.find("-")
                code = segment if dash_index == -1 else segment[:dash_index]

                if gmod is None:
                    return False, None

                if primary_item_start == -1:
                    # Only validate the first node, as subsequent path
                    # validation will handle complex paths
                    success, _ = gmod.try_get_node(code)
                    if not success:
                        self.add_error(
                            error_builder,
                            LocalIdParsingState.PRIMARY_ITEM,
                            f"Invalid start GmodNode in Primary item: {code}",
                        )
                    primary_item_start = i
                    i = self.advance_position(i, segment)
                else:
                    # Determine next state based on segment prefixes
                    is_sec = segment.startswith("sec")
                    is_meta = segment.startswith("meta")
                    is_tilde = segment.startswith("~") if len(segment) > 0 else False

                    if is_sec:
                        next_state = LocalIdParsingState.SECONDARY_ITEM
                    elif is_meta:
                        next_state = LocalIdParsingState.META_QUANTITY
                    elif is_tilde:
                        next_state = LocalIdParsingState.ITEM_DESCRIPTION
                    else:
                        next_state = state

                    if next_state != state:
                        # Try to parse the accumulated path
                        path = span[primary_item_start : i - 1]
                        res = gmod.try_parse_path(path)
                        if res is not None:
                            success, primary_item = res
                            if not success or primary_item is None:
                                # Displays the full GmodPath when first part of
                                # PrimaryItem is invalid
                                self.add_error(
                                    error_builder,
                                    LocalIdParsingState.PRIMARY_ITEM,
                                    f"Invalid GmodPath in Primary item: {path}",
                                )

                        if is_tilde:
                            state = next_state
                        else:
                            i, state = self.advance_parser(
                                i, segment, to_state=next_state
                            )
                        continue

                    # Check individual node validity FIRST (continuing in primary item)
                    success, _ = gmod.try_get_node(code)
                    if not success:
                        self.add_error(
                            error_builder,
                            LocalIdParsingState.PRIMARY_ITEM,
                            f"Invalid GmodNode in Primary item: {code}",
                        )

                        next_state_indexes = self.get_next_state_indexes(span, state)
                        next_state_index, end_of_next_state_index = next_state_indexes

                        if next_state_index == -1:
                            self.add_error(
                                error_builder,
                                LocalIdParsingState.PRIMARY_ITEM,
                                "Invalid or missing '/meta' prefix after Primary item",
                            )
                            return False, None

                        next_segment = span[next_state_index + 1 :]
                        if next_segment.startswith("sec"):
                            next_state = LocalIdParsingState.SECONDARY_ITEM
                        elif next_segment.startswith("meta"):
                            next_state = LocalIdParsingState.META_QUANTITY
                        elif next_segment.startswith("~"):
                            next_state = LocalIdParsingState.ITEM_DESCRIPTION
                        else:
                            raise ValueError("Inconsistent parsing state")

                        # Displays the invalid middle parts of PrimaryItem and not
                        # the whole GmodPath
                        invalid_primary_item_path = span[i:next_state_index]

                        self.add_error(
                            error_builder,
                            LocalIdParsingState.PRIMARY_ITEM,
                            f"Invalid GmodPath: Last part in Primary item: "
                            f"{invalid_primary_item_path}",
                        )

                        i = end_of_next_state_index
                        state = next_state
                        continue

                    # Always advance position, continue processing subsequent segments
                    i = self.advance_position(i, segment)

            elif state == LocalIdParsingState.SECONDARY_ITEM:
                if len(segment) == 0:
                    # Don't automatically advance state if we had
                    # invalid secondary items. This allows the
                    # end logic to detect missing '/meta' prefix
                    if invalid_secondary_item:
                        break
                    state = self.advance_state(state)
                    continue

                dash_index = segment.find("-")
                code = segment if dash_index == -1 else segment[:dash_index]

                if gmod is None:
                    return False, None

                if secondary_item_start == -1:
                    # First segment in secondary item - check if start node is valid
                    success, _ = gmod.try_get_node(code)
                    if not success:
                        self.add_error(
                            error_builder,
                            LocalIdParsingState.SECONDARY_ITEM,
                            f"Invalid start GmodNode in Secondary item: {code}",
                        )

                    secondary_item_start = i
                    i = self.advance_position(i, segment)
                    continue

                # Handle subsequent segments or state transitions
                # Determine next state based on segment prefixes
                is_meta = segment.startswith("meta")
                is_tilde = segment.startswith("~") if len(segment) > 0 else False

                if is_meta:
                    next_state = LocalIdParsingState.META_QUANTITY
                elif is_tilde:
                    next_state = LocalIdParsingState.ITEM_DESCRIPTION
                else:
                    next_state = state

                if next_state != state:
                    # Transitioning to next state - validate the secondary item path
                    path = span[secondary_item_start : i - 1]
                    res = gmod.try_parse_path(path)
                    if res is not None:
                        success, secondary_item = res
                        if not success or secondary_item is None:
                            # display the full GmodPath when first part of
                            # SecondaryItem is invalid
                            invalid_secondary_item = True
                            self.add_error(
                                error_builder,
                                LocalIdParsingState.SECONDARY_ITEM,
                                f"Invalid GmodPath in Secondary item: {path}",
                            )

                            next_start_indexes = self.get_next_state_indexes(
                                span, state
                            )
                            if next_start_indexes[1] != -1:
                                i = next_start_indexes[1]
                                state = next_state
                                continue

                    if is_tilde:
                        state = next_state
                    else:
                        i, state = self.advance_parser(i, segment, to_state=next_state)
                    continue

                # Continuing in secondary item state - validate individual nodes
                success, _ = gmod.try_get_node(code)
                if not success:
                    invalid_secondary_item = True
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.SECONDARY_ITEM,
                        f"Invalid GmodNode in Secondary item: {code}",
                    )

                    # Check if we need to report "Last part" error and skip to
                    # next state
                    next_start_indexes = self.get_next_state_indexes(span, state)
                    if next_start_indexes[0] == -1:
                        # Missing /meta prefix case
                        self.add_error(
                            error_builder,
                            LocalIdParsingState.SECONDARY_ITEM,
                            "Invalid or missing '/meta' prefix after Secondary item",
                        )
                        return False, None

                    # Report "Last part" error for invalid segments
                    invalid_path = span[i : next_start_indexes[0]]
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.SECONDARY_ITEM,
                        f"Invalid GmodPath: Last part in Secondary item: "
                        f"{invalid_path}",
                    )

                    # Determine next state based on what comes next
                    if next_start_indexes[0] + 1 < len(span):
                        next_segment = span[next_start_indexes[0] + 1 :]
                    else:
                        next_segment = ""
                    if next_segment.startswith("meta"):
                        next_state = LocalIdParsingState.META_QUANTITY
                    elif next_segment.startswith("~"):
                        next_state = LocalIdParsingState.ITEM_DESCRIPTION
                    else:
                        raise ValueError("Inconsistent parsing state")

                    i = next_start_indexes[1]
                    state = next_state
                    continue

                # Always advance position, continue processing subsequent segments
                i = self.advance_position(i, segment)

            elif state == LocalIdParsingState.ITEM_DESCRIPTION:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                # Set verbose mode flag
                verbose = True

                meta_index = span.find("/meta")
                if meta_index == -1:
                    self.add_error(
                        error_builder,
                        LocalIdParsingState.ITEM_DESCRIPTION,
                        predefined_message,
                    )
                    return False, None

                segment = span[i : (meta_index + len("/meta"))]
                i, state = self.advance_parser(i, segment, state)

            elif state == LocalIdParsingState.META_QUANTITY:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Quantity,
                    state,
                    i,
                    segment,
                    qty,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, qty = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_CONTENT:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Content,
                    state,
                    i,
                    segment,
                    cnt,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, cnt = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_CALCULATION:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Calculation,
                    state,
                    i,
                    segment,
                    calc,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, calc = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_STATE:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.State,
                    state,
                    i,
                    segment,
                    state_tag,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, state_tag = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_COMMAND:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Command,
                    state,
                    i,
                    segment,
                    cmd,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, cmd = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_TYPE:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Type,
                    state,
                    i,
                    segment,
                    type_tag,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, type_tag = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_POSITION:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Position,
                    state,
                    i,
                    segment,
                    pos,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, pos = result[1], result[2], result[3]

            elif state == LocalIdParsingState.META_DETAIL:
                if len(segment) == 0:
                    state = self.advance_state(state)
                    continue

                result = self.parse_metatag(
                    CodebookName.Detail,
                    state,
                    i,
                    segment,
                    detail,
                    codebooks,
                    error_builder,
                )

                if not result[0]:
                    return False, None

                i, state, detail = result[1], result[2], result[3]

            else:
                i, state = self.advance_parser(i, segment, state)

        if vis_version is None:
            return False, None

        # Create a builder with the parsed components
        builder = LocalIdBuilder.create(vis_version)

        # Apply primary and secondary items
        if primary_item is not None:
            builder, success = builder.try_with_primary_item(primary_item)

        if secondary_item is not None:
            builder, success = builder.try_with_secondary_item(secondary_item)

        # Apply verbose mode
        builder = builder.with_verbose_mode(verbose)

        # Add metadata tags
        if qty is not None:
            builder, success = builder.try_with_metadata_tag(qty)
        if cnt is not None:
            builder, success = builder.try_with_metadata_tag(cnt)
        if calc is not None:
            builder, success = builder.try_with_metadata_tag(calc)
        if state_tag is not None:
            builder, success = builder.try_with_metadata_tag(state_tag)
        if cmd is not None:
            builder, success = builder.try_with_metadata_tag(cmd)
        if type_tag is not None:
            builder, success = builder.try_with_metadata_tag(type_tag)
        if pos is not None:
            builder, success = builder.try_with_metadata_tag(pos)
        if detail is not None:
            builder, success = builder.try_with_metadata_tag(detail)

        # Check for final validation errors
        if builder.is_empty_metadata:
            self.add_error(
                error_builder,
                LocalIdParsingState.COMPLETENESS,
                (
                    "No metadata tags specified. "
                    "Local IDs require atleast 1 metadata tag."
                ),
            )

        # Return success status and builder
        success = not error_builder.has_error and not invalid_secondary_item
        return success, builder if success else None

    def parse_metatag(  # noqa: C901
        self,
        codebook_name: CodebookName,
        state: LocalIdParsingState,
        i: int,
        segment: str,
        tag: MetadataTag | None,
        codebooks: Codebooks | None,
        error_builder: LocalIdParsingErrorBuilder,
    ) -> tuple[bool, int, LocalIdParsingState, MetadataTag | None]:
        """Parse a metadata tag from a segment.

        Args:
            codebook_name: The codebook name for the tag
            state: The current parsing state
            i: The current position in the string
            segment: The segment to parse
            tag: The existing tag, if any
            codebooks: The codebooks to use for parsing
            error_builder: The error builder to add errors to

        Returns:
            A tuple containing:
            - A boolean indicating whether parsing succeeded
            - The new position in the string
            - The new parsing state
            - The parsed tag, or None if parsing failed
        """
        from vista_sdk.vis import VIS

        if codebooks is None:
            return False, i, state, None

        dash_index = segment.find("-")
        tilde_index = segment.find("~")

        prefix_index = tilde_index if dash_index == -1 else dash_index

        if prefix_index == -1:
            self.add_error(
                error_builder,
                state,
                f"Invalid metadata tag: missing prefix '-' or '~' in {segment}",
            )
            i, state = self.advance_parser(i, segment, state)
            return True, i, state, tag

        actual_prefix = segment[:prefix_index]
        actual_state = self.meta_prefix_to_state(actual_prefix)

        if actual_state is None or actual_state < state:
            self.add_error(
                error_builder,
                state,
                f"Invalid metadata tag: unknown prefix {actual_prefix}",
            )
            return False, i, state, tag

        if actual_state > state:
            state = actual_state
            return True, i, state, tag

        next_state = self.next_parsing_state(actual_state)

        value = segment[prefix_index + 1 :]
        if len(value) == 0:
            self.add_error(
                error_builder,
                state,
                f"Invalid {codebook_name.name} metadata tag: missing value",
            )
            return False, i, state, tag

        # Determine if we're dealing with a custom tag (tilde syntax)
        is_custom_syntax = prefix_index == tilde_index

        # Handle custom tags (with tilde syntax)
        if is_custom_syntax:
            # For custom tags, only validate ISO string formatting
            if not VIS.is_iso_string(value):
                self.add_error(
                    error_builder,
                    state,
                    f"Invalid custom {codebook_name.name} metadata tag: "
                    f"not a valid ISO string: {value}",
                )
                i, state = self.advance_parser(i, segment, state)
                return False, i, state, tag

            # Create custom tag directly when using tilde syntax
            tag = MetadataTag(name=codebook_name, value=value, is_custom=True)
        else:
            # Normal tag creation through codebook validation
            tag = codebooks.try_create_tag(codebook_name, value)

        if tag is None:
            error_message = (
                (
                    f"Invalid custom {codebook_name.name} metadata tag: "
                    f"failed to create {value}"
                )
                if is_custom_syntax
                else (
                    f"Invalid {codebook_name.name} metadata tag: '{value}'. "
                    "Tag may not exist in codebook"
                )
            )

            self.add_error(error_builder, state, error_message)
            i, state = self.advance_parser(i, segment, state)
            return True, i, state, tag

        if prefix_index == dash_index and tag.prefix == "~":
            self.add_error(
                error_builder,
                state,
                f"Invalid {codebook_name.name} metadata tag: '{value}'. "
                f"Use prefix '~' for custom values",
            )

        if next_state is None:
            i, state = self.advance_parser(i, segment, state)
        else:
            i, state = self.advance_parser(i, segment, to_state=next_state)

        return True, i, state, tag

    def meta_prefix_to_state(self, prefix: str) -> LocalIdParsingState | None:
        """Convert a metadata prefix to a parsing state.

        Args:
            prefix: The metadata prefix

        Returns:
            The corresponding parsing state, or None if no match
        """
        if prefix == "qty":
            return LocalIdParsingState.META_QUANTITY
        if prefix == "cnt":
            return LocalIdParsingState.META_CONTENT
        if prefix == "calc":
            return LocalIdParsingState.META_CALCULATION
        if prefix == "state":
            return LocalIdParsingState.META_STATE
        if prefix == "cmd":
            return LocalIdParsingState.META_COMMAND
        if prefix == "type":
            return LocalIdParsingState.META_TYPE
        if prefix == "pos":
            return LocalIdParsingState.META_POSITION
        if prefix == "detail":
            return LocalIdParsingState.META_DETAIL

        return None

    def next_parsing_state(
        self, prev: LocalIdParsingState
    ) -> LocalIdParsingState | None:
        """Get the next parsing state after the given state.

        Args:
            prev: The previous parsing state

        Returns:
            The next parsing state, or None if there is no next state
        """
        if prev == LocalIdParsingState.META_QUANTITY:
            return LocalIdParsingState.META_CONTENT
        if prev == LocalIdParsingState.META_CONTENT:
            return LocalIdParsingState.META_CALCULATION
        if prev == LocalIdParsingState.META_CALCULATION:
            return LocalIdParsingState.META_STATE
        if prev == LocalIdParsingState.META_STATE:
            return LocalIdParsingState.META_COMMAND
        if prev == LocalIdParsingState.META_COMMAND:
            return LocalIdParsingState.META_TYPE
        if prev == LocalIdParsingState.META_TYPE:
            return LocalIdParsingState.META_POSITION
        if prev == LocalIdParsingState.META_POSITION:
            return LocalIdParsingState.META_DETAIL

        return None

    def add_error(
        self,
        error_builder: LocalIdParsingErrorBuilder,
        state: LocalIdParsingState,
        message: str | None,
    ) -> None:
        """Add an error to the error builder.

        Args:
            error_builder: The error builder to add the error to
            state: The parsing state where the error occurred
            message: The error message
        """
        # Add the error with the state and message
        error_builder.add_error(state, message)

    def get_next_state_indexes(
        self, span: str, state: LocalIdParsingState
    ) -> tuple[int, int]:
        """Get the indexes for the next state transition.

        Args:
            span: The input string
            state: The current parsing state

        Returns:
            A tuple of (next_index, end_of_next_state_index)
        """
        custom_index = span.find("~")
        end_of_custom_index = custom_index + len("~") + 1 if custom_index != -1 else -1

        meta_index = span.find("/meta")
        end_of_meta_index = meta_index + len("/meta") + 1 if meta_index != -1 else -1

        if state == LocalIdParsingState.PRIMARY_ITEM:
            sec_index = span.find("/sec")
            end_of_sec_index = sec_index + len("/sec") + 1 if sec_index != -1 else -1

            if sec_index != -1:
                return sec_index, end_of_sec_index

            if custom_index != -1:
                return custom_index, end_of_custom_index

            return meta_index, end_of_meta_index

        if state == LocalIdParsingState.SECONDARY_ITEM:
            if custom_index != -1:
                return custom_index, end_of_custom_index

            return meta_index, end_of_meta_index

        return meta_index, end_of_meta_index

    def advance_position(self, i: int, segment: str) -> int:
        """Advance the position in the string.

        Args:
            i: The current position
            segment: The current segment

        Returns:
            The new position
        """
        return i + len(segment) + 1

    def advance_state(self, state: LocalIdParsingState) -> LocalIdParsingState:
        """Advance to the next state.

        Args:
            state: The current state

        Returns:
            The next state
        """
        # Use the ordinal value of the enum to get the next state
        states = list(LocalIdParsingState)
        current_index = states.index(state)
        return states[current_index + 1]

    def advance_parser(
        self,
        i: int,
        segment: str,
        state: LocalIdParsingState | None = None,
        to_state: LocalIdParsingState | None = None,
    ) -> tuple[int, LocalIdParsingState]:
        """Advance both the position and the state.

        Args:
            i: The current position
            segment: The current segment
            state: The current state to advance from
            to_state: The target state to advance to (if provided)

        Returns:
            The new position and state
        """
        new_i = self.advance_position(i, segment)

        if to_state is not None:
            return new_i, to_state

        if state is not None:
            return new_i, self.advance_state(state)

        # Just advance position, keep same state - this should not happen
        raise ValueError("Cannot advance parser without a valid state")
