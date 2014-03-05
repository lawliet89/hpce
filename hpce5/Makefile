CPPFLAGS=-I -Wall -std=c++11 -O3
CXX=clang++
SHELL=/bin/bash

all: process src/v1

#src/housekeeping.o: src/include/housekeeping.hpp

housekeeping = src/housekeeping.o
src/v1: $(housekeeping)

.PHONY: all
