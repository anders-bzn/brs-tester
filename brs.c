#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "hal.h"
#include "vector.h"
#include "tests.h"


void print_usage(char *name)
{
	printf("USAGE: %s\n", name);
	printf("      --help                - This help\n");
	printf("      --selftest            - Run hardware test\n");
	printf("      --init                - init hardware\n");
	printf("   -b --board <filename>    - Test board with testvector in <filename>\n");
	printf("   -l --loop <num of loops> - Loop test\n");
}


int main (int argc, char *argv[])
{
	int loops=1;

	vector_initVectors();

	/*
	 * This is really just debug
	 */
	for (int i = 0; i < argc; i++){
		printf("ARG[%d] %s\n",i, argv[i]);
	}

	/*
	 * Take care of options that should be ran exclusive
	 */
	if (argc == 2){
		if (0 == strcmp("--init", argv[1])) {
			if (hal_setup() < 0) {
				printf("ERROR: hal_setup() failed\n");
				return -1;
			}
			hal_setDefault();
			hal_powerEnable(0);
			hal_teardown();
		}

		if ( 0 == strcmp("--selftest", argv[1])){
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
			return 0; // End execution here
		}
	}

	int i = 0;
	while (i < argc){
		if ( 0 == strcmp("--help", argv[i])){
			print_usage(argv[0]);
		}

		if ( 0 == strcmp("--power-enable=on", argv[i]) && argc == 2){
			/*
			 * Power on
			 */
			if (hal_setup() < 0) {
				printf("ERROR: hal_setup() failed\n");
				return -1;
			}
			hal_setDefault();
			hal_powerEnable(1);
			hal_teardown();
			return 0;
		}

		if ( 0 == strcmp("--power-enable=off", argv[i]) && argc == 2){
			/*
			 * Power off
			 */
			if (hal_setup() < 0) {
				printf("ERROR: hal_setup() failed\n");
				return -1;
			}
			hal_setDefault();
			hal_powerEnable(0);
			hal_teardown();
			return 0;
		}

		if ( 0 == strcmp("--toggle>", argv[i])){
			// Set one pin, an toggle it number of loops!
			// Test to measure the speed
			hal_setDefault();
			pin_setFunction(AE, PIN_OUTPUT);
			pin_setMeasure(AE, 1);
			pin_toggleData(AE, loops);
		}

		if (( 0 == strcmp("--loop", argv[i]) && (argc - 1) > i) ||
		    ( 0 == strcmp("-l", argv[i]) && (argc - 1) > i)){
			if (sscanf(argv[i+1], "%d", &loops) != 1){
				if (hal_setup() < 0) {
					printf("ERROR: hal_setup() failed\n");
					return -1;
				}

				printf ("Error: Number of loops %s\n", argv[i+1]);
				return -1;
			}
		}

		if (( 0 == strcmp("-b", argv[i]) && (argc - 1) > i) ||
		    ( 0 == strcmp("--board", argv[i]) && (argc - 1) > i)){
			struct config *board_config = vector_allocConfig();
			if (0 > vector_loadVectors(argv[i+1], board_config)) {
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

			if (0 > tests_checkVoltages(board_config)) {
				printf("ERROR: Execution halted due to voltage outside limits\n");
				hal_powerEnable(0);
				vector_freeVectors();
				free(board_config);
				hal_setDefault();
				hal_teardown();
				return -1;
			}

			tests_setupBoard(board_config);
			tests_checkPullDown(board_config);

			int l=0;
			while (l < loops) {
				int k=0;
				while(vectors[k] != NULL) {
					tests_checkLogic(board_config, vectors[k]);
					k++;
				}
				l++;
			 }

			hal_powerEnable(0);
			vector_freeVectors();
			free(board_config);
			hal_setDefault();
			hal_teardown();
		}
		i++;
	}

	return 0;
}

/*

 TODO: list of things to implement
 ---------------------------------
 [ ] Test input current
 [ ] Test output drive strengt
 [x] Loop testing
 [x] Free testvectors
 [ ] What and where should result be printed
 [x] udev rule to initiate board and export gpios
 [ ] Single step testing
 [x] Validate test vectors more (validate characters)
 [x] Move out test functions to own file
 [x] Let Makefile install everything?
 [ ] Parse first, then execute
 [x] udev does not run the script. Absoulte path?
 [ ] Break on failures
 [x] Load and verify vectors in own file
 [x] Clean up hal.c (return when error, handle errors)

*/
