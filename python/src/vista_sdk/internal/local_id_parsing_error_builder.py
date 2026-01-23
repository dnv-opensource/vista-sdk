"""Error builder for Local ID parsing."""

from typing import ClassVar

from vista_sdk.internal.local_id_parsing_state import LocalIdParsingState
from vista_sdk.parsing_errors import ParsingErrors


class LocalIdParsingErrorBuilder:
    """Builder for collecting and managing parsing errors for Local IDs."""

    _predefined_error_messages: ClassVar[dict[LocalIdParsingState, str]] = {
        LocalIdParsingState.NAMING_RULE: "Missing or invalid naming rule",
        LocalIdParsingState.VIS_VERSION: "Missing or invalid vis version",
        LocalIdParsingState.PRIMARY_ITEM: (
            "Invalid or missing Primary item. Local IDs require atleast "
            "primary item and 1 metadata tag."
        ),
        LocalIdParsingState.SECONDARY_ITEM: "Invalid secondary item",
        LocalIdParsingState.ITEM_DESCRIPTION: "Missing or invalid /meta prefix",
        LocalIdParsingState.META_QUANTITY: "Invalid metadata tag: Quantity",
        LocalIdParsingState.META_CONTENT: "Invalid metadata tag: Content",
        LocalIdParsingState.META_COMMAND: "Invalid metadata tag: Command",
        LocalIdParsingState.META_POSITION: "Invalid metadata tag: Position",
        LocalIdParsingState.META_CALCULATION: "Invalid metadata tag: Calculation",
        LocalIdParsingState.META_STATE: "Invalid metadata tag: State",
        LocalIdParsingState.META_TYPE: "Invalid metadata tag: Type",
        LocalIdParsingState.META_DETAIL: "Invalid metadata tag: Detail",
        LocalIdParsingState.EMPTY_STATE: "Missing primary path or metadata",
    }

    def __init__(self) -> None:
        """Initialize a new error builder."""
        self._errors: list[tuple[LocalIdParsingState, str]] = []

    @property
    def has_error(self) -> bool:
        """Check if the builder has any errors."""
        return len(self._errors) > 0

    def add_error(
        self, state: LocalIdParsingState, message: str | None
    ) -> "LocalIdParsingErrorBuilder":
        """Add an error to the builder."""
        if message is not None:
            self._errors.append((state, message))
        else:
            if state in self._predefined_error_messages:
                self._errors.append((state, self._predefined_error_messages[state]))
            else:
                self._errors.append((state, f"Error in state {state}"))
        return self

    def build(self) -> ParsingErrors:
        """Build a ParsingErrors object from the collected errors."""
        if not self._errors:
            return ParsingErrors.empty()

        errors: list[tuple[str, str]] = [
            (str(state), message) for state, message in self._errors
        ]
        return ParsingErrors(errors)

    @classmethod
    def empty(cls) -> "LocalIdParsingErrorBuilder":
        """Create an empty error builder."""
        return cls()

    @classmethod
    def create(cls) -> "LocalIdParsingErrorBuilder":
        """Create a new error builder."""
        return cls()
