"""Defines the result of a traversal handler."""

from enum import Enum


class TraversalHandlerResult(Enum):
    """Enumeration of possible results for a traversal handler."""

    STOP = 0
    SKIP_SUBTREE = 1
    CONTINUE = 2
