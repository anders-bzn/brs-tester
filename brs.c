#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hal.h"


int main (int argc, char *argv[]) 
{
	char *str = NULL;
	
	for (int i = 0; i < argc; i++){
		printf("ARG[%d] %s\n",i, argv[i]);
	} 
		
//	exit (0);
	
	if (hal_setup() < 0) {
		printf("ERROR: hal_setup() failed\n");
		return -1;
	} 

//  dump_pinSettings();

	// For the time being, HW bug.
	hal_powerEnable(0);
	hal_setDefault();

	for (int k = AD; k <= AS; k++){
		int data_h, data_l;
		float voltage_h, voltage_l;
		
		pin_setFunction(k, PIN_OUTPUT);
		pin_setDataOut(k, 1);
		pin_setMeasure(k, 1);
		usleep(100000);
		pin_getValue(k, &data_h);
		hal_measureVoltage (&voltage_h);

		pin_setDataOut(k, 0);
		usleep(10000);
		pin_getValue(k, &data_l);
		hal_measureVoltage (&voltage_l);

		pin_getName(k, &str);
		printf("PIN %s, data:%d (%.1f mV), data:%d (%.1f mV)\n", str, data_h, voltage_h, data_l, voltage_l);
		pin_setMeasure(k, 0);
		pin_setFunction(k, PIN_DISABLED);
	}


//	for (int curr = 0; curr < 32; curr++) {
//		hal_enableLoad(curr);
//		usleep(100000);
//		hal_measureCurrent (&current);
//		printf("Load: %d Meas: (%.1f mA)\n", curr, current);
//	}

	hal_powerEnable(1);
	hal_teardown();
	
	return 0;
}
