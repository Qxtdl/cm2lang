CC = gcc
CCFLAGS = -Wall -Wextra -fsanitize=address -O1 -g
SRC_SUBDIR = $(wildcard src/**/*.c)
SRC = $(wildcard src/*.c) $(SRC_SUBDIR)
BUILD_DIR = build/

.PHONY: all compile compile_transpiler clean

all: clean compile run

clean:
	rm -rf $(BUILD_DIR)cm2lc $(BUILD_DIR)cm2lt

compile_transpiler:
	$(MAKE) -C cm2ltranspiler compile

compile: compile_transpiler
	$(CC) $(CCFLAGS) $(WARNINGS) $(SRC) -o $(BUILD_DIR)cm2lc
	mv cm2ltranspiler/build/cm2lt build/cm2lt

CM2LCFLAGS = -fstack-init=65535 -fhalf-size=1
CM2LTFLAGS =
run:
	./build/cm2lc examples/example.cm2l build/compiled.s $(CM2LCFLAGS)
	./build/cm2lt cm2ltranspiler/examples/Zait.cm2lhd build/compiled.s build/transpiled.s $(CM2LTFLAGS)
