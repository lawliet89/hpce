CPPFLAGS=-I -Wall -std=c++11 -O3
CXX=clang++
SHELL=/bin/bash

all: process src/v1

src/housekeeping.o: src/include/housekeeping.hpp
src/window_1d.o: src/include/window_1d.hpp
src/v1: src/housekeeping.o src/window_1d.o

.PHONY: all
