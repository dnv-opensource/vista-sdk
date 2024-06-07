from enum import Enum


class TraversalHandlerResult(Enum):
    STOP = 0
    SKIP_SUBTREE = 1
    CONTINUE = 2