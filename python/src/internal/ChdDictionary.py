from __future__ import annotations
from typing import Optional, Tuple, List, Generic, TypeVar, Dict
TValue = TypeVar('TValue')

class ChdDictionary(Generic[TValue]):
    def __init__(self, items: List[Tuple[str, TValue]]):
        self._dict: Dict[str, TValue] = {key: value for key, value in items}

    def __getitem__(self, key: str) -> TValue:
        if key not in self._dict:
            self.ThrowHelper.throw_key_not_found_exception(key)
        return self._dict[key]

    def __iter__(self):
        return iter(self._dict.items())

    def try_get_value(self, key: str) -> Tuple[Optional[TValue], bool]:
        if key in self._dict:
            return self._dict[key], True
        return None, False


    class ThrowHelper:
        @staticmethod
        def throw_key_not_found_exception(key: str):
            raise KeyError(f"No value associated with key: {key}")

        @staticmethod
        def throw_invalid_operation_exception():
            raise Exception("Invalid operation")


