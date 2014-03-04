#include "include/housekeeping.hpp"
#include <cstring>

void processArgs(int argc, char *argv[], uint32_t &w, uint32_t &h,
                 uint32_t &bits, uint32_t &levels) {
  if (argc < 3) {
    fprintf(stderr, "Usage: process width height [bits] [levels]\n");
    fprintf(stderr, "   bits=8 by default\n");
    fprintf(stderr, "   levels=1 by default\n");
    exit(1);
  }

  w = atoi(argv[1]);
  h = atoi(argv[2]);

  if (argc > 3) {
    bits = atoi(argv[3]);
  }

  if (bits > 32)
    throw std::invalid_argument("Bits must be <= 32.");

  uint32_t tmp = bits;
  while (tmp != 1) {
    tmp >>= 1;
    if (tmp == 0)
      throw std::invalid_argument("Bits must be a binary power.");
  }

  if (((w * bits) % 64) != 0) {
    throw std::invalid_argument(" width*bits must be divisible by 64.");
  }

  levels = 1;
  if (argc > 4) {
    levels = atoi(argv[4]);
  }

  if (abs(levels) > std::min(std::min(w / 4u, h / 4u), 64u)) {
    throw std::invalid_argument(
        "0 <= abs(levels) <= min(width/4, height/4, 64)");
  }

  fprintf(stderr, "Processing %d x %d image with %d bits per pixel.\n", w, h,
          bits);
}

uint64_t calculateBufferSize(uint32_t w, uint32_t h, uint32_t bits,
                             uint32_t levels) {
  // Minimum buffer size requried
  uint64_t minBufferSize = (bits / 8) * (w * (2 * levels) + 2 * levels + 1);
  // make sure it is not bigger than image size
  minBufferSize = std::min(calculateImageSize(w, h, bits), minBufferSize);

  // Round actual buffer size to be a power of 2
  uint64_t bufferSize =
      uint64_t(1u) << uint32_t(floor(log2(minBufferSize) + 1));

  assert(bufferSize >= minBufferSize);
  return bufferSize;
}

// TODO: Optimise this
uint64_t calculateChunkSize(uint32_t w, uint32_t h, uint32_t bits,
                            uint32_t levels, uint64_t bufferSize) {
  uint32_t widthExponent = log2(w * bits) - 3;
  // range 512 - 4k
  // maximum 4K
  uint64_t chunkSize =
      std::min(uint64_t(4096u), uint64_t(1u) << (widthExponent / 4));
  // min 512
  chunkSize = std::max(uint64_t(512u), chunkSize);

  // but must be smaller than bufferSize
  chunkSize = std::min(chunkSize, bufferSize);

  return chunkSize;
}

uint8_t *allocateBuffer(uint64_t size, bool maximise) {
  uint8_t *ptr = new uint8_t[size]();
  if (maximise)
    memset(ptr, 0xff, size);
  return ptr;
}

void deallocateBuffer(uint8_t *buffer) { delete[] buffer; }
