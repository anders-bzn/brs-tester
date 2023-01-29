
BIN = brs_tester
CFLAGS = -g -Wall -O3

CFILES=hal.c brs.c
HFILES=hal.h
LIBS=ugpio

all: $(CFILES) $(HFILES) Makefile
	$(CC) $(CFILES) -o $(BIN) $(CFLAGS) -L/usr/local/lib -l$(LIBS) -Wl,-rpath=/usr/local/lib

clean:
	$(RM) $(BIN)
