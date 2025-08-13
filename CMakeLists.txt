cmake_minimum_required(VERSION 3.20)

project(CoolingSystem 
    VERSION 1.0.0
    DESCRIPTION "Automotive Cooling System Control"
    LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes -Wmissing-prototypes")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wold-style-definition -Wmissing-declarations")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wredundant-decls -Wnested-externs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wcast-qual")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wwrite-strings")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-default -Wswitch-enum")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunreachable-code -Winit-self")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wshadow")
endif()

set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_DOCUMENTATION "Build documentation" OFF)
option(ENABLE_COVERAGE "Enable code coverage" OFF)

if(ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)
    enable_testing()
endif()

set(COOLING_SYSTEM_SOURCES
    src/temp_sensor.c
    src/pid_controller.c
    src/dio_manager.c
    src/state_machine.c
    src/pump_control.c
    src/fan_control.c
    src/can_manager.c
)

set(COOLING_SYSTEM_HEADERS
    src/temp_sensor.h
    src/pid_controller.h
    src/dio_manager.h
    src/state_machine.h
    src/pump_control.h
    src/fan_control.h
    src/can_manager.h
)

add_library(cooling_system_lib STATIC
    ${COOLING_SYSTEM_SOURCES}
    ${COOLING_SYSTEM_HEADERS}
)

target_include_directories(cooling_system_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(cooling_system
    src/main.c
)

target_link_libraries(cooling_system 
    cooling_system_lib
)

if(BUILD_TESTS)
    add_executable(cooling_system_tests
        gtest/test_pid_controller.cpp
        gtest/test_can_manager.cpp
        gtest/test_temp_sensor.cpp
        gtest/test_pump_control.cpp
        gtest/test_fan_control.cpp
        gtest/test_dio_manager.cpp
        gtest/test_state_machine.cpp
    )
    
    target_link_libraries(cooling_system_tests
        cooling_system_lib
        GTest::gtest
        GTest::gtest_main
        Threads::Threads
    )
    
    target_include_directories(cooling_system_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    add_test(NAME CoolingSystemTests COMMAND cooling_system_tests)
    
    include(GoogleTest)
    gtest_discover_tests(cooling_system_tests)
endif()

install(TARGETS cooling_system
    RUNTIME DESTINATION bin
)

install(FILES ${COOLING_SYSTEM_HEADERS}
    DESTINATION include/cooling_system
)

install(TARGETS cooling_system_lib
    ARCHIVE DESTINATION lib
)

message(STATUS "=== Cooling System Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "CXX Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Enable coverage: ${ENABLE_COVERAGE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=====================================")