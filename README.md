# About the brs-tester

This project implements a tester for DEC flipchips in the B/R/S series.
Those modules are used in PDP-8, PDP-8/S, PDP-9 and in PDP-10 with KA processor.
This project was created as a part of the restoration of the PDP-9 with
serial number 203.

The modules uses 0 and -3V volts for the logical 0 and 1. The tester can detect both logical and electrical failures.

The software runs on a Raspberry Pi with [Raspberry Pi OS](https://www.raspberrypi.com/software/). It's been tested with Raspberry Pi OS 11/12 on Raspberry Pi 3/4/5.

![BRS-tester](photos/brs-tester.jpg)

## Electronics
The major parts is:

- Levelshifter boards
- Backplane board with control logic and measurement circuitry
- Power supply
- Load module

### Levelshifter boards
There are fifteen boards that takes two pins each and can act as an input/output level shifter to 3.3 V TTL levels from 0 and -3 V levels. Every pin can be completely disconnect by a relay. Every pin can also be connected to the measurement bus.
### Backplane board
The backplane contains glue logic and a measurement circuit. It also interconnects all signals between the other boards.
### Power supply
Makes all voltages needed for the tester.
### Load module
The load module can connect different resistors to the measurement bus in order to load test outputs from flipchips.
## Software
For the time being the software is run from a shell and takes different
input parameters. Run  brs-tester --help to get help text. When a board is
tested i takes a file with test vectors.

At startup a udev rule triggered on the start of the GPIO subsystem will run
a shell script that will export all necessary GPIO pins and run
'brs-tester init'. This will initialize the tester and put the hardware
in a known state.

## Using the tester
Initialize the hardware
```
$ brs-tester init
```
Run a self test of the hardware, no test object should be in the tester
```
$ brs-tester selftest
```
Run test on a board, loop logical test loop number of times.
```
$ brs-tester test --vector=vectors/b104.fct --loop=10
```
Turn on power to the test object.
```
$ brs-tester debug --power-enable=on
```

## Test vectors
There is a template file in the "vectors" folder that tries to explain how it works.

## Setting up the Raspberry Pi

Download [Raspberry Pi OS 12 Lite](https://downloads.raspberrypi.com/raspios_lite_arm64/images/raspios_lite_arm64-2024-07-04/2024-07-04-raspios-bookworm-arm64-lite.img.xz) (the linked version is what this guide has been written for).

The software uses an ADC connected to the I2C-1 on Raspberry Pi. To enable the ADC support a devicetree overlay needs to be loaded so that the Linux kernel know how it should be probed. On Raspberry Pi this is done by adding lines in the /boot/firmware/config.txt file.


Enable I2C, Set I2C clock to 400kHz and enable support for ADS1115 ADC:

```
dtparam=i2c_arm=on
dtparam=i2c_arm_baudrate=400000
dtoverlay=ads1115
dtparam=cha_enable
dtparam=chb_enable
dtparam=chc_enable
dtparam=chd_enable
```

If you want a serial console connected to the pin header on the back if the backplane board:

```
enable_uart=1
```

All those changes above can be done manually or by running the setup script.
```
sudo ./setup-pi.sh
```
If serial console shall be enables add the 'UART' parameter to the script.
```
sudo ./setup-pi.sh UART
```

After the changes in /boot/firmware/config.txt is saved, you need to reboot. When enabling serial console, the default speed is 115200 baud. If you want something else, change /boot/firmware/cmdline.txt

## Building the software
Install tools for build:

```
sudo apt install git
sudo apt install autoconf
sudo apt install autoconf-archive
sudo apt install libtool
```

Build and install libgpiod from source (the shipped Debian libgpiod-dev package is to old):

```
cd ~/
git clone https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
cd libgpiod
git checkout v2.1.3
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
