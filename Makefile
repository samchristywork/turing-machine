CC := gcc

LIBS := -lcjson

all: build/turing_machine

build/turing_machine: turing_machine.c
	mkdir -p build/
	${CC} turing_machine.c -o $@ ${LIBS}

clean:
	rm -rf build/

.PHONY: clean
