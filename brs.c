/*
 * BRS-tester by Anders Sandahl 2023-2024
 *
 * License GPL 2.0
 *
 * brs.c: Main function and argument parsing
 *
 */
#include <argp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "hal.h"
#include "vector.h"
#include "tests.h"


enum cmd {
    CMD_NONE = 0,
    CMD_INIT,
    CMD_TEST,
    CMD_SELFTEST,
    CMD_DEBUG
};


enum pwr {
    PWR_NONE,
    PWR_ENABLE,
    PWR_DISABLE
};


const char *argp_program_version =
    "brs_tester 0.1";


const char *argp_program_bug_address =
    "<anders@abc80.net>";


/* Program documentation. */
static char doc[] =
    "Program that runs the brs_tester.";


/* The options we understand. */
static struct argp_option options[] = {
    {"vector",       'v', "FILE",     OPTION_ARG_OPTIONAL, "Test vector to run [test]"},
    {"loop",         'l', "NUMBER",   OPTION_ARG_OPTIONAL, "Number of iterations, doing loop testing [test]"},
    {"power-enable", 'P', "on/off",   OPTION_ARG_OPTIONAL, "Manual power control [debug]"},
    {"pin",          'p', "PIN",      OPTION_ARG_OPTIONAL, "Manual pin manipulation [debug]"},
    {"pin-state",    's', "1/0/T/L",  OPTION_ARG_OPTIONAL, "Manual pin state [debug]"},
    {"load",         'L', "NUMBER",   OPTION_ARG_OPTIONAL, "Manual setting load on a pin [debug]"},
    {"sigle-step",   'S', "",         OPTION_ARG_OPTIONAL, "Wait for user input before next test step"},
    { 0 }
};


/* Used by main to communicate with parse_opt. */
struct argp_arguments
{
    enum cmd command;
    char *vector_file;
    char pinstate;
    int pin;
    enum pwr power;
    int output_drive_strength;
    int loops;
    bool single_step;
};


/* Parse option by option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
    know is a pointer to our arguments structure. */
    struct argp_arguments *arguments = state->input;

    switch (key){
    case 'v':
        arguments->vector_file = arg;
        break;
    case 'l':
        if (arg != NULL) {
            arguments->loops = atoi(arg);
        } else {
            argp_usage(state);
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 'L':
        if (arg != NULL) {
            arguments->output_drive_strength = atoi(arg);
        } else {
            argp_usage(state);
            return ARGP_ERR_UNKNOWN;
        }
    case 'P':
        if (arg != NULL && ((0 == strncmp(arg, "on", sizeof("on") -1)))) {
            arguments->power = PWR_ENABLE;
        } else if (arg != NULL && ((0 == strncmp(arg, "off", sizeof("off") -1)))) {
            arguments->power = PWR_DISABLE;
        } else {
            argp_usage (state);
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 'p':
        if (arg != NULL) {
            int pin = pin_getIndex(arg);

            if (pin == -1 || pin == 0 || pin == 1 || pin == 2 ||
                pin == 18 || pin == 19 || pin == 20){
                printf("ERROR: Invalid pin: %s\n", arg);
                argp_usage (state);
                return ARGP_ERR_UNKNOWN;
        } else {
                arguments->pin = pin;
            }
        } else {
            argp_usage (state);
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 's':
        if (arg != NULL && (arg[0] == '0' || arg[0] == '1' || arg[0] == 'T' || arg[0] == 'L')) {
            arguments->pinstate = arg[0];
        } else {
            argp_usage (state);
            return ARGP_ERR_UNKNOWN;
        }
        break;
    case 'S':
            arguments->single_step = true;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num != 0){
            argp_usage (state);
            return ARGP_ERR_UNKNOWN;
        }

        if (0 == strncmp(arg, "test", sizeof("test") -1)) {
            arguments->command = CMD_TEST;
        }
        if (0 == strncmp(arg, "selftest", sizeof("selftest") -1)) {
            arguments->command = CMD_SELFTEST;
        }
        if (0 == strncmp(arg, "init", sizeof("init") -1)) {
            arguments->command = CMD_INIT;
        }
        if (0 == strncmp(arg, "debug", sizeof("debug") -1)) {
            arguments->command = CMD_DEBUG;
        }
        break;
    case ARGP_KEY_END:
        if (state->arg_num != 1){
            argp_usage (state);
            return ARGP_ERR_UNKNOWN;
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


static struct argp argp = { options, parse_opt, NULL, doc };


int main (int argc, char *argv[])
{
    struct argp_arguments args;

    args.command = CMD_NONE;
    args.loops = 1;
    args.vector_file = NULL;
    args.power = PWR_NONE;
    args.pin = -1;
    args.pinstate = '\0';
    args.output_drive_strength = 20;
    args.single_step = false;

    vector_initVectors();

    if (0 > argp_parse (&argp, argc, argv, 0, 0, &args))
        return -1;

    if (args.command == CMD_INIT){
        if (hal_setup() < 0) {
            printf("ERROR: hal_setup() failed\n");
            return -1;
        }
        hal_setDefault();
        hal_powerEnable(0);
        hal_teardown();
        return 0;
    }

    if (args.command == CMD_SELFTEST){
        /*
         * Run selftest
         */
        if (hal_setup() < 0) {
            printf("ERROR: hal_setup() failed\n");
            return -1;
        }
        hal_setDefault();
        tests_selfTest();
        hal_powerEnable(0);
        hal_teardown();
        return 0;
    }

    if (args.command == CMD_TEST) {
        struct config *board_config = vector_allocConfig();
        int l=0;

        if (!args.vector_file) {
            printf("Nothing to do!\n");
            return 0;
        }

        if (0 > vector_loadVectors(args.vector_file, board_config)) {
            vector_freeVectors();
            free(board_config);
            return -1;
        }

        if (hal_setup() < 0) {
            printf("ERROR: hal_setup() failed\n");
            return -1;
        }

        hal_powerEnable(1);
        usleep(1000000);

        printf("TEST: Check voltages\n");
        if (0 > tests_checkVoltages(board_config)) {
            printf("ERROR: Execution halted due to voltage outside limits\n");
            hal_powerEnable(0);
            vector_freeVectors();
            free(board_config);
            hal_setDefault();
            hal_teardown();
            return -1;
        }

        printf("TEST: Setup board\n");
        printf("AAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBB\n");
        printf("ABCDEFHJKLMNPRSTUVABCDEFHJKLMNPRSTUV\n");
        tests_setupBoard(board_config);
        printf("TEST: Check pull downs\n");
        tests_checkPullDown(board_config);

        printf("TEST: Check inputs\n");
        tests_checkInputs(board_config);

        printf("TEST: Check logic\n");
        while (l < args.loops) {
            int k=0;
            printf("AAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBBB\n");
            printf("ABCDEFHJKLMNPRSTUVABCDEFHJKLMNPRSTUV\n");
            while(vectors[k].vector != NULL) {
                if  (vectors[k].type == TYPE_LOGIC) {
                    tests_checkLogic(board_config, vectors[k].vector, args.single_step);
                }
                k++;
            }
            l++;
        }

        printf("\n");
        printf("TEST: Check drive strength\n");
        int k=0;
        while(vectors[k].vector != NULL) {
            if  (vectors[k].type == TYPE_OUTPUT) {
                tests_checkDriveStrength(board_config, vectors[k].vector, args.single_step);
            }
            if (vectors[k].type == TYPE_DEBUG_EXIT) {
                return 0;
            }
            k++;
        }

        hal_powerEnable(0);
        vector_freeVectors();
        free(board_config);
        hal_setDefault();
        hal_teardown();
    }

    if (args.command == CMD_DEBUG) {
        if (hal_setup() < 0) {
            printf("ERROR: hal_setup() failed\n");
            return -1;
        }
        hal_setDefault();
        if (args.power != PWR_NONE) {
            int power = 0;

            if (args.power == PWR_ENABLE) power = 1;
            hal_powerEnable(power);
        }

        if (args.pin != -1 && args.pinstate != '\0') {
            printf("Pin set '%c' %d\n", args.pinstate, args.pin);
            pin_setFunction(args.pin, PIN_OUTPUT);
            pin_setMeasure(args.pin, 1);

           if (args.pinstate == 'T') {
                pin_toggleData(args.pin, args.loops);
            } else if (args.pinstate == '1') {
                pin_setDataOut(args.pin, 1);
            } else if (args.pinstate == '0') {
                pin_setDataOut(args.pin, 0);
            } else if (args.pinstate == 'L') {
                hal_enableLoad(args.output_drive_strength, true);
            }
        }
        hal_teardown();
    }
    return 0;
}
