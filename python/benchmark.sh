#!/bin/bash
# Vista SDK Python Benchmark Runner
# Mirrors the C# build and run experience

set -e

BENCHMARK_DIR="tests/benchmark"
RESULTS_DIR="benchmark_results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}============================================${NC}"
    echo -e "${BLUE}Vista SDK Python Benchmarks${NC}"
    echo -e "${BLUE}============================================${NC}"
    echo
}

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  run [GROUP]           Run benchmarks (optionally for specific group)"
    echo "  list                  List available benchmarks"
    echo "  clean                 Clean benchmark results"
    echo "  help                  Show this help message"
    echo
    echo "Groups: codebooks, gmod, transport, internal"
    echo
    echo "Examples:"
    echo "  $0 run                # Run all benchmarks"
    echo "  $0 run gmod          # Run only GMOD benchmarks"
    echo "  $0 list              # List all available benchmarks"
}

run_benchmarks() {
    local group=$1
    print_header

    if [ ! -d "$RESULTS_DIR" ]; then
        mkdir -p "$RESULTS_DIR"
    fi

    local args=(
        "$BENCHMARK_DIR/"
        "--benchmark-only"
        "--benchmark-sort=mean"
        "--benchmark-group-by=group"
        "--benchmark-json=$RESULTS_DIR/results.json"
        "--benchmark-json-unit=seconds"
        "-v"
    )

    if [ -n "$group" ]; then
        echo -e "${YELLOW}Running benchmarks for group: $group${NC}"
        args+=("-m" "benchmark and $group")
    else
        echo -e "${YELLOW}Running all benchmarks...${NC}"
    fi

    echo
    python -m pytest "${args[@]}"

    if [ $? -eq 0 ]; then
        echo
        echo -e "${GREEN}✓ Benchmarks completed successfully!${NC}"
        echo -e "${GREEN}Results saved to: $RESULTS_DIR/results.json${NC}"
    else
        echo
        echo -e "${RED}✗ Benchmarks failed!${NC}"
        exit 1
    fi
}

list_benchmarks() {
    print_header
    echo -e "${YELLOW}Available benchmark files:${NC}"
    echo

    find "$BENCHMARK_DIR" -name "test_*.py" -type f | sort | while read -r file; do
        basename=$(basename "$file" .py)
        echo -e "  ${GREEN}•${NC} $basename"
    done

    echo
    echo -e "${YELLOW}Available groups:${NC}"
    echo -e "  ${GREEN}•${NC} codebooks - Codebook lookup benchmarks"
    echo -e "  ${GREEN}•${NC} gmod - GMOD parsing and traversal benchmarks"
    echo -e "  ${GREEN}•${NC} transport - Data serialization benchmarks"
    echo -e "  ${GREEN}•${NC} internal - Internal algorithm benchmarks"
}

clean_results() {
    echo -e "${YELLOW}Cleaning benchmark results...${NC}"
    if [ -d "$RESULTS_DIR" ]; then
        rm -rf "$RESULTS_DIR"
        echo -e "${GREEN}✓ Results cleaned${NC}"
    else
        echo -e "${YELLOW}No results to clean${NC}"
    fi
}

# Main script logic
case "${1:-help}" in
    "run")
        run_benchmarks "$2"
        ;;
    "list")
        list_benchmarks
        ;;
    "clean")
        clean_results
        ;;
    "help"|*)
        print_header
        print_usage
        ;;
esac
