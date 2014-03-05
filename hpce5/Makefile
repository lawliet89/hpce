CPPFLAGS=-I -Wall -std=c++11 -O3 -pthread
CXX=clang++
SHELL=/bin/bash

all: process src/v2

src/housekeeping.o: src/include/housekeeping.hpp
src/window_1d.o: src/include/window_1d.hpp
src/v1: src/housekeeping.o src/window_1d.o
src/v2: src/housekeeping.o src/window_1d.o

clean:
	rm -f process src/v1 src/v2 src/window_1d.o src/housekeeping.o

.PHONY: all clean
