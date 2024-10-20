/*
 * BRS-tester by Anders Sandahl 2023-2024
 *
 * License GPL 2.0
 *
 * hal.c: Hardware abstraction
 *
 */
#include <linux/gpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gpiod.h>
#include <errno.h>

#include "hal.h"

/*
 * Filenames
 */
#define FILE_VOLTAGE0_RAW   "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define FILE_VOLTAGE0_SCALE "/sys/bus/iio/devices/iio:device0/in_voltage0_scale"
#define FILE_VOLTAGE1_RAW   "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define FILE_VOLTAGE1_SCALE "/sys/bus/iio/devices/iio:device0/in_voltage1_scale"
#define FILE_VOLTAGE3_RAW   "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define FILE_VOLTAGE3_SCALE "/sys/bus/iio/devices/iio:device0/in_voltage3_scale"

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
    const bool active_low;
    const char num;
    struct gpiod_line_request *io;
};

/*
 * Array of struct that holds references to all GPIO's. Maps defines and
 * initial configuration to all pin numbers.
 */
struct gpio GPIOS[] = {
    [GPIO_ENABLE_POWER].active_low = 0,
    [GPIO_ENABLE_POWER].num = 27,
    [GPIO_ENABLE_POWER].io = NULL,

    [GPIO_SELECT_OUT].active_low = 1,
    [GPIO_SELECT_OUT].num = 24,
    [GPIO_SELECT_OUT].io = NULL,

    [GPIO_SELECT_A].active_low = 0,
    [GPIO_SELECT_A].num = 25,
    [GPIO_SELECT_A].io = NULL,

    [GPIO_SELECT_B].active_low = 0,
    [GPIO_SELECT_B].num = 8,
    [GPIO_SELECT_B].io = NULL,

    [GPIO_SELECT_C].active_low = 0,
    [GPIO_SELECT_C].num = 7,
    [GPIO_SELECT_C].io = NULL,

    [GPIO_SELECT_D].active_low = 0,
    [GPIO_SELECT_D].num = 1,
    [GPIO_SELECT_D].io = NULL,

    [GPIO_LATCH_INPUTS].active_low = 0,
    [GPIO_LATCH_INPUTS].num = 12,
    [GPIO_LATCH_INPUTS].io = NULL,

    [GPIO_INP_A_OE].active_low = 1,
    [GPIO_INP_A_OE].num = 23,
    [GPIO_INP_A_OE].io = NULL,

    [GPIO_INP_B_OE].active_low = 1,
    [GPIO_INP_B_OE].num = 0,
    [GPIO_INP_B_OE].io = NULL,

    [GPIO_INP_C_OE].active_low = 1,
    [GPIO_INP_C_OE].num = 4,
    [GPIO_INP_C_OE].io = NULL,

    [GPIO_INP_D_OE].active_low = 1,
    [GPIO_INP_D_OE].num = 10,
    [GPIO_INP_D_OE].io = NULL,

    [GPIO_D0].active_low = 0,
    [GPIO_D0].num = 5,
    [GPIO_D0].io = NULL,

    [GPIO_D1].active_low = 0,
    [GPIO_D1].num = 6,
    [GPIO_D1].io = NULL,

    [GPIO_D2].active_low = 0,
    [GPIO_D2].num = 13,
    [GPIO_D2].io = NULL,

    [GPIO_D3].active_low = 0,
    [GPIO_D3].num = 19,
    [GPIO_D3].io = NULL,

    [GPIO_D4].active_low = 0,
    [GPIO_D4].num = 26,
    [GPIO_D4].io = NULL,

    [GPIO_D5].active_low = 0,
    [GPIO_D5].num = 21,
    [GPIO_D5].io = NULL,

    [GPIO_D6].active_low = 0,
    [GPIO_D6].num = 20,
    [GPIO_D6].io = NULL,

    [GPIO_D7].active_low = 0,
    [GPIO_D7].num = 16,
    [GPIO_D7].io = NULL,

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
 * gpio chip global reference
 */
static struct gpiod_chip *chip;

/*
 * Total number of boards in the backplane. 15 level shifters and one
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
    [PIN_POWER]     = "POWER",
    [PIN_GND]       = "GND",
    [PIN_INPUT]     = "INPUT",
    [PIN_OUTPUT]    = "OUTPUT",
    [PIN_DISABLED]  = "DISABLED"
};


/*
 * Mapping between PINS on a flipchip to backplane level shifters. Not
 * all pins are possible to test, like power and ground. But have them
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


static int
get_gpio_device(char ** dev) {
    struct gpiochip_info info;
    int fd, ret = 0;

/*
 * Try to figure out if the target is a Rpi 3,4 or 5
 *
 * Rpi 3: Chip label: pinctrl-bcm2835, Chip name: gpiochip0, Number of lines: 54
 * Rpi 4: Chip label: pinctrl-bcm2711, Chip name: gpiochip0, Number of lines 58
 * Rpi 5: Chip label: pinctrl-rp1, Chip name: gpiochip4, Number of lines: 54
 *
 */
    *dev = "/dev/gpiochip0";
    fd = open(*dev, O_RDONLY);

    if (fd < 0) {
        printf("ERROR: Could not open %s: %s\n", *dev, strerror(errno));
        return -1;
    }

    ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
    close(fd);

    if (ret == -1) {
        printf("Unable to get chip info from ioctl: %s\n", strerror(errno));
        return ret;
    }

    if (info.lines == 54 || info.lines == 58) {
        return 0;
    }

    *dev = "/dev/gpiochip4";
    fd = open(*dev, O_RDONLY);

    if (fd < 0) {
        printf("ERROR: Could not open %s: %s\n", *dev, strerror(errno));
        return fd;
    }

    ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
    close(fd);

    if (ret == -1) {
        printf("Unable to get chip info from ioctl: %s\n", strerror(errno));
        return ret;
    }

    if (info.lines == 54) {
        return 0;
    }

    printf("Unable to figure out Rpi model\n");
    return -1;
}


static struct gpiod_line_request *
gpiod_get_output_line(struct gpiod_chip *chip,
              unsigned int offset,
              bool active_low)
{
    struct gpiod_line_request *request = NULL;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    int ret;

    if (!chip)
        return NULL;

    settings = gpiod_line_settings_new();
    if (!settings)
        return NULL;

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_settings_set_active_low(settings, active_low);

    line_cfg = gpiod_line_config_new();
    if (!line_cfg)
        goto free_settings;

    ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
                          settings);
    if (ret)
        goto free_line_config;

    request = gpiod_chip_request_lines(chip, NULL, line_cfg);

free_line_config:
    gpiod_line_config_free(line_cfg);

free_settings:
    gpiod_line_settings_free(settings);

    return request;
}


static int
gpiod_reconfigure(struct gpiod_line_request *request,
          unsigned int offset,
          enum gpiod_line_direction dir,
          bool active_low)
{
    struct gpiod_request_config *req_cfg = NULL;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    int ret = -1;

    settings = gpiod_line_settings_new();
    if (!settings)
        return -1;

    gpiod_line_settings_set_direction(settings, dir);
    if (dir == GPIOD_LINE_DIRECTION_OUTPUT) {
        gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);
    }
    gpiod_line_settings_set_active_low(settings, active_low);

    line_cfg = gpiod_line_config_new();
    if (!line_cfg)
        goto free_settings;

    ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
                          settings);
    if (ret)
        goto free_line_config;

    ret = gpiod_line_request_reconfigure_lines(request, line_cfg);

    gpiod_request_config_free(req_cfg);

free_line_config:
    gpiod_line_config_free(line_cfg);

free_settings:
    gpiod_line_settings_free(settings);
    return ret;
}


/*
 * Clock out data to a board
 */
static int setOut(int channel, int data)
{
    if (channel < 0 || channel > 15) {
        printf("ERR: invalid board index\n");
        return -1;
    }

    /*
     * Set as output and active low for highest bit
     */
    for (int i = GPIO_D0; i <= GPIO_D7; i++) {
        if (channel == LOAD_BOARD_ADR) {
            /*
             * Load board has other pins as active low/high
             */
            switch (i) {
                case GPIO_D6:
                case GPIO_D7:
                    /*
                     * Set active high
                     */
                     gpiod_reconfigure(GPIOS[i].io, GPIOS[i].num, GPIOD_LINE_DIRECTION_OUTPUT, 0);
                     break;
                 default:
                    /*
                     * Set active low
                     */
                     gpiod_reconfigure(GPIOS[i].io, GPIOS[i].num, GPIOD_LINE_DIRECTION_OUTPUT, 1);
                     break;
            }
        } else {
            switch (i) {
                case GPIO_D4:
                case GPIO_D5:
                case GPIO_D6:
                case GPIO_D7:
                    /*
                     * Set active low
                     */
                     gpiod_reconfigure(GPIOS[i].io, GPIOS[i].num, GPIOD_LINE_DIRECTION_OUTPUT, 1);
                     break;
                 default:
                    /*
                     * Set active high
                     */
                     gpiod_reconfigure(GPIOS[i].io, GPIOS[i].num, GPIOD_LINE_DIRECTION_OUTPUT, 0);
                     break;
            }
        }
    }

    gpiod_line_request_set_value(GPIOS[GPIO_SELECT_A].io,
                     GPIOS[GPIO_SELECT_A].num,
                     (channel >> 0) & 1 ? GPIOD_LINE_VALUE_ACTIVE :
                      GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(GPIOS[GPIO_SELECT_B].io,
                     GPIOS[GPIO_SELECT_B].num,
                     (channel >> 1) & 1 ? GPIOD_LINE_VALUE_ACTIVE :
                      GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(GPIOS[GPIO_SELECT_C].io,
                     GPIOS[GPIO_SELECT_C].num,
                     (channel >> 2) & 1 ? GPIOD_LINE_VALUE_ACTIVE :
                      GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(GPIOS[GPIO_SELECT_D].io,
                     GPIOS[GPIO_SELECT_D].num,
                     (channel >> 3) & 1 ? GPIOD_LINE_VALUE_ACTIVE :
                      GPIOD_LINE_VALUE_INACTIVE);

    for (int i = 0; i <= 7; i++) {
        gpiod_line_request_set_value(GPIOS[i + GPIO_D0].io,
                         GPIOS[i + GPIO_D0].num,
                         ((data >> i) & 1) ? GPIOD_LINE_VALUE_ACTIVE :
                          GPIOD_LINE_VALUE_INACTIVE);
    }

    gpiod_line_request_set_value(GPIOS[GPIO_SELECT_OUT].io,
                     GPIOS[GPIO_SELECT_OUT].num,
                     GPIOD_LINE_VALUE_ACTIVE);
    usleep(1);
    gpiod_line_request_set_value(GPIOS[GPIO_SELECT_OUT].io,
                     GPIOS[GPIO_SELECT_OUT].num,
                     GPIOD_LINE_VALUE_INACTIVE);
    return 0;
}


/*
 * Enable/disable power to the flipchip under test
 */
int hal_powerEnable(int on) {
    int ret = gpiod_line_request_set_value(GPIOS[GPIO_ENABLE_POWER].io,
                           GPIOS[GPIO_ENABLE_POWER].num,
                           on ? GPIOD_LINE_VALUE_ACTIVE :
                        GPIOD_LINE_VALUE_INACTIVE);

    if (ret < 0) {
        printf("ERROR: Could not %s power\n",  on ? "enable" : "disable");
    }

    return ret;
}


/*
 * Measure the current between the level shifter and the flipchip pin
 * under test.
 */
int hal_measureCurrent(float *iMeas){
    float uRef_scale = 0;
    float uRef_voltage = 0;
    float uRaw_scale = 0;
    float uRaw_voltage = 0;
    int c;

    FILE *fp = fopen(FILE_VOLTAGE3_RAW, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE3_RAW);
        return -1;
    }

    c = fscanf(fp, "%f", &uRef_voltage);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE3_SCALE, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE3_SCALE);
        return -1;
    }

    c = fscanf(fp, "%f", &uRef_scale);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE0_RAW, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE0_RAW);
        return -1;
    }

    c = fscanf(fp, "%f", &uRaw_voltage);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE0_SCALE, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE0_SCALE);
        return -1;
    }

    c = fscanf(fp, "%f", &uRaw_scale);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    /*
     * R = 10 Ohms, the INA141 has a gain of 10. 1 V equals 10mA, 1mV equals 10uA
     * Result is given in mA.
     */
    *iMeas = (((float)(150.0/12.0) * ((uRaw_scale * uRaw_voltage) - (uRef_scale * uRef_voltage))) + (uRaw_scale * uRaw_voltage))/100;
    return 0;
}


/*
 * Measure the voltage on a flipchip pin under test.
 */
int hal_measureVoltage(float *uMeas){
    float uRef_scale = 0;
    float uRef_voltage = 0;
    float uRaw_scale = 0;
    float uRaw_voltage = 0;
    int c;

    FILE *fp = fopen(FILE_VOLTAGE3_RAW, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE3_RAW);
        return -1;
    }

    c = fscanf(fp, "%f", &uRef_voltage);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE3_SCALE, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE3_SCALE);
        return -1;
    }

    c = fscanf(fp, "%f", &uRef_scale);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE1_RAW, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE1_RAW);
        return -1;
    }

    c = fscanf(fp, "%f", &uRaw_voltage);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE1_SCALE, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE1_SCALE);
        return -1;
    }

    c = fscanf(fp, "%f", &uRaw_scale);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    /*
     * Many magical numbers here. The +15/-15 V is divided with 150k and 12K ohm resistor
     * to a reference voltage of about 1.7 V. This means that the voltage range is mapped
     * to a voltage ~0.5 V to 2.7 V to fit in the ADC range. The calulation gets the real
     * voltage from the ADC range. In millivolts!
     */
    *uMeas = ((float)(150.0/12.0) * ((uRaw_scale * uRaw_voltage) - (uRef_scale * uRef_voltage))) + (uRaw_scale * uRaw_voltage);
    return 0;
}


/*
 * Returns the 1.7V reference voltage.
 */
int hal_measureVoltageRef(float *uMeas){
    float uRef_scale = 0;
    float uRef_voltage = 0;
    int c;

    FILE *fp = fopen(FILE_VOLTAGE3_RAW, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE3_RAW);
        return -1;
    }

    c = fscanf(fp, "%f", &uRef_voltage);
    fclose(fp);

    if (c != 1) {
        return -1;
    }

    fp = fopen(FILE_VOLTAGE3_SCALE, "r");

    if (fp == NULL) {
        printf("ERROR: Could not open: %s\n", FILE_VOLTAGE3_SCALE);
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


/*
 * First latch all inputs from the level-shifter boards. Then change the
 * bus to inputs and read data from one latch at a time.
 */
int hal_getInputs (int *data) {
    int value=0;
    int inputs=0;

    gpiod_line_request_set_value(GPIOS[GPIO_LATCH_INPUTS].io,
                     GPIOS[GPIO_LATCH_INPUTS].num,
                     GPIOD_LINE_VALUE_ACTIVE);
    usleep(1);
    gpiod_line_request_set_value(GPIOS[GPIO_LATCH_INPUTS].io,
                     GPIOS[GPIO_LATCH_INPUTS].num,
                     GPIOD_LINE_VALUE_INACTIVE);

    /*
     * Set as input and all as active high
     */
    for (int i = GPIO_D0; i <= GPIO_D7; i++) {
        gpiod_reconfigure(GPIOS[i].io, GPIOS[i].num, GPIOD_LINE_DIRECTION_INPUT, 0);
    }

    for (int k=0; k <= (GPIO_INP_D_OE - GPIO_INP_A_OE); k++) {
        /*
         * Enable latch output
         */
        gpiod_line_request_set_value(GPIOS[GPIO_INP_A_OE+k].io,
                         GPIOS[GPIO_INP_A_OE+k].num,
                         GPIOD_LINE_VALUE_ACTIVE);
        usleep(1);

        for (int i = GPIO_D0; i <= GPIO_D7; i++) {
            value = gpiod_line_request_get_value(GPIOS[i].io,
                                 GPIOS[i].num);
            inputs |= ((value & 1) << (i - GPIO_D0 + (8 * k)));
        }

        /*
         * Disable latch output
         */
        gpiod_line_request_set_value(GPIOS[GPIO_INP_A_OE+k].io,
                         GPIOS[GPIO_INP_A_OE+k].num,
                         GPIOD_LINE_VALUE_INACTIVE);
    }

    *data = inputs;

    /*
     * Set as output
     */
    for (int i = GPIO_D0; i <= GPIO_D7; i++) {
        gpiod_reconfigure(GPIOS[i].io, GPIOS[i].num, GPIOD_LINE_DIRECTION_OUTPUT, GPIOS[i].active_low);
    }
    return 0;
}


/*
 * Initialize all GPIO's
 */
int hal_setup(void){
    char * path = NULL;
    /*
     *  Open GPIO-chip
     */
    if (get_gpio_device(&path) < 0) {
        return -1;
    }

    chip = gpiod_chip_open(path);
    if (!chip)
        return -1;

    for (int j = GPIO_ENABLE_POWER; j < LAST_GPIO; j++){
        GPIOS[j].io = gpiod_get_output_line(chip,  GPIOS[j].num, GPIOS[j].active_low);

        if (GPIOS[j].io == NULL) {
            printf("ERROR: Could not open gpio %d\n", GPIOS[j].num);
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

/*
 * Release all GPIO's and close connection to the gpio chip.
 */
void hal_teardown(void){
    for (int j = GPIO_ENABLE_POWER; j < LAST_GPIO; j++){
        gpiod_line_request_release(GPIOS[j].io);
        GPIOS[j].io = NULL;
    }
    gpiod_chip_close(chip);
}


/*
 * Set all functions on all level-shifter board to a disabled state.
 */
int hal_setDefault(void)
{
    for (int i = 0; i < NUM_BOARDS; i++) {
        boards[i].data.data = 0;
        if (setOut(i, boards[i].data.data) < 0){
            return -1;
        }
    }
    return 0;
}


/*
 * Enable a resistor load on the measurement bus. The current can be set
 * from 0 to 126mA. Where 0 is disable. This can be used to verify drive
 * strength of output transistors on flipchips.
 */
int hal_enableLoad(int current, bool relayOn)
{
    int retVal = 0;

    if (current <  0 || current > 127) {
        printf("ERR: Current out of range %d\n", current);
        return -1;
    }

    boards[LOAD_BOARD_ADR].data.load.enable = relayOn;
    boards[LOAD_BOARD_ADR].data.load.current = (current >> 1) & 0x3f        ;
    retVal = setOut(boards[LOAD_BOARD_ADR].address, boards[LOAD_BOARD_ADR].data.data);
    return retVal;
}


/*
 * Set function on a pin
 *
 * PIN_OUTPUT   - Input on flipchip
 * PIN_INPUT    - Output on flipchip
 * PIN_POWER    - ABC Pins are always power and can not be changed nor measured
 * PIN_DISABLED - Pin is not used and is disconnected
 * PIN_GND      - Pin must be grounded on flipchip
 */
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

    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    pinSettings[pin].function = function;
    return retVal;
}


/*
 * Return the actual function of a pin.
 */
int pin_getFunction(enum fc_pin pin, enum pinFunction *function)
{
    if (pin > LAST_PIN) {
        printf("ERR: Invalid pin number %d\n", pin);
        return -1;
    }

    *function = pinSettings[pin].function;
    return 0;
}


/*
 * Connect a pin to the measurement circuitry. There is no check if more
 * than one pin is enabled at the same time so be careful.
 */
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

    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    return retVal;
}


/*
 * When a pin is input, pull down can be enabled. Used when output is
 * open-collector
 */
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

    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    return retVal;
}


/*
 * Set data out on a pin
 */
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

    /*
     * Setting do bit ties the pin to ground. "0" defined as 0 so invert
     */
    if (pinSettings[pin].bitOffset == 1) {
        pinSettings[pin].b->data.level.do1 = (data) ? 0 : 1;
        pinSettings[pin].b->data.level.pd1 = (data) ? 1 : 0;
    } else {
        pinSettings[pin].b->data.level.do2 = (data) ? 0 : 1;
        pinSettings[pin].b->data.level.pd2 = (data) ? 1 : 0;
    }

    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    return retVal;
}


/*
 * Toggle a pin as fast as possible. Gives about 130kHz on IO pin (with RPI4).
 */
int pin_toggleData(enum fc_pin pin, int toggles)
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

    /*
     * Set output, this ensures that the bus will have the right bits set.
     * Also, the right address is set.
     */
    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    if (retVal != 0) {
        return -1;
    }

    if (pinSettings[pin].bitOffset == 1) {
        gpiod_line_request_set_value(GPIOS[GPIO_D4].io,
                         GPIOS[GPIO_D4].num,
                         GPIOD_LINE_VALUE_ACTIVE);
    } else {
        gpiod_line_request_set_value(GPIOS[GPIO_D6].io,
                         GPIOS[GPIO_D6].num,
                         GPIOD_LINE_VALUE_ACTIVE);
    }

    for (int i=0; i<toggles; i++){
        if (pinSettings[pin].bitOffset == 1) {
            gpiod_line_request_set_value(GPIOS[GPIO_D5].io,
                             GPIOS[GPIO_D5].num,
                             (i & 1) ? GPIOD_LINE_VALUE_ACTIVE :
                              GPIOD_LINE_VALUE_INACTIVE);
        } else {
            gpiod_line_request_set_value(GPIOS[GPIO_D7].io,
                             GPIOS[GPIO_D7].num,
                             (i & 1) ? GPIOD_LINE_VALUE_ACTIVE :
                              GPIOD_LINE_VALUE_INACTIVE);
        }

        gpiod_line_request_set_value(GPIOS[GPIO_SELECT_OUT].io,
                         GPIOS[GPIO_SELECT_OUT].num,
                         GPIOD_LINE_VALUE_ACTIVE);
        usleep(1);
        gpiod_line_request_set_value(GPIOS[GPIO_SELECT_OUT].io,
                         GPIOS[GPIO_SELECT_OUT].num,
                         GPIOD_LINE_VALUE_INACTIVE);
    }

    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    return retVal;
}


/*
 * Tie a pin to ground
 */
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

    /*
     * Setting do bit ties the pin to ground.
     */
    if (pinSettings[pin].bitOffset == 1) {
        pinSettings[pin].b->data.level.do1 = (data) ? 1 : 0;
    } else {
        pinSettings[pin].b->data.level.do2 = (data) ? 1 : 0;
    }

    retVal = setOut(pinSettings[pin].b->address, pinSettings[pin].b->data.data);
    return retVal;
}


/*
 * Get the digital value of a pin
 */
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


/*
 * Get name from pin index.
 */
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


/*
 * Get pin index from name.
 */
enum fc_pin pin_getIndex(char *str)
{
    enum fc_pin pin=-1;
    for (enum fc_pin i=0; i<LAST_PIN; i++){
        if(0 == strcmp((char *)pinSettings[i].name, str)){
            pin=i;
            break;
        }
    }
    return pin;
}
