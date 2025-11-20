CC = gcc
CCFLAGS = -std=c2x -pedantic -Wall -Wextra -O1 -g # -fsanitize=address
SRC_SUBDIR = $(wildcard src/**/*.c)
SRC = $(wildcard src/*.c) $(SRC_SUBDIR)
BUILD_DIR = build/

.PHONY: all compile compile_transpiler just_compile_lang clean

all: clean compile run

clean:
	rm -rf $(BUILD_DIR)cm2lc $(BUILD_DIR)cm2lt

compile_transpiler:
	$(MAKE) -C cm2ltranspiler compile

just_compile_lang:
	$(CC) $(CCFLAGS) $(WARNINGS) $(SRC) -o $(BUILD_DIR)cm2lc

compile: just_compile_lang compile_transpiler
	mv cm2ltranspiler/build/cm2lt build/cm2lt

CM2LCFLAGS = -fstack-init=0 -fhalf-size=2 -fno-syntax-analyzer
CM2LTFLAGS =
run:
	./build/cm2lc examples/fib.cm2l build/compiled.s $(CM2LCFLAGS)
	./build/cm2lt cm2ltranspiler/examples/rv32i.cm2lhd build/compiled.s build/transpiled.s $(CM2LTFLAGS)
