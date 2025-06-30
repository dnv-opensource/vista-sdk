"""This module implements a generic dictionary class for handling key-value pairs with type safety."""  # noqa: E501

from __future__ import annotations

from collections.abc import Iterator
from typing import Generic, TypeVar

TValue = TypeVar("TValue")


class Dictionary(Generic[TValue]):
    """A generic dictionary class that provides type-safe access to key-value pairs."""

    def __init__(self, items: list[tuple[str, TValue]]) -> None:
        """Initialize the dictionary with a list of key-value pairs."""
        self._dict: dict[str, TValue] = dict(items)

    def __getitem__(self, key: str) -> TValue:
        """Get the value associated with the given key."""
        if key not in self._dict:
            self.ThrowHelper.throw_key_not_found_exception(key)
        return self._dict[key]

    def __iter__(self) -> Iterator[tuple[str, TValue]]:
        """Return an iterator over the key-value pairs in the dictionary."""
        return iter(self._dict.items())

    def try_get_value(self, key: str) -> tuple[TValue | None, bool]:
        """Try to get the value associated with the given key."""
        if key in self._dict:
            return self._dict[key], True
        return None, False

    class ThrowHelper:
        """Helper class for throwing exceptions related to dictionary operations."""

        @staticmethod
        def throw_key_not_found_exception(key: str) -> None:
            """Throw a KeyError if the key is not found in the dictionary."""
            raise KeyError(f"No value associated with key: {key}")

        @staticmethod
        def throw_invalid_operation_exception() -> None:
            """Throw an exception for invalid operations."""
            raise Exception("Invalid operation")
