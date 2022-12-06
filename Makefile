CC := gcc

CFLAGS := -g -Wall -Wpedantic
LIBS := -lcjson

all: build/turing_machine

build/turing_machine: turing_machine.c version.h
	mkdir -p build/
	${CC} ${CFLAGS} turing_machine.c -o $@ ${LIBS}

clean:
	rm -rf build/

.PHONY: clean
