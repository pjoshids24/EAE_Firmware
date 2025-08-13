# EAE Combined Challenge

This is a submission for EAE Combined Challenge. A comprehensive C implementation of a cooling system control firmware with CAN bus communication, PID control, state machine architecture, and comprehensive unit testing.

## Features

### Core Functionality
- **PID Controller**: Precise temperature control with configurable gains
- **State Machine**: Robust state management (INIT, OFF, STANDBY, COOLING, CRITICAL_TEMP, FAULT)
- **Temperature Sensor**: ADC-based sensor reading with fault detection
- **Pump Control**: PWM-based coolant pump speed control
- **Fan Control**: PWM-based cooling fan speed control
- **Digital I/O**: Ignition switch and coolant level monitoring
- **Safety Functions**: Overheat protection, low coolant detection, fault handling
- **CAN Manager**: Send/receive temperature, pump, fan, and system status

## Project Structure

```
cooling_system/
├── src/                    # Source files
│   ├── main.c             # Main application entry point
│   ├── pid_controller.c   # PID control implementation
│   ├── temp_sensor.c      # Temperature sensor driver
│   ├── pump_control.c     # Pump PWM control
│   ├── fan_control.c      # Fan PWM control
│   ├── dio_manager.c      # Digital I/O management
│   ├── state_machine.c    # System state machine
│   ├── can_manager.c      # CAN bus communication
│   ├── utils.c            # Utility functions
│   └── *.h               # Header files
├── tests/                 # Unit tests
│   ├── test_main.cpp      # Test runner
│   ├── test_pid_controller.cpp
│   └── test_can_manager.cpp
│   └── test_*.cpp
├── CMakeLists.txt         # Build configuration
├── Dockerfile             # Container setup
├── build.sh               # Build/launch script
└── README.md              # This file
```

## Building and Running

### Prerequisites

**Linux/macOS:**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake gcc g++ libgtest-dev

```bash
chmod +x build_and_run.sh
./build.sh 35.0 2.0 0.5 0.1
```
