#!/usr/bin/env python3
"""Main entry point for Vista SDK Python benchmarks."""

from __future__ import annotations

import argparse
import json
import platform
import sys
from datetime import datetime
from pathlib import Path
from typing import Any

import pytest


def format_time(seconds: float) -> str:
    """Format time in appropriate units."""
    if seconds >= 1:
        return f"{seconds:.2f} s"
    if seconds >= 0.001:
        return f"{seconds * 1000:.1f} ms"
    if seconds >= 0.000001:
        return f"{seconds * 1_000_000:.1f} μs"
    return f"{seconds * 1_000_000_000:.0f} ns"


def format_ops(ops: float) -> str:
    """Format operations per second."""
    if ops >= 1_000_000:
        return f"{ops / 1_000_000:.1f}M ops/s"
    if ops >= 1_000:
        return f"{ops / 1_000:.0f}K ops/s"

    return f"{ops:.0f} ops/s"


def format_ops_table(ops: float) -> str:
    """Format OPS for table display (in Kops/s)."""
    if ops >= 1_000_000:
        return f"{ops / 1_000:,.0f}"
    if ops >= 1_000:
        return f"{ops / 1_000:,.1f}"

    return f"{ops / 1_000:.2f}"


def format_group_name(group: str) -> str:
    """Convert group name to readable title."""
    # Handle underscore-separated names like ISO19848_DataChannelList_Lookup
    return group.replace("_", " ")


# Manual mapping for representative summary: (category, operation) -> (group, test_name)
SUMMARY_MAPPING: list[tuple[str, str, str, str]] = [
    ("Lookup", "Codebooks lookup", "CodebooksLookup", "test_codebooks_lookup"),
    ("Lookup", "Gmod node by code", "GmodLookup", "test_lookup_by_code"),
    (
        "Lookup",
        "DataChannel by short_id",
        "ISO19848_DataChannelList_Lookup",
        "test_lookup_by_short_id",
    ),
    (
        "Lookup",
        "DataChannel by local_id",
        "ISO19848_DataChannelList_Lookup",
        "test_lookup_by_local_id",
    ),
    (
        "Serialization",
        "JSON serialize (DC)",
        "DataChannelListSerialization",
        "test_json_serialize",
    ),
    (
        "Serialization",
        "JSON deserialize (DC)",
        "DataChannelListSerialization",
        "test_json_deserialize",
    ),
    (
        "Domain",
        "DataChannelList to domain",
        "ISO19848_DataChannelList_ToDomain",
        "test_data_channel_list_to_domain",
    ),
    (
        "Domain",
        "TimeSeriesData to domain",
        "ISO19848_TimeSeriesData_ToDomain",
        "test_time_series_data_to_domain",
    ),
    ("Parsing", "LocalId complex", "LocalIdParse", "test_parse_complex"),
    ("Parsing", "GmodPath full path", "GmodPathParse", "test_try_parse_full_path"),
    ("Versioning", "Path conversion", "GmodVersioningConvertPath", "test_convert_path"),
    ("Traversal", "Full Gmod traversal", "GmodTraversal", "test_full_traversal"),
]


def generate_summary_table(benchmarks: list[dict[str, Any]]) -> list[str]:
    """Generate the curated summary table for representative SDK benchmarks."""
    # Build lookup: (group, test_name) -> stats
    lookup: dict[tuple[str, str], dict[str, Any]] = {}
    for bench in benchmarks:
        group = bench.get("group", "default")
        # Extract test name without parameters
        full_name = bench["name"].split("::")[-1]
        test_name = full_name.split("[")[0]
        lookup[(group, test_name)] = bench["stats"]

    lines: list[str] = []
    lines.append("| Category | Operation | Mean Time | Throughput |")
    lines.append("|----------|-----------|-----------|------------|")

    for category, operation, group, test_name in SUMMARY_MAPPING:
        stats = lookup.get((group, test_name))
        if stats:
            mean = format_time(stats["mean"])
            ops = format_ops(stats["ops"])
            lines.append(f"| {category} | {operation} | {mean} | {ops} |")

    return lines


def generate_markdown(json_path: Path, output_path: Path) -> None:
    """Generate BENCHMARKS.md from pytest-benchmark JSON output.

    Dynamically generates markdown for all benchmark groups found in the JSON.
    """
    with Path.open(json_path) as f:
        data = json.load(f)

    benchmarks: list[dict[str, Any]] = data.get("benchmarks", [])
    machine_info = data.get("machine_info", {})

    # Get environment info
    python_version = machine_info.get("python_version", platform.python_version())
    platform_name = machine_info.get("system", platform.system())
    date_str = datetime.now(tz=machine_info.get("timezone")).strftime("%B %Y")

    # Group benchmarks by their group
    groups: dict[str, list[dict[str, Any]]] = {}
    for bench in benchmarks:
        group = bench.get("group", "default")
        if group not in groups:
            groups[group] = []
        groups[group].append(bench)

    # Generate markdown
    lines: list[str] = []
    lines.append("# Vista SDK Python Benchmarks")
    lines.append("")
    lines.append("Performance benchmarks for the Vista SDK Python implementation.")
    lines.append("")
    lines.append(f"**Environment:** Python {python_version}, pytest-benchmark 5.1.0")
    lines.append(f"**Platform:** {platform_name}")
    lines.append(f"**Last Updated:** {date_str}")
    lines.append("")

    # Summary table - curated representative benchmarks
    lines.append("## Summary")
    lines.append("")
    summary_table = generate_summary_table(benchmarks)
    lines.extend(summary_table)
    lines.append("")

    # Detailed results - one section per group
    lines.append("## Detailed Results")
    lines.append("")

    # Special notes for specific groups
    group_notes: dict[str, str] = {
        "LocalIdParse": (
            "> **Note:** Parsing performance depends on the structural location "
            "of the target node in the GMOD tree due to depth-first search (DFS) "
            "traversal, not just the apparent path complexity. Paths where the "
            "target node appears later in the child iteration order require more "
            "traversal to find."
        ),
    }

    for group_name in sorted(groups.keys()):
        group_benchmarks = groups[group_name]

        lines.append(f"### {format_group_name(group_name)}")
        lines.append("")
        lines.append("| Test | Mean | Std Dev | OPS |")
        lines.append("|------|------|---------|-----|")

        for bench in sorted(group_benchmarks, key=lambda x: x["stats"]["mean"]):
            name = bench["name"].split("::")[-1]
            stats = bench["stats"]
            lines.append(
                f"| {name} | {format_time(stats['mean'])} | "
                f"{format_time(stats['stddev'])} | {format_ops(stats['ops'])} |"
            )
        lines.append("")

        # Add note if exists for this group
        if group_name in group_notes:
            lines.append(group_notes[group_name])
            lines.append("")

    # Running benchmarks section
    lines.append("## Running Benchmarks")
    lines.append("")
    lines.append("```bash")
    lines.append("# Run all benchmarks and generate this file")
    lines.append("python run_benchmarks.py --generate-markdown")
    lines.append("")
    lines.append("# Run all benchmarks")
    lines.append("python run_benchmarks.py")
    lines.append("")
    lines.append("# Run specific benchmark group")
    lines.append("python run_benchmarks.py --group gmod")
    lines.append("")
    lines.append("# Run with verbose output")
    lines.append("python run_benchmarks.py --verbose")
    lines.append("```")
    lines.append("")

    # Notes section
    lines.append("## Notes")
    lines.append("")
    lines.append("- All times are mean values across multiple iterations")
    lines.append("- OPS = Operations Per Second")
    lines.append(
        "- Benchmarks run with garbage collection enabled (realistic conditions)"
    )
    lines.append("- Results may vary based on hardware and system load")
    lines.append("")

    # Write output
    output_path.write_text("\n".join(lines))
    print(f"Generated {output_path}")

    # Also update README.md with summary
    update_readme_summary(summary_table)


def update_readme_summary(summary_table: list[str]) -> None:
    """Update the Benchmark Results section in README.md with the summary table."""
    readme_path = Path("README.md")
    if not readme_path.exists():
        print("README.md not found, skipping update")
        return

    content = readme_path.read_text()

    # Find the "### Benchmark Results" section
    start_marker = "### Benchmark Results"
    end_marker = "See [BENCHMARKS.md]"

    start_idx = content.find(start_marker)
    if start_idx == -1:
        print("Could not find '### Benchmark Results' in README.md, skipping update")
        return

    # Find the end of this section
    section_start = start_idx + len(start_marker)
    end_idx = content.find(end_marker, section_start)
    if end_idx == -1:
        end_idx = len(content)

    # Build the summary section
    summary_lines: list[str] = ["", ""]
    summary_lines.extend(summary_table)
    summary_lines.extend(["", ""])

    # Replace the section content
    new_content = (
        content[:start_idx]
        + start_marker
        + "\n".join(summary_lines)
        + content[end_idx:]
    )
    readme_path.write_text(new_content)
    print(f"Updated {readme_path}")


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
        help="Run specific benchmark group",
        choices=["codebooks", "gmod", "transport", "localid", "iso19848"],
    )
    parser.add_argument(
        "--save-data",
        "-s",
        action="store_true",
        help="Save benchmark results to JSON",
    )
    parser.add_argument(
        "--generate-markdown",
        action="store_true",
        help="Generate BENCHMARKS.md from results",
    )
    parser.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help="Verbose output",
    )

    args = parser.parse_args()

    # Map group names to file patterns
    group_to_pattern = {
        "codebooks": "test_codebooks",
        "gmod": "test_gmod",
        "transport": "test_data_channel",
        "localid": "test_local_id",
        "iso19848": "test_iso19848",
    }

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
        # Use -k to filter by test file name pattern
        pattern = group_to_pattern.get(args.group, args.group)
        if pattern:
            pytest_args.extend(["-k", pattern])

    # Always save JSON if generating markdown, or if explicitly requested
    json_path = Path("benchmark_results.json")
    if args.save_data or args.generate_markdown:
        pytest_args.append(f"--benchmark-json={json_path}")

    if args.verbose:
        pytest_args.append("-v")

    print("Vista SDK Python Benchmarks")
    print("=" * 40)
    print(f"Running benchmarks with: {' '.join(pytest_args)}")
    print()

    # Run benchmarks
    result = pytest.main(pytest_args)

    # Generate markdown if requested
    if args.generate_markdown and json_path.exists():
        print()
        print("=" * 40)
        generate_markdown(json_path, Path("BENCHMARKS.md"))

    return result


if __name__ == "__main__":
    sys.exit(main())
