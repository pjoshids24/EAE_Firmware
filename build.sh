#!/bin/bash

set -e

DEFAULT_SETPOINT=35.0
DEFAULT_KP=2.0
DEFAULT_KI=0.5
DEFAULT_KD=0.1

SETPOINT=${1:-$DEFAULT_SETPOINT}
KP=${2:-$DEFAULT_KP}
KI=${3:-$DEFAULT_KI}
KD=${4:-$DEFAULT_KD}

print_usage() {
    echo "Usage: $0 [setpoint] [kp] [ki] [kd]"
    echo "  setpoint: Target temperature (default: $DEFAULT_SETPOINT)"
    echo "  kp:       Proportional gain (default: $DEFAULT_KP)"
    echo "  ki:       Integral gain (default: $DEFAULT_KI)"
    echo "  kd:       Derivative gain (default: $DEFAULT_KD)"
    echo ""
    echo "Examples:"
    echo "  $0                    # Use all defaults"
    echo "  $0 40.0               # Set temperature to 40°C"
    echo "  $0 40.0 2.5 0.3 0.05  # Custom PID parameters"
}

check_dependencies() {
    echo "Checking dependencies..."
    
    if ! command -v cmake &> /dev/null; then
        echo "ERROR: CMake is not installed!"
        echo "Install with: sudo apt-get install cmake"
        exit 1
    fi
    
    if ! command -v make &> /dev/null; then
        echo "ERROR: Make is not installed!"
        echo "Install with: sudo apt-get install build-essential"
        exit 1
    fi
    
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "Found CMake version: $CMAKE_VERSION"
    
    if ! command -v pkg-config &> /dev/null; then
        echo "WARNING: pkg-config not found, some dependencies might not be detected"
    fi
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    print_usage
    exit 0
fi

echo "========================================"
echo "Cooling System Build and Launch Script"
echo "========================================"

check_dependencies

BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

echo "Entering build directory..."
cd "$BUILD_DIR"

echo "Configuring project with CMake..."
if ! cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DCMAKE_VERBOSE_MAKEFILE=ON; then
    echo "ERROR: CMake configuration failed!"
    echo ""
    echo "Common solutions:"
    echo "1. Install missing dependencies: sudo apt-get install libgtest-dev"
    echo "2. Update CMake: sudo apt-get install cmake"
    echo "3. Check if all source files are present"
    exit 1
fi

echo "Building project..."
if ! make -j$(nproc); then
    echo "ERROR: Build failed!"
    echo ""
    echo "Try building with verbose output:"
    echo "  make VERBOSE=1"
    exit 1
fi

echo "Build successful!"

echo "Running unit tests..."
if [ -f "./cooling_system_tests" ]; then
    if ! ./cooling_system_tests; then
        echo "WARNING: Some unit tests failed!"
        echo "Continuing with application launch..."
    else
        echo "All unit tests passed!"
    fi
else
    echo "WARNING: Unit tests executable not found, skipping tests"
fi

echo "Launching cooling system..."
if [ ! -f "./cooling_system" ]; then
    echo "ERROR: cooling_system executable not found!"
    echo "Build may have failed or executable name changed"
    exit 1
fi

echo "Starting with parameters:"
echo "  Setpoint: $SETPOINT°C"
echo "  PID Gains: Kp=$KP, Ki=$KI, Kd=$KD"
echo ""

exec ./cooling_system "$SETPOINT" "$KP" "$KI" "$KD"