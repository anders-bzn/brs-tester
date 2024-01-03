# About the brs-tester

This project implements a tester for DEC flipchips in the B/R/S series.
Those modules are used in PDP-8, PDP-8/S, PDP-9 and in PDP-10 with KA processor.
This project was created as a part of the restoration os the PDP-9 with
serial number 203.

The modules uses 0 and -3V volts for the logical 0 and 1. The tester can detect both logical and electrical failures.

The software runs on a Raspberry Pi with [Raspberry Pi OS](https://www.raspberrypi.com/software/). It's been tested with Raspberry Pi OS 11/12 on Raspberry Pi 3/4/5.

![BRS-tester](https://www.abc80.net/pics/brs-tester.jpg)

## Electronics
The major parts is:

- Levelshifter boards
- Backplane board with control logic and measurement circuitry
- Power supply
- Load module

### Levelshifter boards
There are fifteen boards that takes two pins each and can act as an input/outout levelshifter to 3.3 V TTL levels from 0 and -3 V levels. Every pin can be completely disconnect by a relay. Every pin can also be connected to the measurement bus.
### Backplane board
The backplane contains glue logic and a measurement circuit. It also interconnects all signals between the other boards.
### Power supply
Makes all voltages needed for the tester.
### Load module
The load module can connect different resistors to the measurement bus in order to load test outputs from flipchips.
## Software
For the time beeing the software is run from a shell and takes different
input parameters. Run  brs-tester --help to get help text. When a board is
tested i takes a file with test vectors.

At startup a udev rule tiggered on the start of the GPIO subsysten will run
a shell script that will export all necessary GPIO pins and run
'brs-tester --init'. This will initialize the tester and put the hardware
in a known state.

## Using the tester
TBD

## Test vectors
There is a template file in the "vectors" folder that tries to explain how it works.

## Setting up the Raspberry Pi
In /boot/config.txt
Enable I2C, Set I2C clock to 400kHz

```
dtparam=i2c_arm=on
dtparam=i2c_arm_baudrate=400000
```

Enable support for ADS1115 ADC:

```
dtoverlay=ads1115
dtparam=cha_enable
dtparam=chb_enable
dtparam=chc_enable
dtparam=chd_enable
```

If you want a serial console connected to the pin header on the back:

```
enable_uart=1
```

The default speed is 115200 baud, if you want something else, change /boot/cmdline.txt

Install tools for build:

...
sudo apt install git
sudo apt install autoconf
sudo apt install autoconf-archive
...

Install libgpiod:

```
cd ~/
git clone https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
cd libgpiod
./autogen.sh
make
sudo make install
```

Get and install the brs-tester (this package),

```
cd ~/
git clone https://github.com/anders-bzn/brs-tester.git
cd brs-tester
make
sudo make install
```
