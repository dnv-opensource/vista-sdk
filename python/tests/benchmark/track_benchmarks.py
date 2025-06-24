"""AI Generatded.

Track and analyze benchmark results over time,storing them in a SQLite
database and generating reports.
"""
#!/usr/bin/env python3

import json
import os
import sqlite3
import statistics
import sys
from datetime import datetime, timezone
from pathlib import Path

RESULTS_DIR = Path(__file__).parent / "benchmark_results"
DB_PATH = RESULTS_DIR / "benchmark_history.db"


def init_database() -> None:
    """Initialize the benchmark history database."""
    RESULTS_DIR.mkdir(exist_ok=True)

    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    # Create tables if they don't exist
    c.execute("""
        CREATE TABLE IF NOT EXISTS benchmark_runs (
            id INTEGER PRIMARY KEY,
            timestamp TEXT,
            commit_hash TEXT,
            python_version TEXT
        )
    """)

    c.execute("""
        CREATE TABLE IF NOT EXISTS benchmark_results (
            run_id INTEGER,
            benchmark_name TEXT,
            min_time REAL,
            max_time REAL,
            mean_time REAL,
            std_dev REAL,
            memory_mb REAL,
            FOREIGN KEY(run_id) REFERENCES benchmark_runs(id)
        )
    """)

    conn.commit()
    conn.close()


def store_results(results_file: Path) -> None:
    """Store benchmark results in the database."""
    with Path(results_file).open() as f:
        results = json.load(f)

    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    # Insert run metadata
    c.execute(
        """
        INSERT INTO benchmark_runs (timestamp, commit_hash, python_version)
        VALUES (?, ?, ?)
    """,
        (
            datetime.now(timezone.utc).isoformat(),
            os.environ.get("GIT_COMMIT", "unknown"),
            sys.version.split()[0],
        ),
    )
    run_id = c.lastrowid

    # Insert individual benchmark results
    for benchmark in results["benchmarks"]:
        c.execute(
            """
            INSERT INTO benchmark_results
            (run_id, benchmark_name, min_time, max_time, mean_time, std_dev, memory_mb)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """,
            (
                run_id,
                benchmark["name"],
                benchmark["min"],
                benchmark["max"],
                benchmark["mean"],
                benchmark.get("std_dev", 0),
                benchmark.get("memory_mb", 0),
            ),
        )

    conn.commit()
    conn.close()


def analyze_trends() -> list[dict]:
    """Analyze benchmark trends and detect regressions."""
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    # Get all benchmark names
    c.execute("SELECT DISTINCT benchmark_name FROM benchmark_results")
    benchmarks = [row[0] for row in c.fetchall()]

    # Analyze each benchmark
    regressions = []
    for benchmark in benchmarks:
        # Get recent results
        c.execute(
            """
            SELECT mean_time
            FROM benchmark_results r
            JOIN benchmark_runs runs ON r.run_id = runs.id
            WHERE benchmark_name = ?
            ORDER BY runs.timestamp DESC
            LIMIT 10
        """,
            (benchmark,),
        )

        recent_times = [row[0] for row in c.fetchall()]
        if len(recent_times) < 2:
            continue

        # Calculate statistics
        baseline_mean = statistics.mean(recent_times[1:])
        baseline_std = (
            statistics.stdev(recent_times[1:]) if len(recent_times) > 2 else 0
        )
        latest = recent_times[0]

        # Check for regression
        if latest > baseline_mean + (2 * baseline_std):
            regression = {
                "benchmark": benchmark,
                "baseline_mean": baseline_mean,
                "latest": latest,
                "percentage_increase": ((latest - baseline_mean) / baseline_mean) * 100,
            }
            regressions.append(regression)

    conn.close()
    return regressions


def generate_report() -> Path:
    """Generate a benchmark report."""
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()

    report = []
    report.append("# Benchmark Report")
    report.append(f"\nGenerated: {datetime.now(timezone.utc).isoformat()}")

    # Get latest run info
    c.execute("""
        SELECT timestamp, commit_hash, python_version
        FROM benchmark_runs
        ORDER BY timestamp DESC
        LIMIT 1
    """)
    latest_run = c.fetchone()
    if latest_run:
        report.append("\n## Latest Run")
        report.append(f"- Timestamp: {latest_run[0]}")
        report.append(f"- Commit: {latest_run[1]}")
        report.append(f"- Python Version: {latest_run[2]}")

    # Get recent benchmark results
    c.execute("""
        SELECT
            r.benchmark_name,
            r.mean_time,
            r.memory_mb
        FROM benchmark_results r
        JOIN benchmark_runs runs ON r.run_id = runs.id
        WHERE runs.timestamp = (
            SELECT MAX(timestamp) FROM benchmark_runs
        )
    """)

    report.append("\n## Latest Results")
    report.append("\nBenchmark | Mean Time (s) | Memory (MB)")
    report.append("---|---|---")
    for row in c.fetchall():
        report.append(f"{row[0]} | {row[1]:.6f} | {row[2]:.1f}")

    # Add regression analysis
    regressions = analyze_trends()
    if regressions:
        report.append("\n## Performance Regressions")
        for reg in regressions:
            report.append(f"\n### {reg['benchmark']}")
            report.append(f"- Baseline mean: {reg['baseline_mean']:.6f}s")
            report.append(f"- Latest: {reg['latest']:.6f}s")
            report.append(f"- Increase: {reg['percentage_increase']:.1f}%")

    conn.close()

    # Write report
    report_path = (
        RESULTS_DIR
        / f"benchmark_report_{datetime.now(timezone.utc).strftime('%Y%m%d')}.md"
    )
    with report_path.open("w") as f:
        f.write("\n".join(report))

    return report_path


def main() -> None:
    """Main entry point for benchmark history tracking."""
    init_database()

    # Store results if a results file is provided
    if len(sys.argv) > 1:
        results_file = Path(sys.argv[1])
        if results_file.exists():
            store_results(results_file)

    # Generate and print report path
    report_path = generate_report()
    print(f"Report generated at: {report_path}")


if __name__ == "__main__":
    main()
