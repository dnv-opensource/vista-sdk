import hashlib
from collections import defaultdict
import zlib
from typing import Any, Tuple
from typing import Generic, TypeVar, Dict


from python.src import GmodNode

TValue = TypeVar('TValue')

class ChdDictionary(Generic[TValue]):
    def __init__(self, items):
        size = 1
        while size < len(items):
            size *= 2
        size *= 2 

        h = [[] for _ in range(size)]

        for i, (key, value) in enumerate(items):
            hash_val = self.hash(key)
            h[hash_val & (size - 1)].append((i + 1, hash_val))

        h.sort(key=lambda x: len(x), reverse=True)

        indices = [0] * size
        seeds = [0] * size

        for index in range(len(h)):
            if len(h[index]) > 1:
                sub_keys = h[index]
                seed = 0
                entries = {}
                while True:
                    seed += 1
                    entries.clear()
                    retry = False
                    for i, hash_val in sub_keys:
                        new_hash = self.Hashing.seed(seed, hash_val, size)
                        if new_hash not in entries and indices[new_hash] == 0:
                            entries[new_hash] = i
                        else:
                            retry = True
                            break
                    if not retry:
                        break

                for hash_key, i in entries.items():
                    indices[hash_key] = i

                seeds[sub_keys[0][1] & (size - 1)] = seed

        self.table = [None] * size
        free = []
        for i in range(len(indices)):
            if indices[i] == 0:
                free.append(i)
            else:
                indices[i] -= 1
                self.table[i] = items[indices[i]]

        for index in range(index, len(h)):
            if len(h[index]) > 0:
                k = h[index][0]
                dst = free.pop(0)
                indices[dst] = k[0] - 1
                self.table[dst] = items[indices[dst]]
                seeds[k[1] & (size - 1)] = -1 - dst

        self.seeds = seeds
        self._table = self.table
        self._seeds = self.seeds

    def __getitem__(self, key : str):
        value, found = self.try_get_value(key)
        if not found:
            self.ThrowHelper.throw_key_not_found_exception(key)
        return value

    def __iter__(self):
        for item in self._table:
            if item is not None:
                yield item

    def try_get_value(self, key) -> Tuple[Any, bool]:
        if not key:
            return None, False 

        hash_val = self.hash(key) 
        size = len(self._table)
        index = hash_val & (size - 1)
        seed = self._seeds[index]

        if seed is not None:
            if seed < 0:
                real_index = -seed - 1
                kvp = self._table[real_index]
                if kvp and kvp[0] == key:
                    return kvp[1], True
            else:
                index = ChdDictionary.Hashing.seed(seed, hash_val, size)
                kvp = self._table[index]
                if kvp and kvp[0] == key:
                    return kvp[1], True
                
        return None, False 


    class Enumerator:
        def __init__(self, table):
            self._table = table
            self._index = -1

        def __iter__(self):
            return self

        def __next__(self):
            while True:
                self._index += 1
                if self._index >= len(self._table):
                    raise StopIteration
                if self._table[self._index] is not None: 
                    key, value = self._table[self._index]
                    return (key, value) 
                
        @property
        def current(self):
            if self._index >= len(self._table) or self._table[self._index] is None:
                raise ValueError("Invalid operation or no current value.")
            key, value = self._table[self._index]
            return (key, value) 

        def reset(self):
            self._index = -1


    # TODO: See if it is right with the hashing 
    def hash(self, key: str) -> int:
        hash_val = 0x811C9DC5
        for ch in key:
            byte_val = ord(ch) 
            hash_val = ChdDictionary.Hashing.fnv(hash_val, byte_val) 
        return hash_val
    

    class Hashing:
        @staticmethod
        def fnv(hash_val: int, byte_val: int) -> int:
            return ((byte_val ^ hash_val) * 0x01000193) & 0xFFFFFFFF

        @staticmethod
        def crc32(hash_val: int, byte_val: int) -> int:
            return zlib.crc32(byte_val.to_bytes(1, 'little'), hash_val) & 0xFFFFFFFF

        @staticmethod
        def seed(seed: int, hash_val: int, size: int) -> int:
            x = seed + hash_val
            x ^= x >> 12
            x ^= x << 25
            x ^= x >> 27
            return (x * 0x2545F4914F6CDD1D & (size - 1))

        

    class ThrowHelper:
        @staticmethod
        def throw_key_not_found_exception(key):
            raise KeyError(f"No value associated to key: {key}")

        @staticmethod
        def throw_invalid_operation_exception():
            raise Exception("Invalid operation")
