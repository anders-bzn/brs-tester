#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ugpio/ugpio.h>
#include <errno.h>
#include "hal.h"

/*
 * Definitions for GPIO's
 */

enum gpio_list {
	GPIO_ENABLE_POWER = 0,
	GPIO_SELECT_OUT   = 1,
	GPIO_SELECT_A     = 2,
	GPIO_SELECT_B     = 3,
	GPIO_SELECT_C     = 4,
	GPIO_SELECT_D     = 5,
	GPIO_LATCH_INPUTS = 6,
	GPIO_INP_A_OE     = 7,
	GPIO_INP_B_OE     = 8,
	GPIO_INP_C_OE     = 9,
	GPIO_INP_D_OE     = 10,
	GPIO_D0           = 11,
	GPIO_D1           = 12,
	GPIO_D2           = 13,
	GPIO_D3           = 14,
	GPIO_D4           = 15,
	GPIO_D5           = 16,
	GPIO_D6           = 17,
	GPIO_D7           = 18,
	GPIO_ENABLE_AUX   = 19,
	LAST_GPIO         = 20,
};


struct gpio {
	const int flags;
	const char active_low;
	const char num;
	ugpio_t *io;
};


struct gpio GPIOS[] = {
	[GPIO_ENABLE_POWER].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_ENABLE_POWER].active_low = 0,
	[GPIO_ENABLE_POWER].num = 27,
	[GPIO_ENABLE_POWER].io = NULL,

	[GPIO_SELECT_OUT].flags = GPIOF_OUT_INIT_HIGH,
	[GPIO_SELECT_OUT].active_low = 1,
	[GPIO_SELECT_OUT].num = 24,
	[GPIO_SELECT_OUT].io = NULL,

	[GPIO_SELECT_A].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_SELECT_A].active_low = 0,
	[GPIO_SELECT_A].num = 25,
	[GPIO_SELECT_A].io = NULL,

	[GPIO_SELECT_B].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_SELECT_B].active_low = 0,
	[GPIO_SELECT_B].num = 8,
	[GPIO_SELECT_B].io = NULL,

	[GPIO_SELECT_C].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_SELECT_C].active_low = 0,
	[GPIO_SELECT_C].num = 7,
	[GPIO_SELECT_C].io = NULL,

	[GPIO_SELECT_D].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_SELECT_D].active_low = 0,
	[GPIO_SELECT_D].num = 1,
	[GPIO_SELECT_D].io = NULL,

	[GPIO_LATCH_INPUTS].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_LATCH_INPUTS].active_low = 0,
	[GPIO_LATCH_INPUTS].num = 12,
	[GPIO_LATCH_INPUTS].io = NULL,

	[GPIO_INP_A_OE].flags = GPIOF_OUT_INIT_HIGH,
	[GPIO_INP_A_OE].active_low = 1,
	[GPIO_INP_A_OE].num = 23,
	[GPIO_INP_A_OE].io = NULL,

	[GPIO_INP_B_OE].flags = GPIOF_OUT_INIT_HIGH,
	[GPIO_INP_B_OE].active_low = 1,
	[GPIO_INP_B_OE].num = 0,
	[GPIO_INP_B_OE].io = NULL,

	[GPIO_INP_C_OE].flags = GPIOF_OUT_INIT_HIGH,
	[GPIO_INP_C_OE].active_low = 1,
	[GPIO_INP_C_OE].num = 4,
	[GPIO_INP_C_OE].io = NULL,

	[GPIO_INP_D_OE].flags = GPIOF_OUT_INIT_HIGH,
	[GPIO_INP_D_OE].active_low = 1,
	[GPIO_INP_D_OE].num = 10,
	[GPIO_INP_D_OE].io = NULL,

	[GPIO_D0].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D0].active_low = 0,
	[GPIO_D0].num = 5,
	[GPIO_D0].io = NULL,

	[GPIO_D1].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D1].active_low = 0,
	[GPIO_D1].num = 6,
	[GPIO_D1].io = NULL,

	[GPIO_D2].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D2].active_low = 0,
	[GPIO_D2].num = 13,
	[GPIO_D2].io = NULL,

	[GPIO_D3].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D3].active_low = 0,
	[GPIO_D3].num = 19,
	[GPIO_D3].io = NULL,

	[GPIO_D4].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D4].active_low = 0,
	[GPIO_D4].num = 26,
	[GPIO_D4].io = NULL,

	[GPIO_D5].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D5].active_low = 0,
	[GPIO_D5].num = 21,
	[GPIO_D5].io = NULL,

	[GPIO_D6].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D6].active_low = 0,
	[GPIO_D6].num = 20,
	[GPIO_D6].io = NULL,

	[GPIO_D7].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_D7].active_low = 0,
	[GPIO_D7].num = 16,
	[GPIO_D7].io = NULL,

	[GPIO_ENABLE_AUX].flags = GPIOF_OUT_INIT_LOW,
	[GPIO_ENABLE_AUX].active_low = 0,
	[GPIO_ENABLE_AUX].num = 22,
	[GPIO_ENABLE_AUX].io = NULL,
};


/*
 * Definitions that describes the hardware of the tester
 */

/*
		Data out for level shifter
		------
		| D0 | Measurement 1 enable (active high)
		------
		| D1 | Measurement 2 enable (active high)
		------
		| D2 | Pin 1 enable (active high)
		------
		| D3 | Pin 2 enable (active high)
		------
		| D4 | Pull down 1 (active low)
		------
		| D5 | Data out 1 (active low)
		------
		| D6 | Pull down 2 (active low)
		------
		| D7 | Data out 2 (active low)
		------
*/

struct board {
	int address;
	union {
	struct {
		char meas1:1;
		char meas2:1;
		char pin1:1;
		char pin2:1;
		char pd1:1;
		char do1:1;
		char pd2:1;
		char do2:1;
	} level;
	struct {
		char current:6;
		char enable:1;
	} load;
	unsigned char data;
	} data;
};

/*
 * Total number of boards in the backplane. 15 levelshifters and one
 * board for loading outputs on tested flipchips
 */
#define NUM_BOARDS 16
#define LOAD_BOARD_ADR 15
/*
 * All boards in an array
 */
static struct board boards[NUM_BOARDS];

/*
 * Settings for all pins from a flipchip
 */
struct pinSetting {
	const int pinNumber;
	const char *name;
	struct board *b;
	const int bitOffset;
	const int inputBitMask;
	enum pinFunction function;
};

static const char *pinFunctionStr[] = {
	[PIN_POWER]	= "POWER",
	[PIN_GND]	= "GND",
	[PIN_INPUT]	= "INPUT",
	[PIN_OUTPUT]	= "OUTPUT",
	[PIN_DISABLED]	= "DISABLED"
};


/*
 * Mapping between PINS on a flipchip to backplane levelshivers. Not
 * All pins are possible to test, like power and ground. But have them
 * in the list anyway.
 */
static struct pinSetting pinSettings[] = {
	[AA].pinNumber = AA,
	[AA].name = "AA",
	[AA].b = NULL,
	[AA].bitOffset = -1,
	[AA].inputBitMask = -1,
	[AA].function = PIN_POWER,
	[AB].pinNumber = AB,
	[AB].name = "AB",
	[AB].b = NULL,
	[AB].bitOffset = -1,
	[AB].inputBitMask = -1,
	[AB].function = PIN_POWER,
	[AC].pinNumber = AC,
	[AC].name = "AC",
	[AC].b = NULL,
	[AC].bitOffset = -1,
	[AC].inputBitMask = -1,
	[AC].function = PIN_POWER,
	[AD].pinNumber = AD,
	[AD].name = "AD",
	[AD].b = &(boards[0]),
	[AD].bitOffset = 1,
	[AD].inputBitMask = 1 << 1,
	[AD].function = PIN_DISABLED,
	[AE].pinNumber = AE,
	[AE].name = "AE",
	[AE].b = &(boards[0]),
	[AE].bitOffset = 2,
	[AE].inputBitMask = 1 << 0,
	[AE].function = PIN_DISABLED,
	[AF].pinNumber = AF,
	[AF].name = "AF",
	[AF].b = &(boards[1]),
	[AF].bitOffset = 1,
	[AF].inputBitMask = 1 << 3,
	[AF].function = PIN_DISABLED,
	[AH].pinNumber = AH,
	[AH].name = "AH",
	[AH].b = &(boards[1]),
	[AH].bitOffset = 2,
	[AH].inputBitMask = 1 << 2,
	[AH].function = PIN_DISABLED,
	[AJ].pinNumber = AJ,
	[AJ].name = "AJ",
	[AJ].b = &(boards[2]),
	[AJ].bitOffset = 1,
	[AJ].inputBitMask = 1 << 5,
	[AJ].function = PIN_DISABLED,
	[AK].pinNumber = AK,
	[AK].name = "AK",
	[AK].b = &(boards[2]),
	[AK].bitOffset = 2,
	[AK].inputBitMask = 1 << 4,
	[AK].function = PIN_DISABLED,
	[AL].pinNumber = AL,
	[AL].name = "AL",
	[AL].b = &(boards[3]),
	[AL].bitOffset = 1,
	[AL].inputBitMask = 1 << 7,
	[AL].function = PIN_DISABLED,
	[AM].pinNumber = AM,
	[AM].name = "AM",
	[AM].b = &(boards[3]),
	[AM].bitOffset = 2,
	[AM].inputBitMask = 1 << 6,
	[AM].function = PIN_DISABLED,
	[AN].pinNumber = AN,
	[AN].name = "AN",
	[AN].b = &(boards[4]),
	[AN].bitOffset = 1,
	[AN].inputBitMask = 1 << 9,
	[AN].function = PIN_DISABLED,
	[AP].pinNumber = AP,
	[AP].name = "AP",
	[AP].b = &(boards[4]),
	[AP].bitOffset = 2,
	[AP].inputBitMask = 1 << 8,
	[AP].function = PIN_DISABLED,
	[AR].pinNumber = AR,
	[AR].name = "AR",
	[AR].b = &(boards[5]),
	[AR].bitOffset = 1,
	[AR].inputBitMask = 1 << 11,
	[AR].function = PIN_DISABLED,
	[AS].pinNumber = AS,
	[AS].name = "AS",
	[AS].b = &(boards[5]),
	[AS].bitOffset = 2,
	[AS].inputBitMask = 1 << 10,
	[AS].function = PIN_DISABLED,
	[AT].pinNumber = AT,
	[AT].name = "AT",
	[AT].b = &(boards[6]),
	[AT].bitOffset = 1,
	[AT].inputBitMask = 1 << 13,
	[AT].function = PIN_DISABLED,
	[AU].pinNumber = AU,
	[AU].name = "AU",
	[AU].b = &(boards[6]),
	[AU].bitOffset = 2,
	[AU].inputBitMask = 1 << 12,
	[AU].function = PIN_DISABLED,
	[AV].pinNumber = AV,
	[AV].name = "AV",
	[AV].b = &(boards[7]),
	[AV].bitOffset = 1,
	[AV].inputBitMask = 1 << 15,
	[AV].function = PIN_DISABLED,
	[BA].pinNumber = BA,
	[BA].name = "BA",
	[BA].b = NULL,
	[BA].bitOffset = -1,
	[BA].inputBitMask = -1,
	[BA].function = PIN_POWER,
	[BB].pinNumber = BB,
	[BB].name = "BB",
	[BB].b = NULL,
	[BB].bitOffset = -1,
	[BB].inputBitMask = -1,
	[BB].function = PIN_POWER,
	[BC].pinNumber = BC,
	[BC].name = "BC",
	[BC].b = NULL,
	[BC].bitOffset = -1,
	[BC].inputBitMask = -1,
	[BC].function = PIN_POWER,
	[BD].pinNumber = BD,
	[BD].name = "BD",
	[BD].b = &(boards[7]),
	[BD].bitOffset = 2,
	[BD].inputBitMask = 1 << 14,
	[BD].function = PIN_DISABLED,
	[BE].pinNumber = BE,
	[BE].name = "BE",
	[BE].b = &(boards[8]),
	[BE].bitOffset = 1,
	[BE].inputBitMask = 1 << 17,
	[BE].function = PIN_DISABLED,
	[BF].pinNumber = BF,
	[BF].name = "BF",
	[BF].b = &(boards[8]),
	[BF].bitOffset = 2,
	[BF].inputBitMask = 1 << 16,
	[BF].function = PIN_DISABLED,
	[BH].pinNumber = BH,
	[BH].name = "BH",
	[BH].b = &(boards[9]),
	[BH].bitOffset = 1,
	[BH].inputBitMask = 1 << 19,
	[BH].function = PIN_DISABLED,
	[BJ].pinNumber = BJ,
	[BJ].name = "BJ",
	[BJ].b = &(boards[9]),
	[BJ].bitOffset = 2,
	[BJ].inputBitMask = 1 << 18,
	[BJ].function = PIN_DISABLED,
	[BK].pinNumber = BK,
	[BK].name = "BK",
	[BK].b = &(boards[10]),
	[BK].bitOffset = 1,
	[BK].inputBitMask = 1 << 21,
	[BK].function = PIN_DISABLED,
	[BL].pinNumber = BL,
	[BL].name = "BL",
	[BL].b = &(boards[10]),
	[BL].bitOffset = 2,
	[BL].inputBitMask = 1 << 20,
	[BL].function = PIN_DISABLED,
	[BM].pinNumber = BM,
	[BM].name = "BM",
	[BM].b = &(boards[11]),
	[BM].bitOffset = 1,
	[BM].inputBitMask = 1 << 23,
	[BM].function = PIN_DISABLED,
	[BN].pinNumber = BN,
	[BN].name = "BN",
	[BN].b = &(boards[11]),
	[BN].bitOffset = 2,
	[BN].inputBitMask = 1 << 22,
	[BN].function = PIN_DISABLED,
	[BP].pinNumber = BP,
	[BP].name = "BP",
	[BP].b = &(boards[12]),
	[BP].bitOffset = 1,
	[BP].inputBitMask = 1 << 25,
	[BP].function = PIN_DISABLED,
	[BR].pinNumber = BR,
	[BR].name = "BR",
	[BR].b = &(boards[12]),
	[BR].bitOffset = 2,
	[BR].inputBitMask = 1 << 24,
	[BR].function = PIN_DISABLED,
	[BS].pinNumber = BS,
	[BS].name = "BS",
	[BS].b = &(boards[13]),
	[BS].bitOffset = 1,
	[BS].inputBitMask = 1 << 27,
	[BS].function = PIN_DISABLED,
	[BT].pinNumber = BT,
	[BT].name = "BT",
	[BT].b = &(boards[13]),
	[BT].bitOffset = 2,
	[BT].inputBitMask = 1 << 26,
	[BT].function = PIN_DISABLED,
	[BU].pinNumber = BU,
	[BU].name = "BU",
	[BU].b = &(boards[14]),
	[BU].bitOffset = 1,
	[BU].inputBitMask = 1 << 29,
	[BU].function = PIN_DISABLED,
	[BV].pinNumber = BV,
	[BV].name = "BV",
	[BV].b = &(boards[14]),
	[BV].bitOffset = 2,
	[BV].inputBitMask = 1 << 28,
	[BV].function = PIN_DISABLED,
};


/*
 * This defines to what bit in what byte every inputs PIN is connected
 *
	Byte 0 in         Byte 1 in         Byte 2 in         Byte 3 in
	------            ------            ------            ------
	| D0 | Pin AD     | D0 | Pin AN     | D0 | Pin BE     | D0 | Pin BP
	------            ------            ------            ------
	| D1 | Pin AE     | D1 | Pin AP     | D1 | Pin BF     | D1 | Pin BR
	------            ------            ------            ------
	| D2 | Pin AF     | D2 | Pin AR     | D2 | Pin BH     | D2 | Pin BS
	------            ------            ------            ------
	| D3 | Pin AH     | D3 | Pin AS     | D3 | Pin BJ     | D3 | Pin BT
	------            ------            ------            ------
	| D4 | Pin AJ     | D4 | Pin AT     | D4 | Pin BK     | D4 | Pin BU
	------            ------            ------            ------
	| D5 | Pin AK     | D5 | Pin AU     | D5 | Pin BL     | D5 | Pin BV
	------            ------            ------            ------
	| D6 | Pin AL     | D6 | Pin AV     | D6 | Pin BM     | D6 | GND
	------            ------            ------            ------
	| D7 | Pin AM     | D7 | Pin BD     | D7 | Pin BN     | D7 | GND
	------            ------            ------            ------
*/


/*
 * Enable/disable power to a flipchip
 */
int hal_powerEnable(int on) {
	int ret = ugpio_set_value(GPIOS[GPIO_ENABLE_POWER].io, on);

	if (ret < 0) {
		printf("ERROR: Could not %s power\n",  on ? "enable" : "disable");
	}

	return ret;
}


int hal_measureCurrent(float *iMeas){
	float uRef_scale = 0;
	float uRef_voltage = 0;
	float uRaw_scale = 0;
	float uRaw_voltage = 0;
	int c;

	FILE *fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage3_raw");
		return -1;
	}

	c = fscanf(fp, "%f", &uRef_voltage);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_scale", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage3_scale");
		return -1;
	}

	c = fscanf(fp, "%f", &uRef_scale);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage0_raw");
		return -1;
	}

	c = fscanf(fp, "%f", &uRaw_voltage);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage0_scale", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage0_scale");
		return -1;
	}

	c = fscanf(fp, "%f", &uRaw_scale);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	// R = 10 Ohms, the INA141 has a gain of 10. 1 V equals 10mA, 1mV equals 10uA
	// Result is given in mA.
	*iMeas = (((float)(150.0/12.0) * ((uRaw_scale * uRaw_voltage) - (uRef_scale * uRef_voltage))) + (uRaw_scale * uRaw_voltage))/100;
	return 0;
}

int hal_measureVoltage(float *uMeas){
	float uRef_scale = 0;
	float uRef_voltage = 0;
	float uRaw_scale = 0;
	float uRaw_voltage = 0;
	int c;

	FILE *fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage3_raw");
		return -1;
	}

	c = fscanf(fp, "%f", &uRef_voltage);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_scale", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage3_scale");
		return -1;
	}

	c = fscanf(fp, "%f", &uRef_scale);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage1_raw", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage1_raw");
		return -1;
	}

	c = fscanf(fp, "%f", &uRaw_voltage);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage1_scale", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage1_scale");
		return -1;
	}

	c = fscanf(fp, "%f", &uRaw_scale);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	// Many magical numbers here. The +15/-15 V is divided with 150k and 12K ohm resistor
	// to a reference voltage of about 1.7 V. This means that the voltage range is mapped
	// to a voltage ~0.5 V to 2.7 V to fit in the ADC range. The calulation gets the real
	// voltage from the ADC range. In millivolts!

	*uMeas = ((float)(150.0/12.0) * ((uRaw_scale * uRaw_voltage) - (uRef_scale * uRef_voltage))) + (uRaw_scale * uRaw_voltage);
	return 0;
}


int hal_measureVoltageRef(float *uMeas){
	float uRef_scale = 0;
	float uRef_voltage = 0;
	int c;

	FILE *fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_raw", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage3_raw");
		return -1;
	}

	c = fscanf(fp, "%f", &uRef_voltage);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	fp = fopen("/sys/bus/iio/devices/iio:device0/in_voltage3_scale", "r");

	if (fp == NULL) {
		printf("ERROR: %s\n", "/sys/bus/iio/devices/iio:device0/in_voltage3_scale");
		return -1;
	}

	c = fscanf(fp, "%f", &uRef_scale);
	fclose(fp);

	if (c != 1) {
		return -1;
	}

	*uMeas = (uRef_scale * uRef_voltage);
	return 0;
}


int hal_setOut(int channel, int data)
{
	if (channel < 0 || channel > 15) {
		printf("ERR: invalid board index\n");
		return -1;
	}

	// Set as output and active low for highest bit
	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		if (channel == LOAD_BOARD_ADR) {
			// Load board has other pins as active low/high
			if (i < GPIO_D6) {
				if (ugpio_set_activelow(GPIOS[i].io, 1) < 0){
					return -1;
				}
			} else {
				if (ugpio_set_activelow(GPIOS[i].io, 0) < 0){
					return -1;
				}
			}
		} else {
			if (i < GPIO_D4) {
				if (ugpio_set_activelow(GPIOS[i].io, 0) < 0){
					return -1;
				}
			} else {
				if (ugpio_set_activelow(GPIOS[i].io, 1) < 0){
					return -1;
				}
			}
		}
	}


	ugpio_set_value(GPIOS[GPIO_SELECT_A].io, channel & 1);
	ugpio_set_value(GPIOS[GPIO_SELECT_B].io, (channel >> 1) & 1);
	ugpio_set_value(GPIOS[GPIO_SELECT_C].io, (channel >> 2) & 1);
	ugpio_set_value(GPIOS[GPIO_SELECT_D].io, (channel >> 3) & 1);

	for (int i = 0; i <= 7; i++) {
		ugpio_set_value(GPIOS[i + GPIO_D0].io, (data >> i) & 1);
	}

	ugpio_set_value(GPIOS[GPIO_SELECT_OUT].io, 1);
	usleep(1);
	ugpio_set_value(GPIOS[GPIO_SELECT_OUT].io, 0);
	return 0;
}


int hal_getInputs (int *data) {
	int value=0;
	int inputs=0;

	ugpio_set_value(GPIOS[GPIO_LATCH_INPUTS].io, 1);
	usleep(1);
	ugpio_set_value(GPIOS[GPIO_LATCH_INPUTS].io, 0);

	// Set as input and all as active high
	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		ugpio_direction_input(GPIOS[i].io);
		if (ugpio_set_activelow(GPIOS[i].io, 0) < 0){
			return -1;
		}
	}

	// Enable latch A output
	ugpio_set_value(GPIOS[GPIO_INP_A_OE].io, 1);
	usleep(1);

	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		value = ugpio_get_value(GPIOS[i].io);
		inputs |= ((value & 1) << (i - GPIO_D0));
	}

	// Disable latch A output
	ugpio_set_value(GPIOS[GPIO_INP_A_OE].io, 0);

	// Enable latch B output
	ugpio_set_value(GPIOS[GPIO_INP_B_OE].io, 1);
	usleep(1);

	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		value = ugpio_get_value(GPIOS[i].io);
		inputs |= (value & 1) << (i - GPIO_D0 + 8);
	}

	// Disable latch B output
	ugpio_set_value(GPIOS[GPIO_INP_B_OE].io, 0);

	// Enable latch C output
	ugpio_set_value(GPIOS[GPIO_INP_C_OE].io, 1);
	usleep(1);

	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		value = ugpio_get_value(GPIOS[i].io);
		inputs |= (value & 1) << (i - GPIO_D0 + 16);
	}

	// Disable latch C output
	ugpio_set_value(GPIOS[GPIO_INP_C_OE].io, 0);

	// Enable latch D output
	ugpio_set_value(GPIOS[GPIO_INP_D_OE].io, 1);
	usleep(1);

	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		value = ugpio_get_value(GPIOS[i].io);
		inputs |= (value & 1) << (i - GPIO_D0 + 24);
	}

	// Disable latch D output
	ugpio_set_value(GPIOS[GPIO_INP_D_OE].io, 0);
	*data=inputs;

	// Set as output
	for (int i = GPIO_D0; i <= GPIO_D7; i++) {
		ugpio_direction_output(GPIOS[i].io, 0);
	}
	return 0;
}


int hal_setup(void){
	for (int j = GPIO_ENABLE_POWER; j < LAST_GPIO; j++){
		GPIOS[j].io = ugpio_request_one(GPIOS[j].num, GPIOS[j].flags, "");

		if (GPIOS[j].io == NULL) {
			printf("ERROR: Could not alloc pin %d\n", GPIOS[j].num);
			return -1;
		}

		if (ugpio_full_open(GPIOS[j].io) < 0){
			printf("ERROR: Could not open %d\n", j);
			return -1;
		}

		if (ugpio_set_activelow(GPIOS[j].io, GPIOS[j].active_low) < 0){
			printf("ERROR: Could not set_activelow %d %d\n", j, errno);
			return -1;
		}
	}

	/*
	 * Initialize boards array
	 */
	for (int i = 0; i < NUM_BOARDS; i++) {
		boards[i].data.data = 0;
		boards[i].address = i;
	}
	return 0;
}


void hal_teardown(void){
	for (int j = GPIO_ENABLE_POWER; j < LAST_GPIO; j++){
		ugpio_close(GPIOS[GPIO_ENABLE_POWER].io);
		ugpio_free(GPIOS[GPIO_ENABLE_POWER].io);
		GPIOS[GPIO_ENABLE_POWER].io = NULL;
	}
}


int hal_setDefault(void)
{
	for (int i = 0; i < NUM_BOARDS; i++) {
		boards[i].data.data = 0;
		if (hal_setOut(i, boards[i].data.data) < 0){
			return -1;
		}
	}
	return 0;
}


int hal_enableLoad(int current)
{
	int retVal = 0;

	if (current <  0 || current > 127) {
		printf("ERR: Current out of range %d\n", current);
		return -1;
	}

	boards[LOAD_BOARD_ADR].data.load.enable = (current == 0) ? 0 : 1;
	boards[LOAD_BOARD_ADR].data.load.current = current >> 1;
	retVal = hal_setOut(boards[LOAD_BOARD_ADR].address, boards[LOAD_BOARD_ADR].data.data);
	return retVal;
}


int pin_setFunction(enum fc_pin pin, enum pinFunction function)
{
	int retVal = 0;

	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	if (pinSettings[pin].function == PIN_POWER ||
				(function != PIN_INPUT &&
				function != PIN_OUTPUT &&
				function != PIN_GND &&
				function != PIN_DISABLED)) {
		printf("ERR: Invalid pin function PIN_%s set to PIN_%s\n", pinFunctionStr[pinSettings[pin].function], pinFunctionStr[function]);
		return -1;
	}

	if (pinSettings[pin].bitOffset == 1) {
		pinSettings[pin].b->data.level.pin1 = (function == PIN_INPUT || function == PIN_OUTPUT) ? 1 : 0;
	} else {
		pinSettings[pin].b->data.level.pin2 = (function == PIN_INPUT || function == PIN_OUTPUT) ? 1 : 0;
	}

	if (function == PIN_GND){
		if (pinSettings[pin].bitOffset == 1) {
			pinSettings[pin].b->data.level.pin1 = 1;
			pinSettings[pin].b->data.level.do1 = 1;
			pinSettings[pin].b->data.level.pd1 = 0;
		} else {
			pinSettings[pin].b->data.level.pin2 = 1;
			pinSettings[pin].b->data.level.do2 = 1;
			pinSettings[pin].b->data.level.pd2 = 0;
		}
	}

	if (function == PIN_DISABLED){
		if (pinSettings[pin].bitOffset == 1) {
			pinSettings[pin].b->data.level.do1 = 0;
			pinSettings[pin].b->data.level.pd1 = 0;
		} else {
			pinSettings[pin].b->data.level.do2 = 0;
			pinSettings[pin].b->data.level.pd2 = 0;
		}
	}

	retVal = hal_setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
	pinSettings[pin].function = function;
	return retVal;
}


int pin_getFunction(enum fc_pin pin, enum pinFunction *function)
{
	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	*function = pinSettings[pin].function;
	return 0;
}


int pin_setMeasure(enum fc_pin pin, int enable)
{
	int retVal = 0;

	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	if (pinSettings[pin].function == PIN_POWER ||
	    pinSettings[pin].function == PIN_GND) {
		printf("ERR: Invalid pin function PIN_%s\n", pinFunctionStr[pinSettings[pin].function]);
		return -1;
	}

	if (pinSettings[pin].bitOffset == 1) {
		pinSettings[pin].b->data.level.meas1 = (enable) ? 1 : 0;
	} else {
		pinSettings[pin].b->data.level.meas2 = (enable) ? 1 : 0;
	}

	retVal = hal_setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
	return retVal;
}


int pin_enablePullDown(enum fc_pin pin, int enable)
{
	int retVal = 0;

	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	if (pinSettings[pin].function != PIN_INPUT) {
		printf("ERR: Invalid pin function PIN_%s\n", pinFunctionStr[pinSettings[pin].function]);
		return -1;
	}

	if (pinSettings[pin].bitOffset == 1) {
		pinSettings[pin].b->data.level.pd1 = (enable) ? 1 : 0;
	} else {
		pinSettings[pin].b->data.level.pd2 = (enable) ? 1 : 0;
	}

	retVal = hal_setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
	return retVal;
}


int pin_setDataOut(enum fc_pin pin, int data)
{
	int retVal = 0;

	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	if (pinSettings[pin].function != PIN_OUTPUT) {
		printf("ERR: Invalid pin function PIN_%s\n", pinFunctionStr[pinSettings[pin].function]);
		return -1;
	}

	// Setting do bit ties the pin to ground. "0" defined as 0 so invert
	if (pinSettings[pin].bitOffset == 1) {
		pinSettings[pin].b->data.level.do1 = (data) ? 0 : 1;
		pinSettings[pin].b->data.level.pd1 = (data) ? 1 : 0;
	} else {
		pinSettings[pin].b->data.level.do2 = (data) ? 0 : 1;
		pinSettings[pin].b->data.level.pd2 = (data) ? 1 : 0;
	}

	retVal = hal_setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
	return retVal;
}


int pin_setGnd(enum fc_pin pin, int data)
{
	int retVal = 0;

	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	if (pinSettings[pin].function == PIN_POWER) {
		printf("ERR: Invalid pin function PIN_%s\n", pinFunctionStr[pinSettings[pin].function]);
		return -1;
	}

	// Setting do bit ties the pin to ground.
	if (pinSettings[pin].bitOffset == 1) {
		pinSettings[pin].b->data.level.do1 = (data) ? 1 : 0;
	} else {
		pinSettings[pin].b->data.level.do2 = (data) ? 1 : 0;
	}

	retVal = hal_setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
	return retVal;
}


int pin_getValue(enum fc_pin pin, int *val)
{
	int data;
	int retVal = 0;

	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		return -1;
	}

	if (pinSettings[pin].function != PIN_INPUT &&
	    pinSettings[pin].function != PIN_OUTPUT) {
		printf("ERR: Invalid pin function %s\n", pinFunctionStr[pinSettings[pin].function]);
		return -1;
	}

	retVal = hal_getInputs(&data);
	*val = (pinSettings[pin].inputBitMask & data) ? 1 : 0;
	return retVal;
}


int pin_getName(enum fc_pin pin, char **str)
{
	if (pin > LAST_PIN) {
		printf("ERR: Invalid pin number %d\n", pin);
		str = NULL;
		return -1;
	}

	*str = (char *)pinSettings[pin].name;
	return 0;
}


void dump_pinSettings(void)
{
	for (int i=AA; i < LAST_PIN; i++) {
		printf ("Pin: %2.2d %s, boardAddress: %2d, bitOffset: %2d, inputMask %8.8x, %8.8x Function: %s\n",
			pinSettings[i].pinNumber,
			pinSettings[i].name,
			pinSettings[i].b ? pinSettings[i].b->address : -1,
			pinSettings[i].bitOffset,
			pinSettings[i].inputBitMask,
			pinSettings[i].b ? pinSettings[i].b->data.data : -1,
			pinFunctionStr[pinSettings[i].function]);
	}
}
