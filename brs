#!/bin/bash

# Alloc gpio's
GPIOS="18 24 25 8 7 1 12 16 20 21 26 19 13 6 5 10 27 4 0 23"

for GPIO in $GPIOS; do {
	[ -e  /sys/class/gpio/gpio$GPIO ] || echo $GPIO > /sys/class/gpio/export
} done

sleep 1

./brs_tester $@
