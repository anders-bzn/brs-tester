/*
 * Definition of PIN vs number
 */
enum fc_pin {
	AA = 0,
	AB = 1,
	AC = 2,
	AD = 3,
	AE = 4,
	AF = 5,
	AH = 6,
	AJ = 7,
	AK = 8,
	AL = 9,
	AM = 10,
	AN = 11,
	AP = 12,
	AR = 13,  
	AS = 14,
	AT = 15,
	AU = 16,
	AV = 17,
	BA = 18, 
	BB = 19,
	BC = 20,
	BD = 21,
	BE = 22,
	BF = 23,
	BH = 24,
	BJ = 25,
	BK = 26,
	BL = 27,
	BM = 28,
	BN = 29,
	BP = 30,
	BR = 31,
	BS = 32,
	BT = 33,
	BU = 34,
	BV = 35,
	LAST_PIN = 36,
};

/*
 * Valid functions of a pin
 */
enum pinFunction {
	PIN_POWER	= 0,
	PIN_GND		= 1,
	PIN_INPUT	= 2,
	PIN_OUTPUT	= 3,
	PIN_DISABLED	= 4
};

int hal_powerEnable(int on);
int hal_measureCurrent(float *iMeas);
int hal_measureVoltage(float *uMeas);
int hal_setOut(int channel, int data);
int hal_getInputs (int *data);
int hal_setup(void);
int hal_enableLoad(int current);
int pin_setFunction(enum fc_pin pin, enum pinFunction function);
int pin_setMeasure(enum fc_pin pin, int enable);
int pin_enablePullDown(enum fc_pin pin, int enable);
int pin_setDataOut(enum fc_pin pin, int data);
int pin_setGnd(enum fc_pin pin, int data);
int pin_getValue(enum fc_pin pin, int *val);
int pin_getName(enum fc_pin pin, char **str);
