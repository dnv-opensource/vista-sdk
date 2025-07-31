"""Internal hashing benchmarks matching C# implementation."""

import zlib
from typing import Protocol

import pytest
from pytest_benchmark.fixture import BenchmarkFixture

from tests.benchmark.benchmark_base import (
    BenchmarkConfig,
    MethodOrderPolicy,
    SummaryOrderPolicy,
    run_benchmark,
)


class Hasher(Protocol):
    """Protocol for hash functions."""

    def hash(self, data: bytes) -> int:
        """Hash the given data."""
        ...


class BclHasher:
    """Built-in Python hash equivalent to C# BCL."""

    def hash(self, data: bytes) -> int:
        """Hash using built-in Python hash function."""
        return hash(data.decode("utf-8"))


class BclOrdHasher:
    """Ordinal hash equivalent to C# GetHashCodeOrdinal."""

    def hash(self, data: bytes) -> int:
        """Hash using ordinal method (each char contributes differently)."""
        result = 0
        for _, b in enumerate(data):
            result = ((result << 5) + result) ^ b
        return result


class Crc32Hasher:
    """CRC32 hash equivalent to C# Crc32IntrinsicHasher."""

    def hash(self, data: bytes) -> int:
        """Hash using CRC32 algorithm."""
        return zlib.crc32(data) & 0xFFFFFFFF


class FnvHasher:
    """FNV hash equivalent to C# FnvHasher."""

    def hash(self, data: bytes) -> int:
        """Hash using FNV algorithm."""
        hash_value = 0x811C9DC5
        for byte in data:
            hash_value = ((hash_value * 0x01000193) ^ byte) & 0xFFFFFFFF
        return hash_value


class LarssonHasher:
    """Larsson hash equivalent to C# LarssonHasher."""

    def hash(self, data: bytes) -> int:
        """Hash using Larsson algorithm."""
        hash_value = 0
        for byte in data:
            hash_value = (
                (hash_value << 6) + (hash_value << 16) - hash_value + byte
            ) & 0xFFFFFFFF
        return hash_value


@pytest.mark.benchmark(group="internal")
class TestShortStringHash:
    """Mirror of C#'s ShortStringHash benchmark class."""

    @pytest.mark.parametrize("input_str", ["400", "H346.11112"])
    def test_bcl(self, benchmark: BenchmarkFixture, input_str: str) -> None:
        """Mirror of C#'s Bcl benchmark method (baseline)."""
        hasher = BclHasher()

        def hash_string() -> int:
            return hasher.hash(input_str.encode("utf-8"))

        config = BenchmarkConfig(
            group="ShortStringHash",
            baseline=True,
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Bcl",
        )

        result = run_benchmark(benchmark, hash_string, config)
        assert isinstance(result, int)

    @pytest.mark.parametrize("input_str", ["400", "H346.11112"])
    def test_bcl_ord(self, benchmark: BenchmarkFixture, input_str: str) -> None:
        """Mirror of C#'s BclOrd benchmark method."""
        hasher = BclOrdHasher()

        def hash_string() -> int:
            return hasher.hash(input_str.encode("utf-8"))

        config = BenchmarkConfig(
            group="ShortStringHash",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="BclOrd",
        )

        result = run_benchmark(benchmark, hash_string, config)
        assert isinstance(result, int)

    @pytest.mark.parametrize("input_str", ["400", "H346.11112"])
    def test_larsson(self, benchmark: BenchmarkFixture, input_str: str) -> None:
        """Mirror of C#'s Larsson benchmark method."""
        hasher = LarssonHasher()

        def hash_string() -> int:
            return hasher.hash(input_str.encode("utf-8"))

        config = BenchmarkConfig(
            group="ShortStringHash",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Larsson",
        )

        result = run_benchmark(benchmark, hash_string, config)
        assert isinstance(result, int)

    @pytest.mark.parametrize("input_str", ["400", "H346.11112"])
    def test_crc32_intrinsic(self, benchmark: BenchmarkFixture, input_str: str) -> None:
        """Mirror of C#'s Crc32Intrinsic benchmark method."""
        hasher = Crc32Hasher()

        def hash_string() -> int:
            return hasher.hash(input_str.encode("utf-8"))

        config = BenchmarkConfig(
            group="ShortStringHash",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Crc32Intrinsic",
        )

        result = run_benchmark(benchmark, hash_string, config)
        assert isinstance(result, int)

    @pytest.mark.parametrize("input_str", ["400", "H346.11112"])
    def test_fnv(self, benchmark: BenchmarkFixture, input_str: str) -> None:
        """Mirror of C#'s Fnv benchmark method."""
        hasher = FnvHasher()

        def hash_string() -> int:
            return hasher.hash(input_str.encode("utf-8"))

        config = BenchmarkConfig(
            group="ShortStringHash",
            method_order=MethodOrderPolicy.Declared,
            summary_order=SummaryOrderPolicy.FastestToSlowest,
            description="Fnv",
        )

        result = run_benchmark(benchmark, hash_string, config)
        assert isinstance(result, int)
