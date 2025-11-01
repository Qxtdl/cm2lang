CC = gcc
CCFLAGS = -Wall -Wextra -fsanitize=address -O1 -g
SRC_SUBDIR = $(wildcard src/**/*.c)
SRC = $(wildcard src/*.c) $(SRC_SUBDIR)
BUILD_DIR = build/

.PHONY: all compile clean

all: clean compile run

clean:
	rm -rf $(BUILD_DIR)cm2lc

compile:
	$(CC) $(CCFLAGS) $(WARNINGS) $(SRC) -o $(BUILD_DIR)cm2lc

CM2LCFLAGS = -fstack-init=0 -fhalf-size=2
CM2LTFLAGS =
run:
	./build/cm2lc examples/example.cm2l build/compiled.s $(CM2LCFLAGS)
	./build/cm2lt cm2ltranspiler/examples/rv32i.cm2lhd build/compiled.s build/transpiled.s $(CM2LTFLAGS)
