BIN = brs_tester
CFLAGS = -g -Wall -O3
CFILES=hal.c brs.c vector.c tests.c
HFILES=hal.h vector.h tests.h
LIBS=ugpio
RULES_DIR=/usr/lib/udev/rules.d
BIN_DIR=/usr/local/bin

INIT=brs-init.sh
RULES=99-brs_tester-init.rules

all: $(CFILES) $(HFILES) Makefile
	$(CC) $(CFILES) -o $(BIN) $(CFLAGS) -L/usr/local/lib -l$(LIBS) -Wl,-rpath=/usr/local/lib

host: $(CFILES) $(HFILES) Makefile
	$(CC) -D __HOST $(CFILES) -o $(BIN) $(CFLAGS)

install: all
	install -m 0755 $(BIN) $(BIN_DIR)
	install -m 0755 $(INIT) $(BIN_DIR)
	install -m 0644 $(RULES) $(RULES_DIR)
clean:
	$(RM) $(BIN)
