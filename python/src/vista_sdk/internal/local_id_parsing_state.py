"""LocalIdParsingState enum for parsing state management."""

from enum import Enum, auto


class LocalIdParsingState(Enum):
    """Enum for tracking parsing state of a LocalId string.

    This enum is used by the parser to track the current state when parsing
    a LocalId string. Each state represents a different part of the format.
    """

    START = auto()
    NAMING_RULE = auto()
    VIS_VERSION = auto()
    PRIMARY_ITEM = auto()
    SECONDARY_ITEM_PREFIX = auto()
    SECONDARY_ITEM = auto()
    ITEM_DESCRIPTION = auto()
    META_QUANTITY = auto()
    META_CONTENT = auto()
    META_CALCULATION = auto()
    META_STATE = auto()
    META_COMMAND = auto()
    META_TYPE = auto()
    META_POSITION = auto()
    META_DETAIL = auto()
    META_PREFIX = auto()
    META_TAG = auto()
    STOP = auto()

    # For "other" errors
    EMPTY_STATE = 100
    FORMATTING = 101
    COMPLETENESS = 102

    # UniversalId
    NAMING_ENTITY = 200
    IMO_NUMBER = 201

    def __lt__(self, other) -> bool:  # noqa : ANN001
        """Less than comparison (<) between parsing states."""
        if not isinstance(other, LocalIdParsingState):
            return NotImplemented
        return self.value < other.value

    def __gt__(self, other) -> bool:  # noqa : ANN001
        """Greater than comparison (>) between parsing states."""
        if not isinstance(other, LocalIdParsingState):
            return NotImplemented
        return self.value > other.value

    def __le__(self, other) -> bool:  # noqa : ANN001
        """Less than or equal comparison (<=) between parsing states."""
        if not isinstance(other, LocalIdParsingState):
            return NotImplemented
        return self.value <= other.value

    def __ge__(self, other) -> bool:  # noqa : ANN001
        """Greater than or equal comparison (>=) between parsing states."""
        if not isinstance(other, LocalIdParsingState):
            return NotImplemented
        return self.value >= other.value
