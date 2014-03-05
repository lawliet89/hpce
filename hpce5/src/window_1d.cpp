#include "include/window_1d.hpp"
#include <unistd.h>
#include <iostream>
#include <cassert>

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
               const uint32_t chunk_size, const uint32_t img_width_pix,
               const uint32_t img_height_pix, const uint32_t n_levels,
               const uint8_t bit_width,
               bool &stop, ConditionalMutex &readConditional,
               std::atomic<int> &readSemaphore)
{
  uint8_t *current = in_buf;

  while(1) {
    // std::cerr << "[Window] Waiting for read to be done..." << std::endl;
    while(readSemaphore != 0 && !stop);

    if (stop) {
      // std::cerr << "[Window] Exiting" << std::endl;
      return;
    }

    // A lock here is necessary to ensure that the reader thread is not awake
    // and checking on condition
    readConditional.lockAndUpdate([&] { readSemaphore -= n_levels; });

    // std::cerr << "[Window] Artificial Spinning..." << std::endl;
    // artificial spinning to take up time
    uint64_t acc = 0;
    for (uint32_t i = 0; i < chunk_size; ++i) {
      acc += *(current);

      if (i == chunk_size/2) {
        // try to signal reading thread
        readConditional.notify_all();
        // std::cerr << "[Window] Hinting read... " << std::endl;

      }
    }

    // Test write
    write(STDOUT_FILENO, current, chunk_size);

    current += chunk_size;
    if (current >= (in_buf + buf_size)) {
      current = in_buf;
    }
    // std::cerr << "[Window] Chunk done " << acc << std::endl;
  }
}
