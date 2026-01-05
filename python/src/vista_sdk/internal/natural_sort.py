"""Internal utilities for natural sorting of strings with embedded numbers."""

import re
from re import Pattern


def natural_sort_key(
    s: str, _nsre: Pattern[str] = re.compile(r"(\d+)")
) -> list[int | str]:
    """Generate a key for natural sorting of strings with numbers.

    Args:
        s: The string to generate a sort key for.
        _nsre: Compiled regex pattern for splitting numbers (internal).

    Returns:
        A list of string and integer parts for comparison.
    """
    return [int(text) if text.isdigit() else text.lower() for text in _nsre.split(s)]
