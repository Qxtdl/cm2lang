CC = gcc
CCFLAGS = -Wall -Wextra
SRC_SUBDIR = $(wildcard src/**/*.c)
SRC = $(wildcard src/*.c) $(SRC_SUBDIR)
BUILD_DIR = build/

.PHONY: all compile clean

all: clean compile run

clean:
	rm -rf $(BUILD_DIR)cm2lc

compile:
	$(CC) $(CCFLAGS) $(SRC) -o $(BUILD_DIR)cm2lc

run:
	./build/cm2lc examples/example.cm2l
