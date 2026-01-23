"""Tests for ISO string validation.

Mirrors C# IsISOStringTests.
Tests rules according to: "ISO19848 5.2.1, Note 1" and "RFC3986 2.3 - Unreserved characters"s
"""  # noqa: E501

from pathlib import Path

import pytest

from vista_sdk.vis import VIS

# All allowed characters per ISO19848 5.2.1 and RFC3986 2.3
ALL_ALLOWED_CHARACTERS = (
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~"
)


class TestISOString:
    """Test class for ISO string validation."""

    @pytest.mark.parametrize("char", list(ALL_ALLOWED_CHARACTERS))
    def test_all_valid_characters(self, char: str) -> None:
        """Test that all allowed characters are valid ISO string characters."""
        assert VIS.is_iso_string(char), f"Character '{char}' should be valid"

    def test_all_allowed_in_one(self) -> None:
        """Test that a string containing all allowed characters is valid."""
        assert VIS.is_iso_string(ALL_ALLOWED_CHARACTERS)

    @pytest.mark.parametrize(
        ("input_str", "expected"),
        [
            ("test", True),
            ("TeST", True),
            ("with space", False),
            ("#%/*", False),
        ],
    )
    def test_spot_tests(self, input_str: str, expected: bool) -> None:
        """Spot tests for ISO string validation."""
        assert VIS.is_iso_string(input_str) == expected

    def test_smoke_test_parsing(self) -> None:
        """Smoke test parsing all LocalIds from test data file.

        Mirrors C# SmokeTest_Parsing.
        """
        testdata_path = Path(__file__).parent / "testdata" / "LocalIds.txt"

        errors: list[tuple[str, Exception | None]] = []
        count = 0
        succeeded = 0

        with testdata_path.open() as f:
            for line in f:
                local_id_str = line.strip()
                if not local_id_str:
                    continue

                count += 1
                try:
                    match = VIS.match_iso_local_id_string(local_id_str)
                    if not match:
                        errors.append((local_id_str, None))
                    else:
                        succeeded += 1
                except Exception as ex:
                    errors.append((local_id_str, ex))

        if errors:
            for local_id, err in errors:
                error_msg = err.args[0] if err else "Not a match"
                print(f"Failed to parse {local_id} with error {error_msg}")

        assert len(errors) == 0, f"Found {len(errors)} errors"
        assert succeeded == count, f"Only {succeeded}/{count} succeeded"
