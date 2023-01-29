#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hal.h"


int main (int argc, char *argv[]) 
{
	int data = 0;
	char *str = NULL;
	
	for (int i = 0; i < argc; i++){
		printf("ARG[%d] %s\n",i, argv[i]);
	} 
		
//	exit (0);
	
	hal_setup();

//  dump_pinSettings();

	// For the time being, HW bug.
	hal_powerEnable(0);

	pin_setFunction(AD, PIN_OUTPUT);
	pin_setFunction(AE, PIN_INPUT);
	pin_enablePullDown(AE, 1);
	pin_setMeasure(AE, 1);

	float voltage,current;

//  while (1) {
		pin_setDataOut(AD, 1);
		usleep(100000);
		pin_getValue(AE, &data);
		hal_measureVoltage (&voltage);
		printf("AD 1, AE %d (%.1f mV)\n", data, voltage);

		pin_setDataOut(AD, 0);
		usleep(100000);
		pin_getValue(AE, &data);
		hal_measureVoltage (&voltage);

		
		
		pin_getName(AD, &str);
		
		printf("- %s 0, AE %d (%.1f mV)\n", str, data, voltage);
//  }

	pin_setMeasure(AE, 0);
	pin_setMeasure(AF, 1);
	pin_setFunction(AF, PIN_INPUT);
	usleep(100000);
	pin_getValue(AF, &data);
	hal_measureVoltage (&voltage);
	printf("AF %d (%.1f mV)\n", data, voltage);

	pin_setMeasure(AF, 0);
	pin_setMeasure(AK, 1);
	pin_setFunction(AK, PIN_INPUT);
	usleep(100000);
	pin_getValue(AK, &data);
	hal_measureVoltage (&voltage);
	hal_measureCurrent (&current);
	printf("AK %d (%.1f mV) (%.1f mA)\n", data, voltage, current);
	usleep(2000000);

	pin_setGnd(AK, 1);
	usleep(100000);
	pin_getValue(AK, &data);
	hal_measureVoltage (&voltage);
	hal_measureCurrent (&current);
	printf("AK %d (%.1f mV) (%.1f mA)\n", data, voltage, current);
	usleep(2000000);

	for (int curr = 0; curr < 32; curr++) {
		printf("Load %d mA\n", curr);
		hal_enableLoad(curr);
		usleep(100000);
		hal_measureCurrent (&current);
		printf("Load: (%.1f mA)\n", current);
		usleep(300000);
	}

	printf("Load 0 mA\n");
	hal_enableLoad(0);
	usleep(100000);
	hal_measureCurrent (&current);
	printf("Load: (%.1f mA)\n", current);
	usleep(10000000);

	hal_setup();
	hal_powerEnable(1);
	
	return 0;
}
