#!/usr/bin/env python3
"""Main entry point for Vista SDK Python benchmarks.

Mirrors the functionality of C#'s Program.cs
"""

import argparse
import sys

import pytest


def main() -> int:
    """Main benchmark runner - mirrors C# BenchmarkSwitcher functionality."""
    parser = argparse.ArgumentParser(description="Vista SDK Python Benchmarks")
    parser.add_argument(
        "--filter",
        "-f",
        help="Filter benchmarks by name pattern",
        default="",
    )
    parser.add_argument(
        "--group",
        "-g",
        help="Run specific benchmark group (codebooks, gmod, transport, internal)",
        choices=["codebooks", "gmod", "transport", "internal"],
    )
    parser.add_argument(
        "--memory",
        "-m",
        action="store_true",
        help="Enable memory profiling",
    )
    parser.add_argument(
        "--save-data",
        "-s",
        action="store_true",
        help="Save benchmark results to JSON",
    )
    parser.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help="Verbose output",
    )

    args = parser.parse_args()

    # Build pytest arguments
    pytest_args = [
        "tests/benchmark/",
        "--benchmark-only",
        "--benchmark-sort=mean",
        "--benchmark-group-by=group",
    ]

    if args.filter:
        pytest_args.extend(["-k", args.filter])

    if args.group:
        pytest_args.extend(["-m", f"benchmark and {args.group}"])

    if args.memory:
        pytest_args.append("--benchmark-memory")

    if args.save_data:
        pytest_args.extend(
            [
                "--benchmark-json=benchmark_results.json",
                "--benchmark-json-unit=seconds",
            ]
        )

    if args.verbose:
        pytest_args.append("-v")

    print("Vista SDK Python Benchmarks")
    print("=" * 40)
    print(f"Running benchmarks with: {' '.join(pytest_args)}")
    print()

    # Run benchmarks
    return pytest.main(pytest_args)


if __name__ == "__main__":
    sys.exit(main())
