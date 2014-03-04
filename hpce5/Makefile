CPPFLAGS=-I -Wall -std=c++11 -O3
CXX=clang++
SHELL=/bin/bash

housekeeping = src/housekeeping.o

all: process src/v1

src/housekeeping.o: src/include/housekeeping.hpp
src/v1: $(housekeeping)


.PHONY: all
