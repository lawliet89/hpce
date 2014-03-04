#ifndef Housekeeping_H
#define Housekeeping_H

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>

void processArgs(int argc, char *argv[], uint32_t &w, uint32_t &h,
                 uint32_t &bits, uint32_t &levels);
constexpr uint64_t calculateImageSize(uint32_t w, uint32_t h, uint32_t bits) {
  return w * h * bits / 8lu;
}
uint64_t calculateBufferSize(uint32_t w, uint32_t h, uint32_t bits,
                             uint32_t levels);
uint64_t calculateChunkSize(uint32_t w, uint32_t h, uint32_t bits,
                            uint32_t levels, uint64_t bufferSize);

uint8_t *allocateBuffer(uint64_t size, bool maximise = false);
void deallocateBuffer(uint8_t *buffer);

#endif // Housekeeping_H
