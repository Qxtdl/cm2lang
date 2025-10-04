CC = gcc
CCFLAGS = -Wall -Wextra
SRC = $(wildcard src/*.c)
BUILD_DIR = build/

.PHONY: all compile clean

all: clean compile

clean:
	rm -rf $(BUILD_DIR)*

compile:
	$(CC) $(CCFLAGS) $(SRC) -o $(BUILD_DIR)cm2lc

