#!/bin/bash

CFILE="/boot/firmware/config.txt"

if [ -z "$(grep -x "dtparam=i2c_arm=on" "$CFILE")" ]; then
    echo "dtparam=i2c_arm=on" >> "$CFILE"
    echo Add "dtparam=i2c_arm=on" to "$CFILE"
fi

if [ -z "$(grep -x "dtparam=i2c_arm_baudrate=400000" "$CFILE")" ]; then
    echo "dtparam=i2c_arm_baudrate=400000" >> "$CFILE"
    echo Add "dtparam=i2c_arm_baudrate=400000" to "$CFILE"
fi

if [ -z "$(grep -x "dtoverlay=ads1115" "$CFILE")" ]; then
    echo "dtoverlay=ads1115" >> "$CFILE"
    echo Add "dtoverlay=ads1115" to "$CFILE"
fi

if [ -z "$(grep -x "dtparam=cha_enable" "$CFILE")" ]; then
    echo "dtparam=cha_enable" >> "$CFILE"
    echo Add "dtparam=cha_enable" to "$CFILE"
fi

if [ -z "$(grep -x "dtparam=chb_enable" "$CFILE")" ]; then
    echo "dtparam=chb_enable" >> "$CFILE"
    echo Add "dtparam=chb_enable" to "$CFILE"
fi

if [ -z "$(grep -x "dtparam=chc_enable" "$CFILE")" ]; then
    echo "dtparam=chc_enable" >> "$CFILE"
    echo Add "dtparam=chc_enable" to "$CFILE"
fi

if [ -z "$(grep -x "dtparam=chd_enable" "$CFILE")" ]; then
    echo "dtparam=chd_enable" >> "$CFILE"
    echo Add "dtparam=chd_enable" to "$CFILE"
fi

if [ "$1" == "UART" ]; then
    if [ -z "$(grep -x "enable_uart=1" "$CFILE")" ]; then
        echo "enable_uart=1" >> "$CFILE"
        echo Add "enable_uart=1" to "$CFILE"
    fi
fi
