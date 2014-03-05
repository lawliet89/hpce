#ifndef HOUSEKEEPING_H_
#define HOUSEKEEPING_H_

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <mutex>
#include <condition_variable>

struct ConditionalMutex {
	std::mutex m;
	std::condition_variable cv;

	template <typename Callable> std::unique_lock<std::mutex> waitFor(Callable callable) {
	  std::unique_lock<std::mutex> lk(m);
	  cv.wait(lk, callable);
	  return lk;
	}
	template <typename Callable> void updateAndNotify(Callable callable) {
	   std::lock_guard<std::mutex> lk(m);
	   callable();
	   cv.notify_all();
	}
};

void processArgs(int argc, char *argv[], uint32_t &w, uint32_t &h,
                 uint32_t &bits, uint32_t &levels);
constexpr uint64_t calculateImageSize(uint32_t w, uint32_t h, uint32_t bits) {
  return w * h * bits / 8lu;
}
uint64_t calculateBufferSize(uint32_t w, uint32_t h, uint32_t bits,
                             uint32_t levels);
uint32_t calculateChunkSize(uint32_t w, uint32_t h, uint32_t bits,
                            uint32_t levels, uint64_t bufferSize);

uint8_t *allocateBuffer(uint64_t size, bool maximise = false);
void deallocateBuffer(uint8_t *buffer);

void zeroiseBuffer(uint8_t *buffer, uint64_t size);
void oneiseBuffer(uint8_t *buffer, uint64_t size);


#endif // HOUSEKEEPING_H_
