#!/bin/bash

# run_tests.sh - Comprehensive test runner for cooling system
# Author: Generated Code
# Date: 2025

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

print_usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -v, --verbose       Verbose test output"
    echo "  -f, --filter <pattern>  Run only tests matching pattern"
    echo "  -c, --coverage      Generate code coverage report"
    echo "  -q, --quick         Quick test run (skip long tests)"
    echo "  --unit              Run only unit tests"
    echo "  --integration       Run only integration tests"
    echo "  --valgrind          Run tests with Valgrind memory checking"
    echo ""
    echo "Examples:"
    echo "  $0                          # Run all tests"
    echo "  $0 -v                       # Verbose output"
    echo "  $0 -f \"PID*\"               # Run only PID tests"
    echo "  $0 --unit                   # Run only unit tests"
    echo "  $0 -c                       # Generate coverage report"
}

# Default parameters
VERBOSE=""
FILTER=""
COVERAGE=false
QUICK=false
UNIT_ONLY=false
INTEGRATION_ONLY=false
VALGRIND=false
BUILD_DIR="build"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -v|--verbose)
            VERBOSE="--gtest_verbose"
            shift
            ;;
        -f|--filter)
            FILTER="--gtest_filter=$2"
            shift 2
            ;;
        -c|--coverage)
            COVERAGE=true
            shift
            ;;
        -q|--quick)
            QUICK=true
            shift
            ;;
        --unit)
            UNIT_ONLY=true
            shift
            ;;
        --integration)
            INTEGRATION_ONLY=true
            shift
            ;;
        --valgrind)
            VALGRIND=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

echo -e "${BLUE}Cooling System Test Runner${NC}"
echo "=========================="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory not found. Creating and building...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Change to build directory
cd "$BUILD_DIR"

# Configure build with coverage if requested
if [ "$COVERAGE" = true ]; then
    echo -e "${YELLOW}Configuring with code coverage...${NC}"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
else
    echo -e "${YELLOW}Configuring build...${NC}"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
fi

# Build the project
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

# Check if test executable exists
if [ ! -f "./cooling_system_tests" ]; then
    echo -e "${RED}Error: Test executable not found!${NC}"
    exit 1
fi

# Prepare test command
TEST_CMD="./cooling_system_tests"

# Add verbose flag if requested
if [ -n "$VERBOSE" ]; then
    TEST_CMD="$TEST_CMD $VERBOSE"
fi

# Add filter if specified
if [ -n "$FILTER" ]; then
    TEST_CMD="$TEST_CMD $FILTER"
elif [ "$UNIT_ONLY" = true ]; then
    TEST_CMD="$TEST_CMD --gtest_filter=-IntegrationTest*"
elif [ "$INTEGRATION_ONLY" = true ]; then
    TEST_CMD="$TEST_CMD --gtest_filter=IntegrationTest*"
fi

# Add quick test filter if requested
if [ "$QUICK" = true ]; then
    # Exclude long-running tests
    TEST_CMD="$TEST_CMD --gtest_filter=-*StressTest*:*LongRunning*:*RealTimePerformance*"
fi

# Run tests with Valgrind if requested
if [ "$VALGRIND" = true ]; then
    echo -e "${PURPLE}Running tests with Valgrind memory checking...${NC}"
    which valgrind > /dev/null 2>&1 || {
        echo -e "${RED}Error: Valgrind not found. Please install valgrind.${NC}"
        exit 1
    }
    TEST_CMD="valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 $TEST_CMD"
fi

# Display test configuration
echo -e "${YELLOW}Test Configuration:${NC}"
echo "  Verbose: $([ -n "$VERBOSE" ] && echo "YES" || echo "NO")"
echo "  Filter: $([ -n "$FILTER" ] && echo "${FILTER#--gtest_filter=}" || echo "ALL")"
echo "  Coverage: $([ "$COVERAGE" = true ] && echo "YES" || echo "NO")"
echo "  Quick Mode: $([ "$QUICK" = true ] && echo "YES" || echo "NO")"
echo "  Unit Only: $([ "$UNIT_ONLY" = true ] && echo "YES" || echo "NO")"
echo "  Integration Only: $([ "$INTEGRATION_ONLY" = true ] && echo "YES" || echo "NO")"
echo "  Valgrind: $([ "$VALGRIND" = true ] && echo "YES" || echo "NO")"
echo ""

# Run the tests
echo -e "${YELLOW}Running tests...${NC}"
echo "Command: $TEST_CMD"
echo ""

# Execute tests
eval $TEST_CMD
TEST_RESULT=$?

# Check test results
if [ $TEST_RESULT -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✓ All tests passed!${NC}"
else
    echo ""
    echo -e "${RED}✗ Some tests failed!${NC}"
fi

# Generate coverage report if requested
if [ "$COVERAGE" = true ] && [ $TEST_RESULT -eq 0 ]; then
    echo ""
    echo -e "${YELLOW}Generating code coverage report...${NC}"
    
    # Check if gcov is available
    which gcov > /dev/null 2>&1 || {
        echo -e "${RED}Warning: gcov not found. Cannot generate coverage report.${NC}"
        exit $TEST_RESULT
    }
    
    # Generate coverage data
    gcov -r ../src/*.c
    
    # Check if lcov is available for HTML report
    if which lcov > /dev/null 2>&1 && which genhtml > /dev/null 2>&1; then
        echo -e "${YELLOW}Generating HTML coverage report...${NC}"
        
        # Create coverage directory
        mkdir -p coverage
        
        # Generate lcov info file
        lcov --capture --directory . --output-file coverage/coverage.info
        lcov --remove coverage/coverage.info '/usr/*' --output-file coverage/coverage.info
        lcov --remove coverage/coverage.info '*/tests/*' --output-file coverage/coverage.info
        
        # Generate HTML report
        genhtml coverage/coverage.info --output-directory coverage/html
        
        echo -e "${GREEN}Coverage report generated: coverage/html/index.html${NC}"
    else
        echo -e "${YELLOW}lcov/genhtml not available. Raw gcov files generated.${NC}"
    fi
fi

# Run CTest if available
if [ $TEST_RESULT -eq 0 ] && which ctest > /dev/null 2>&1; then
    echo ""
    echo -e "${YELLOW}Running CTest...${NC}"
    ctest --output-on-failure
fi

# Summary
echo ""
echo -e "${BLUE}Test Summary:${NC}"
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}  Status: PASSED${NC}"
else
    echo -e "${RED}  Status: FAILED${NC}"
fi

echo "  Build Directory: $BUILD_DIR"
echo "  Test Executable: ./cooling_system_tests"

if [ "$COVERAGE" = true ]; then
    echo "  Coverage Report: coverage/html/index.html"
fi

exit $TEST_RESULT