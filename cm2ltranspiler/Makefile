CC = gcc
CCFLAGS = -Wall -Wextra -fsanitize=address -O1 -g
SRC_SUBDIR = $(wildcard src/**/*.c)
SRC = $(wildcard src/*.c) $(SRC_SUBDIR)
BUILD_DIR = build/

.PHONY: all compile clean

all: clean compile run

clean:
	rm -rf $(BUILD_DIR)cm2lt

compile:
	$(CC) $(CCFLAGS) $(WARNINGS) $(SRC) -o $(BUILD_DIR)cm2lt

CM2LTFLAGS =
run:
	./build/cm2lt examples/rv32i.cm2lhd examples/compiled.s build/transpiled.s $(CM2LTFLAGS)
