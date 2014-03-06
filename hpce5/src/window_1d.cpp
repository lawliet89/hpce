#include "include/window_1d.hpp"
#include <unistd.h>
#include <iostream>
#include <mutex>

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
               const uint32_t chunk_size, const uint32_t img_width_pix,
               const uint32_t img_height_pix, const uint32_t n_levels,
               const uint8_t bit_width, ReadWriteSync &producer,
               ReadWriteSync &consumer)
{
  try{
    uint8_t *current = in_buf, *inBufEnd = in_buf + buf_size;
    uint8_t *currentWrite = out_buf, *outBufEnd = out_buf + buf_size;
    uint64_t bytesSoFar = 0;
    uint64_t imageSize = img_width_pix * img_height_pix * bit_width/8;

    while(1) {
      producer.consumerWait();

      if (producer.eof()) {
        consumer.signalEof();
        return;
      }

      producer.consume();
      std::unique_lock<std::mutex> lock = consumer.producerWait();

      // std::cerr << "[Window] Artificial Spinning..." << std::endl;
      // artificial spinning to take up time
      uint64_t acc = 0;
      for (uint32_t i = 0; i < chunk_size; ++i) {
        *(currentWrite + i) = *(current + i);

        if (i == chunk_size/2) {
          // try to signal reading thread
          producer.hintProducer();

        }
      }
      consumer.produce(std::move(lock));

      current += chunk_size;
      if (current >= inBufEnd) {
        current = in_buf;
      }

      currentWrite += chunk_size;
      if (currentWrite >= outBufEnd) {
        currentWrite = out_buf;
      }
      // std::cerr << "[Window] Chunk done " << acc << "\n";

      bytesSoFar += chunk_size;
      if (bytesSoFar >= imageSize) {
        std::cerr << "[Window] Image Boundary. Waitng for reset..." << std::endl;
        std::unique_lock<std::mutex> resetLock = consumer.waitForReset();

        // reset
        bytesSoFar = 0;
        currentWrite = out_buf;
        current = in_buf;
        consumer.resetDone(std::move(resetLock));
        producer.signalReset();
      }
    }
  }
  catch (std::exception &e) {
    // std::cerr << "Caught exception : " << e.what() << "\n";
    return;
  }
}
