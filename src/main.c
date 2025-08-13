#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

#include "pid_controller.h"
#include "temp_sensor.h"
#include "pump_control.h"
#include "fan_control.h"
#include "dio_manager.h"
#include "state_machine.h"

#define LOOP_DELAY_NS (100000000)
#define MIN_SET_POINT (25.0f)
#define MAX_SET_POINT (40.0f)

typedef struct {
    float setpoint;
    float kp;
    float ki;
    float kd;
} cmdOptions_t;

static cmdOptions_t cmdInstance;
static cmdOptions_t* options = &cmdInstance;
static bool running = true;

void print_usage(const char* program_name);
int parse_arguments(int argc, char* argv[]);
void signal_handler(int signal);

/*
 * Signal handler for graceful shutdown
 */
void signal_handler(int signal)
{
    (void)signal;
    printf("\nReceived shutdown signal. Stopping system...\n");
    running = false;
}

/*
 * Print usage information
 */
void print_usage(const char* program_name)
{
    printf("Usage: %s <setpoint> [kp] [ki] [kd]\n", program_name);
    printf("  setpoint: Target value (required)\n");
    printf("  kp:       Proportional gain (optional, default: %.2f)\n", PID_DEFAULT_KP);
    printf("  ki:       Integral gain (optional, default: %.2f)\n", PID_DEFAULT_KI);
    printf("  kd:       Derivative gain (optional, default: %.2f)\n", PID_DEFAULT_KD);
    printf("\nExamples:\n");
    printf("  %s 100.0\n", program_name);
    printf("  %s 100.0 2.0\n", program_name);
    printf("  %s 100.0 2.0 0.5 0.1\n", program_name);
}

/*
 * Parse command line arguments
 */
int parse_arguments(int argc, char* argv[])
{
    if (argc < 2) {
        printf("Error: Setpoint is required\n");
        return 0;
    }
    
    if (argc > 5) {
        printf("Error: Too many arguments\n");
        return 0;
    }
    
    options->setpoint = PID_DEFAULT_SETPOINT;
    options->kp = PID_DEFAULT_KP;
    options->ki = PID_DEFAULT_KI;
    options->kd = PID_DEFAULT_KD;

    char* endptr;
    options->setpoint = strtod(argv[1], &endptr);
    if (*endptr != '\0') {
        printf("Error: Invalid setpoint value '%s'\n", argv[1]);
        return 0;
    }
    if ((options->setpoint > MAX_SET_POINT) || (options->setpoint < MIN_SET_POINT)) {
        printf("Error: Setpoint value out of limits. '%s'\n", argv[1]);
        return 0;
    }
    if (!pid_setSetpoint(options->setpoint)) {
        return 0;
    }
    
    if (argc >= 3) {
        options->kp = strtod(argv[2], &endptr);
        if (*endptr != '\0') {
            printf("Error: Invalid Kp value '%s'\n", argv[2]);
            return 0;
        }
        if (options->kp < 0) {
            printf("Error: Kp has negative value '%s'\n", argv[2]);
            return 0;
        }
    }
    
    if (argc >= 4) {
        options->ki = strtod(argv[3], &endptr);
        if (*endptr != '\0') {
            printf("Error: Invalid Ki value '%s'\n", argv[3]);
            return 0;
        }
        if (options->ki < 0) {
            printf("Error: Ki has negative value '%s'\n", argv[3]);
            return 0;
        }
    }
    
    if (argc >= 5) {
        options->kd = strtod(argv[4], &endptr);
        if (*endptr != '\0') {
            printf("Error: Invalid Kd value '%s'\n", argv[4]);
            return 0;
        }
        if (options->kd < 0) {
            printf("Error: Kd has negative value '%s'\n", argv[4]);
            return 0;
        }
    }

    if (!pid_setGains(options->kp, options->ki, options->kd)) {
        return 0;
    }
    
    return 1;
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
    struct timespec timer = {0, LOOP_DELAY_NS};
    
    printf("Cooling System\n");
    printf("================================\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (!parse_arguments(argc, argv)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("Configuration:\n");
    printf("  Setpoint: %.2fÂ°C\n", options->setpoint);
    printf("  PID Gains: Kp=%.3f, Ki=%.3f, Kd=%.3f\n", 
           options->kp, options->ki, options->kd);
    printf("\n");
        
    while (running) {
        sm_update();
        canManager_processMessages();
        canManager_periodicSend();
        nanosleep(&timer, NULL);
    }
    
    printf("\nShutdown complete.\n");
    return EXIT_SUCCESS;
}