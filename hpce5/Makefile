CPPFLAGS=-Xclang -load -Xclang
CPPFLAGS+=/home/yongwen/work/fyp/llvm/projects/LLVM-Obfuscator/Debug+Asserts/lib/LLVMObfuscatorTransforms.so
CPPFLAGS+= -Wall -std=c++11 -O3 -pthread
# CPPFLAGS+= -mllvm -debug-only=boguscf
# CPPFLAGS+= -mllvm -bcfProbability=1.0
# CPPFLAGS += -mllvm -print-after-all
CXX=/home/yongwen/work/fyp/build/Debug+Asserts/bin/clang++
SHELL=/bin/bash

all: process src/v2
process:
	g++  -Wall -std=c++11 -O3 -pthread   process.cpp   -o process
src/housekeeping.o: src/include/housekeeping.hpp
src/read_write_sync.o: src/include/read_write_sync.hpp
src/v2: src/housekeeping.o src/read_write_sync.o

clean:
	rm -f process src/v1 src/v2 src/window_1d.o src/housekeeping.o src/read_write_sync.o

.PHONY: all clean
