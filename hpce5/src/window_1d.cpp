#include "include/window_1d.hpp"
#include "include/extern.hpp"
#include "include/conditional_mutex.hpp"
#include <atomic>
#include <iostream>
#include <cassert>

extern bool stop;
extern ConditionalMutex readConditional;
extern std::atomic<int> readSemaphore;

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
               const uint32_t chunk_size, const uint32_t img_width_pix,
               const uint32_t img_height_pix, const uint32_t n_levels,
               const uint8_t bit_width)
{
  while(1) {
    std::cerr << "[Window] Waiting for read to be done..." << std::endl;
    while(readSemaphore != 0 && !stop);

    if (stop) {
      std::cerr << "[Window] Exiting" << std::endl;
      return;
    }

    std::cerr << "[Window] Artificial Spinning..." << std::endl;
    // artificial spinning to take up time
    uint64_t acc = 0;
    for (uint32_t i = 0; i < chunk_size; ++i) {
      acc += *(in_buf);
    }

    // A lock here is necessary because the following could happen:
    // - reader has lock, and is checking condition.
    // - As it has read the value and/or checked the value, we modify
    // - Before it goes back to sleep, we notify
    // - And therefore reader never gets woken up again
    readConditional.lockUpdateAndNotify([=] { readSemaphore -= n_levels; });
    std::cerr << "[Window] Chunk done" << std::endl;
  }
}
