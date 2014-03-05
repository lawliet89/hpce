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

      // if (i == chunk_size/2) {
      //   // try to signal reading thread
      //   readConditional.notify_all();
      //   std::cerr << "[Window] Hinting read... " << std::endl;

      // }
    }

    readSemaphore -= n_levels;
    // readConditional.notify_all();
    std::cerr << "[Window] Chunk done" << std::endl;
  }
}
