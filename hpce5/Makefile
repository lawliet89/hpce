CPPFLAGS= -Wall -std=c++11 -O3 -pthread
#CXX=clang++
SHELL=/bin/bash

all: process src/v2

src/housekeeping.o: src/include/housekeeping.hpp
src/read_write_sync.o: src/include/read_write_sync.hpp
src/v2: src/housekeeping.o src/read_write_sync.o

clean:
	rm -f process src/v1 src/v2 src/window_1d.o src/housekeeping.o src/read_write_sync.o

.PHONY: all clean
