/*
 * BRS-tester by Anders Sandahl 2023-2024
 *
 * License GPL 2.0
 *
 * tests.c: Functions for testing
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#include "hal.h"
#include "vector.h"


int tests_selfTest(void)
{
    float voltage_ref;
    int voltage_ref_ok = 0;
    char *str = NULL;

    hal_powerEnable(1);
    hal_setDefault();

    /*
     * First measure reference voltage, it should be 1700 mV nominal
     */
    if (hal_measureVoltageRef(&voltage_ref) < 0) {
        return -1;
    }

    if (fabs(voltage_ref - 1700.0) < 15) {
        voltage_ref_ok = 1;
    }

    printf("Reference voltage: %7.1f mV (1700.0 mV)                              [ %s ]\n", voltage_ref, voltage_ref_ok ? " OK " : "FAIL");

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
             * SetA "1" and measure
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

            if ( fabs(voltage_l) < 10.0 && fabs(voltage_h + 3700.0 ) < 50.0) {
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
        float current_error;
        int current_ok = 0;

        /*
         * In an ideal world, grounding the load resistors would have given
         * current 2, 4, 8, 16, 32 and 64 mA. However, the measurement resistor for current
         * and RDS(on) in the FET end up in series when testing the loads.
         */
        const float currents[] = {-1.9, -3.9, -7.7, -15.0, -28.2, -50.2};

        hal_enableLoad(curr);
        usleep(10000);
        hal_measureCurrent (&current);
        current_error = fabs(currents[i] - current);

        if ((current_error < fabs(currents[i] * 0.05)) ||
            (current_error < 0.2)){
            current_ok = 1;
        }

        printf("Load: %.2d Meas: (%7.1f mA) (%7.1f mA)                              [ %s ]\n",
               curr * -1, current, currents[i], (current_ok) ? " OK " : "FAIL");
    }

    pin_setFunction(AE, PIN_DISABLED);
    pin_setMeasure(AE, 0);

    /*
     * Test drive strength
     */
    hal_enableLoad(32);

    for (int pin = AD; pin < LAST_PIN; pin++) {
        float current, voltage;
        int result = 0;

        if (pin == BA || pin == BB || pin == BC)
            continue;

        pin_setFunction(pin, PIN_OUTPUT);
        pin_setDataOut(pin, 0);
        pin_setMeasure(pin, 1);
        usleep(100000);
        hal_measureCurrent (&current);
        hal_measureVoltage (&voltage);

        result = (fabs(current) > 28.0 && fabs(voltage) < 400.0);

        printf("Drive strength: %.2d mA, meas: (%7.1f mA) (%7.1f mV)                 [ %s ]\n",
            32, current, voltage, (result) ? " OK " : "FAIL");

        pin_setDataOut(pin, 1);
        pin_setMeasure(pin, 0);
        pin_setFunction(pin, PIN_DISABLED);
    }
    hal_enableLoad(0);
    hal_setDefault();
    hal_powerEnable(0);
    return 0;
}


int tests_setupBoard(struct config const *b_cfg)
{
    char const *str = b_cfg->pin_def;

    /* Setup string can look like this:
     * pppiodiodiodiodiod------------------
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
     * 'd' - pull down net on tested board
     * 'g' - pin should be grounded on tested board
     * '-' - pin should not be used
     */

    for (int pin = AA; pin < LAST_PIN; pin ++) {
        switch (str[pin]) {
        case 'p':
            printf("p");
            break;
        case '-':
            printf("-");
            break;
        case 'i':
            /*
             * NOTE: PIN_OUTPUT is from the testers view.
             * The format string describes the board under
             * test.
             */
            printf("i");
            pin_setFunction(pin, PIN_OUTPUT);
            break;
        case 'O':
            /*
             * Open collector need pull down.
             */
            printf("O");
            pin_setFunction(pin, PIN_INPUT);
            pin_enablePullDown(pin, 1);
            break;
        case 'o':
            /*
             * NOTE: PIN_INPUT is from the testers view.
             * The format string describes the board under
             * test.
             */
            printf("o");
            pin_setFunction(pin, PIN_INPUT);
            break;
        case 'd':
            /*
             * NOTE: PIN_INPUT is from the testers view.
             * The format string describes the board under
             * test. Setup pull down as input???
             */
            printf("d");
            pin_setFunction(pin, PIN_INPUT);
            break;
        case 'g':
            /*
             * NOTE: Pin should be tied to ground dring test
             */
            printf("g");
            pin_setFunction(pin, PIN_GND);
            break;
        default:
            printf("ERROR: Format error\n");
            return -1;
        }
    }
    printf("\n\n");
    return 0;
}


int tests_checkVoltages(struct config const *b_cfg)
{
    char const *str = b_cfg->pin_def;
    float voltage;
    int retVal = 0;

    /* Setup string can look like this:
     * pppiodiodiodiodiod------------------
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
     * 'd' - pull down net on tested board
     * 'g' - pin should be grounded on tested board
     * '-' - pin should not be used
     */

    for (int pin = AA; pin < LAST_PIN; pin ++) {
        int voltage_ok = 0;
        char *pinName;

        pin_getName(pin, &pinName);

        switch (str[pin]) {
        case '-':
            break;
        case 'p':
        case 'g':
        case 'i':
            printf("Pin: %s %c\n", pinName, str[pin]);
            break;
        case 'o':
        case 'O':
        case 'd':
            /*
             * Check that voltage are in valid range
             */
            pin_setMeasure(pin, 1);
            usleep(50000);
            hal_measureVoltage(&voltage);
            pin_setMeasure(pin, 0);

            if ( voltage < 10.0 && voltage > -4000.0){
                voltage_ok = 1;
            }

            /*
             * Return -1 if any voltage is outside the accepted range. Testing should not continue
             * to protect the tester.
             */
            if (voltage_ok != 1){
                retVal = -1;
            }

            printf("Pin: %s %c voltage %7.1f                 [ %s ]\n", pinName, str[pin], voltage, voltage_ok ? " OK " : "FAIL");
            break;
        default:
            printf("ERROR: Format error\n");
            return -1;
        }
    }
    printf("\n");
    return retVal;
}

int tests_checkPullDown(struct config const *b_cfg)
{
    const char *str = b_cfg->pin_def;
    float voltage_l;
    float voltage_h;
    float current;

    /* Setup string can look like this:
     * pppiodiodiodiodiod------------------
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
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
        case 'O':
            break;
        case 'd':
            /*
             * Check that pull down works
             */
            pin_setMeasure(pin, 1);
            usleep(100000);
            hal_measureVoltage(&voltage_l);
            pin_setGnd(pin, 1);
            usleep(100000);
            hal_measureVoltage(&voltage_h);
            hal_measureCurrent(&current);
            pin_setGnd(pin, 0);
            pin_setMeasure(pin, 0);

            /*
             * Check voltage high and low against configuration
             */
            if ( fabs(voltage_l - b_cfg->load_low) < b_cfg->load_low_margin &&
                 fabs(voltage_h - b_cfg->load_high) < b_cfg->load_high_margin){
                voltage_ok = 1;
            }

            /*
             * Check current against configuration
             */
            if ( fabs(current - b_cfg->load_current) < b_cfg->load_current_margin) {
                current_ok = 1;
            }

            printf("Pin: %s %c voltage %7.1f %7.1f %7.1f [ %s ]\n", pinName, str[pin], voltage_l, voltage_h, current, (voltage_ok && current_ok) ? " OK " : "FAIL");
            break;
        default:
            printf("ERROR: Format error\n");
            return -1;
        }
    }
    printf("\n");
    return 0;
}


int tests_checkLogic(struct config const *b_cfg, char *vector)
{
    char const *setup = b_cfg->pin_def;
    bool test_failed = false;

    /* Setup string can look like this:
     * pppiodiodiodiodiod------------------
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
     * 'd' - pull down net on tested board
     * 'g' - pin should be grounded on tested board
     * '-' - pin should not be used
     */

    for (int pin = AA; pin < LAST_PIN; pin++) {
        switch (setup[pin]) {
        case 'p':
        case 'g':
        case '-':
        case 'd':
        case 'o':
        case 'O':
            printf("%c", vector[pin]);
            break;
        case 'i':
            /*
             * Set output
             */
            if (vector[pin] == 'T') {
                pin_setDataOut(pin, 0);
            } else {
                pin_setDataOut(pin, vector[pin] - '0');
            }
            printf("%c", vector[pin]);
            break;
        default:
            printf("ERROR: Format error\n");
            return -1;
        }
    }
    printf("\n");

    /*
     * Check for toggling pin
     */
    for (int pin = AA; pin < LAST_PIN; pin ++) {
        char *pinName;

        pin_getName(pin, &pinName);
        if (setup[pin] == 'i' && vector[pin] == 'T') {
            pin_toggleData(pin, b_cfg->toggles);
            printf("Toogle pin %s %d times\n", pinName, b_cfg->toggles);
        }
    }

    for (int pin = AA; pin < LAST_PIN; pin ++) {
        int data_in;
        int data_ok=0;

        switch (setup[pin]) {
        case 'p':
        case 'g':
        case '-':
        case 'i':
            printf("-");
            break;
        case 'd':
        case 'o':
        case 'O':
            /*
             * Get data in. '-' in format string means "don't check".
             */

            if (vector[pin] != '-') {
                pin_getValue(pin, &data_in);

                if (vector[pin] == ('0' + data_in)){
                    data_ok = 1;
                } else {
                    test_failed = true;
                }

                printf("%c", data_ok ? '0' + data_in : 'F');
                break;
            } else {
                printf("x");
            }
            break;
        default:
            printf("ERROR: Format error\n");
            return -1;
        }
    }
    printf("      [ %s ] \n", test_failed ? "FAIL" : " OK ");
    return 0;
}


int tests_checkDriveStrength(struct config const *b_cfg, char *vector)
{
    printf("%s\n", vector);

    float voltage1, voltage2;
    char *pinName;
    char const *setup = b_cfg->pin_def;
    bool test_run = false;
    bool result;

    /* Setup string can look like this:
     * pppiodiodiodiodiod------------------
     *
     * 'p' - power pin, do nothing
     * 'i' - input pin on tested board
     * 'o' - output pin on tested board
     * 'O' - output pin on tested board, open collector
     * 'd' - pull down net on tested board
     * 'g' - pin should be grounded on tested board
     * '-' - pin should not be used
     */

    for (int pin = AA; pin < LAST_PIN; pin++) {
        switch (setup[pin]) {
        case 'p':
        case 'g':
        case '-':
        case 'd':
            break;
        case 'i':
            /*
             * Set output
             */
            if (vector[pin] == 'P') {
                // Handle later, no Pulse function yet.
            } else {
                pin_setDataOut(pin, vector[pin] - '0');
            }
            break;
        case 'o':
        case 'O':
            if (vector[pin] == 'L') {
                pin_setMeasure(pin, 1);
                usleep(100000);
                hal_measureVoltage(&voltage1);

                /*
                 * Here, enable dummy load
                 */
                hal_enableLoad(b_cfg->output_drive_strength);
                usleep(200000);
                hal_measureVoltage(&voltage2);
                hal_enableLoad(0);
                usleep(100);
                pin_setMeasure(pin, 0);
                pin_getName(pin, &pinName);

                if (fabs(voltage1 - b_cfg->output_voltage_high) > b_cfg->output_voltage_margin){
                    result = false;
                } else {
                    result = true;
                }

                /*
                 * Do some limit testing. Compare voltage
                 */
                printf("Pin %s %3d mA  voltage %7.1f mV         [ %s ]\n", pinName,
                                                                           0,
                                                                           voltage1,
                                                                           result ? " ok " : "fail");

                if (fabs(voltage2 - b_cfg->output_voltage_high) > b_cfg->output_voltage_margin){
                    result = false;
                } else {
                    result = true;
                }

                printf("Pin %s %3d mA  voltage %7.1f mV         [ %s ]\n", pinName,
                                                                           b_cfg->output_drive_strength,
                                                                           voltage2,
                                                                           result ? " ok " : "fail");

                test_run = true;
            }

            break;


        default:
            printf("ERROR: Format error\n");
            return -1;
        }
    }

    if (test_run) printf("\n");

    return 0;
}


/*
 * This function will loop over all inputs and verify that they don't
 * draw to much current on the input pins.
 */
int tests_checkInputs(struct config const *b_cfg)
{
    char const *setup = b_cfg->pin_def;

    /* Setup string can look like this:
     * pppiodiodiodiodiod------------------
     *
     * 'i' - input pin on tested board
     *
     * Just care about the inputs.
     */

    for (int pin = AA; pin < LAST_PIN; pin++) {
        if (setup[pin] == 'i') {
            /*
             * Set inactive level on all inputs
             */
            pin_setDataOut(pin, 1);
        }
    }

    for (int pin = AA; pin < LAST_PIN; pin++) {
        if (setup[pin] == 'i') {
            float current, voltage;
            int result;

            char *str;
            pin_getName(pin, &str);
            pin_setMeasure(pin, 1);
            pin_setDataOut(pin, 1);
            usleep(200000);
            hal_measureCurrent(&current);
            hal_measureVoltage(&voltage);

            if ((fabs(voltage - b_cfg->input_voltage_low) > 100) ||
                (fabs(current - b_cfg->input_current_low) > b_cfg->input_current_margin)){
                result = 0;
            } else {
                result = 1;
            }

            printf("Pin: %s 0 voltage %7.1f current %7.1f [ %s ]\n", str, voltage, current, result ? " OK " : "FAIL");
            pin_setDataOut(pin, 0);
            usleep(100000);
            hal_measureCurrent(&current);
            hal_measureVoltage(&voltage);

            if ((fabs(voltage - b_cfg->input_voltage_high) > 100) ||
                (fabs(current - b_cfg->input_current_high) > b_cfg->input_current_margin)){
                result = 0;
            } else {
                result = 1;
            }
            printf("Pin: %s 1 voltage %7.1f current %7.1f [ %s ]\n", str, voltage, current, result ? " OK " : "FAIL");
            pin_setMeasure(pin, 0);
            pin_setDataOut(pin, 0);
        }
    }
    printf("\n");
    return 0;
}
