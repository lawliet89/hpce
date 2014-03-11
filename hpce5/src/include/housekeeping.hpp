#ifndef HOUSEKEEPING_H_
#define HOUSEKEEPING_H_

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>

enum class Operation {
  ERODE,
  DILATE
};

void processArgs(int argc, char *argv[], uint32_t &w, uint32_t &h,
                 uint32_t &bits, uint32_t &levels, Operation &firstOp);

constexpr uint64_t calculateImageSize(uint32_t w, uint32_t h, uint32_t bits) {
  return ((uint64_t)w * h * bits) / 8lu;
}
uint64_t calculateBufferSize(uint32_t w, uint32_t h, uint32_t bits,
                             uint32_t levels);
uint32_t calculateChunkSize(uint32_t w, uint32_t h, uint32_t bits,
                            uint32_t levels, uint64_t bufferSize);

uint8_t *allocateBuffer(uint64_t size, bool maximise = false);
void deallocateBuffer(uint8_t *buffer);

void zeroiseBuffer(uint8_t *buffer, uint64_t size);
void oneiseBuffer(uint8_t *buffer, uint64_t size);

void trivialPassthrough();

#endif // HOUSEKEEPING_H_
