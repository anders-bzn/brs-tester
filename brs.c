#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "hal.h"

int selfTest(void) {
	float voltage_ref;
	int voltage_ref_ok = 0;
	char *str = NULL;

	hal_powerEnable(1);
	hal_setDefault();

	/*
	 * First measure referece voltage, it sould be 1700 mV nominal
	 */
	hal_measureVoltageRef(&voltage_ref);

	if (fabs(voltage_ref - 1700.0) < 15) {
		voltage_ref_ok = 1;
	}

	printf("Reference voltage: %5.1f mV (1700.0 mV)                               [ %s ]\n", voltage_ref, voltage_ref_ok ? " OK " : "FAIL");

	/*
	 * Loop through all PINS and test them, high, low and with load.
	 */
	for (int k = AA; k < LAST_PIN; k++){
		enum pinFunction function;

		pin_getFunction(k, &function);
		pin_getName(k, &str);

		if (function == PIN_POWER) {
			printf("PIN %s, PIN_POWER                                                      [  --  ]\n", str);
		} else {
			int voltage_ok = 0;
			int current_ok = 0;
			float current;
			int data_h, data_l;
			float voltage_h, voltage_l;

			pin_setFunction(k, PIN_OUTPUT);

			/*
			 * Set "1" and measure
			 */
			pin_setDataOut(k, 1);
			pin_setMeasure(k, 1);
			usleep(10000);
			pin_getValue(k, &data_h);
			hal_measureVoltage (&voltage_h);

			/*
			 * Set "0" and measure
			 */
			pin_setDataOut(k, 0);
			usleep(10000);
			pin_getValue(k, &data_l);
			hal_measureVoltage (&voltage_l);

			/*
			 * Test to measure with load (check measurement resistance)
			 */
			hal_enableLoad(4);
			usleep(10000);
			hal_measureCurrent (&current);
			hal_enableLoad(0);

			if ( fabs(voltage_l) < 5.0 && fabs(voltage_h + 3700.0 ) < 50.0) {
				voltage_ok = 1;
			}

			if ( fabs(current + 4.0) < 0.1){
				current_ok = 1;
			}

			printf("PIN %s, D '1':%d (%7.1f mV), D '0':%d (%7.1f mV) I -4 mA: %7.1f mA [ %s ]\n",
				str, data_h, voltage_h, data_l, voltage_l, current, (!data_l && data_h && voltage_ok && current_ok) ? " OK " : "FAIL");
			pin_setMeasure(k, 0);
			pin_setFunction(k, PIN_DISABLED);
		}
	}

	/*
	 * Test the programmable load. Use PIN AE. If this pin has failed
	 * the test above, then this test will fail.
	 */
	pin_setFunction(AE, PIN_OUTPUT);
	pin_setDataOut(AE, 0);
	pin_setMeasure(AE, 1);

	for (int curr = 2, i = 0; curr < 128; curr = (curr << 1), i++) {
		float current;
		int current_ok = 0;
		const float currents[] = {-2.0, -3.9, -7.8, -15.2, -29.0, -52.9};

		hal_enableLoad(curr);
		usleep(10000);
		hal_measureCurrent (&current);

		if (fabs(currents[i] - current) < 0.1){
			current_ok = 1;
		}

		printf("Load: %.2d Meas: (%5.1f mA) (%5.1f mA)                                  [ %s ]\n",
			curr * -1, current, currents[i], (current_ok) ? " OK " : "FAIL");
	}

	pin_setFunction(AE, PIN_DISABLED);
	pin_setMeasure(AE, 0);
	hal_setDefault();
	hal_powerEnable(0);
	return 0;
}


int setupBoard (char *str){

	/* Setup string can look like this:
	 * pppiodiodiodiodiod------------------
	 *
	 * 'p' - power pin, do nothing
	 * 'i' - input pin on testet board
	 * 'o' - output pin on tested board
	 * 'd' - pull down net on tested board
	 * 'g' - pin should be grounded on tested board
	 * '-' - pin should not be used
	 */

	for (int pin = AA; pin < LAST_PIN; pin ++) {
		switch (str[pin]) {
		case 'p':
			printf("P");
			break;
		case '-':
			printf("-");
			break;
		case 'i':
			/*
			 * NOTE: PIN_OUTPUT is from the testers view.
			 * The format string descripbes the board under
			 * test.
			 */
			printf("I");
			pin_setFunction(pin, PIN_OUTPUT);
			break;
		case 'o':
			/*
			 * NOTE: PIN_INPUT is from the testers view.
			 * The format string descripbes the board under
			 * test.
			 */
			printf("O");
			pin_setFunction(pin, PIN_INPUT);
			pin_enablePullDown(pin, 1);
			break;
		case 'd':
			/*
			 * NOTE: PIN_INPUT is from the testers view.
			 * The format string descripbes the board under
			 * test. Setup pull down as input???
			 */
			printf("D");
			pin_setFunction(pin, PIN_INPUT);
			break;
		case 'g':
			/*
			 * NOTE: Pin should be tied to ground dring test
			 */
			printf("G");
			pin_setFunction(pin, PIN_GND);
			break;
		default:
			printf("ERROR: Format error\n");
			return -1;
		}
	}
	printf("\n");
	return 0;
}

int checkVoltages(char *str){
	float voltage;

	/* Setup string can look like this:
	 * pppiodiodiodiodiod------------------
	 *
	 * 'p' - power pin, do nothing
	 * 'i' - input pin on testet board
	 * 'o' - output pin on tested board
	 * 'd' - pull down net on tested board
	 * 'g' - pin should be grounded on tested board
	 * '-' - pin should not be used
	 */

	for (int pin = AA; pin < LAST_PIN; pin ++) {
		int voltage_ok = 0;
		char *pinName;

		pin_getName(pin, &pinName);

		switch (str[pin]) {
		case 'p':
		case 'g':
		case '-':
			printf("PIN: %s %c\n", pinName, str[pin]);
			break;
		case 'i':
		case 'o':
		case 'd':
			/*
			 * Check that voltage are in valid range
			 */
			pin_setMeasure(pin, 1);
			usleep(50000);
			hal_measureVoltage(&voltage);
			pin_setMeasure(pin, 0);

			if ( voltage < 10.0 && voltage > -3800.0){
				voltage_ok = 1;
			}

			printf("PIN: %s %c voltage %7.1f [ %s ]\n", pinName, str[pin], voltage, voltage_ok ? " OK " : "FAIL");
			break;
		default:
			printf("ERROR: Format error\n");
			return -1;
		}
	}
	printf("\n");
	return 0;
}

int checkPullDown(char *str){
	float voltage_l;
	float voltage_h;
	float current;

	/* Setup string can look like this:
	 * pppiodiodiodiodiod------------------
	 *
	 * 'p' - power pin, do nothing
	 * 'i' - input pin on testet board
	 * 'o' - output pin on tested board
	 * 'd' - pull down net on tested board
	 * 'g' - pin should be grounded on tested board
	 * '-' - pin should not be used
	 */

	for (int pin = AA; pin < LAST_PIN; pin ++) {
		int voltage_ok = 0;
		int current_ok = 0;
		char *pinName;

		pin_getName(pin, &pinName);

		switch (str[pin]) {
		case 'p':
		case 'g':
		case '-':
		case 'i':
		case 'o':
//			printf("-", pinName, str[pin]);
			break;
		case 'd':
			/*
			 * Check that pull down works
			 */
			pin_setMeasure(pin, 1);
			usleep(50000);
			hal_measureVoltage(&voltage_h);
			pin_setGnd(pin, 1);
			usleep(100000);
			hal_measureVoltage(&voltage_l);
			hal_measureCurrent(&current);
			pin_setGnd(pin, 0);
			pin_setMeasure(pin, 0);

			if ( fabs(voltage_l + 100.0) < 50.0 && fabs(voltage_h + 3700) < 100){
				voltage_ok = 1;
			}

			if ( fabs(current + 10.0) < 0.6) {
				current_ok = 1;
			}

			printf("PIN: %s %c voltage %7.1f %7.1f %7.1f [ %s ]\n", pinName, str[pin], voltage_l, voltage_h, current, (voltage_ok && current_ok) ? " OK " : "FAIL");
			break;
		default:
			printf("ERROR: Format error\n");
			return -1;
		}
	}
	printf("\n");
	return 0;
}












int checkLogic(char *setup, char *vector){

	/* Setup string can look like this:
	 * pppiodiodiodiodiod------------------
	 *
	 * 'p' - power pin, do nothing
	 * 'i' - input pin on testet board
	 * 'o' - output pin on tested board
	 * 'd' - pull down net on tested board
	 * 'g' - pin should be grounded on tested board
	 * '-' - pin should not be used
	 */

	for (int pin = AA; pin < LAST_PIN; pin ++) {
		switch (setup[pin]) {
		case 'p':
		case 'g':
		case '-':
		case 'd':
		case 'o':
			printf("%c", vector[pin]);
			break;

		case 'i':
			/*
			 * Set output
			 */
			pin_setDataOut(pin, vector[pin] - '0');
			printf("%c", vector[pin]);
			break;
		default:
			printf("ERROR: Format error\n");
			return -1;
		}
	}
	printf("\n");

	for (int pin = AA; pin < LAST_PIN; pin ++) {
		int data_in;

		switch (setup[pin]) {
		case 'p':
		case 'g':
		case '-':
		case 'i':
			printf("-");
			break;

		case 'd':
		case 'o':
			/*
			 * Get data in
			 */
			pin_getValue(pin, &data_in);
			printf("%d", data_in);
			break;
		default:
			printf("ERROR: Format error\n");
			return -1;
		}
	}
	printf("\n");

	return 0;
}


int main (int argc, char *argv[])
{

	for (int i = 0; i < argc; i++){
		printf("ARG[%d] %s\n",i, argv[i]);
	}

	if (hal_setup() < 0) {
		printf("ERROR: hal_setup() failed\n");
		return -1;
	}

	int i = 0;

	while (i < argc){
		if ( 0 == strcmp("selftest", argv[i])){
			selfTest();
		}

		if ( 0 == strcmp("test", argv[i])){
			hal_powerEnable(1);
			usleep(1000000);
			checkVoltages("pppiodiodiodiodiod------------------");
			setupBoard("pppiodiodiodiodiod------------------");
			checkPullDown("pppiodiodiodiodiod------------------");
			checkLogic("pppiodiodiodiodiod------------------","---011011011011011------------------");
			checkLogic("pppiodiodiodiodiod------------------","---101101101101101------------------");
			checkLogic("pppiodiodiodiodiod------------------","---011101101101101------------------");
			checkLogic("pppiodiodiodiodiod------------------","---101011101101101------------------");
			checkLogic("pppiodiodiodiodiod------------------","---101101011101101------------------");
			checkLogic("pppiodiodiodiodiod------------------","---101101101011101------------------");
			checkLogic("pppiodiodiodiodiod------------------","---101101101101011------------------");
			usleep(100000);
			hal_powerEnable(0);
		}
		i++;
	}

	hal_setDefault();
	hal_powerEnable(0);
	hal_teardown();
	return 0;
}

