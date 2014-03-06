#include "include/housekeeping.hpp"
#include "include/conditional_mutex.hpp"
#include "include/window_1d.hpp"
#include <unistd.h>

#include <thread>
#include <atomic>

void readInput(uint8_t *buffer, uint32_t chunkSize, uint64_t bufferSize,
                   uint64_t imageSize, uint32_t levels,
                   ConditionalMutex &readConditional,
                   std::atomic<int> &readSemaphore, bool &consumerStop);

int main(int argc, char *argv[]) {
  try {
    /*
      Global Parameters
    */
    uint32_t w, h, bits = 8, levels = 1;
    uint64_t bufferSize, imageSize;
    uint32_t chunkSize;

    /*
      Global Buffers
    */
    uint8_t *bufferPass1 = nullptr;

    /*
      Global Synchronisation Primitives
    */

    // stdin read -> first pass
    ConditionalMutex readConditional;
    std::atomic<int> readSemaphore;
    bool readStop = false;

    processArgs(argc, argv, w, h, bits, levels);

    if (levels == 0) {
      trivialPassthrough();
      return 0;
    }

    bufferSize = calculateBufferSize(w, h, bits, levels);
    chunkSize = calculateChunkSize(w, h, bits, levels, bufferSize);

    // The bufferSize is "ideal". To allow us to read-ahead by one chunk, we
    // shall extend its size by one chunk
    bufferSize += chunkSize;

    imageSize = calculateImageSize(w, h, bits);
    bufferPass1 = allocateBuffer(bufferSize);

    std::cerr << "Pass Buffer Size: " << bufferSize << std::endl;
    std::cerr << "Chunk Size: " << chunkSize << std::endl;

    // Set up concurrency
    readSemaphore = levels * -1;
    std::thread pass1Thread(window_1d, bufferPass1, nullptr, bufferSize,
                            chunkSize, w, h, levels, bits,
                            std::ref(readStop), std::ref(readConditional),
                            std::ref(readSemaphore));

    readInput(bufferPass1, chunkSize, bufferSize, imageSize,
      levels, readConditional, readSemaphore, readStop);
    pass1Thread.join();

    deallocateBuffer(bufferPass1);
    return 0;
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception : " << e.what() << "\n";
    return 1;
  }
}

void readInput(uint8_t *buffer, uint32_t chunkSize, uint64_t bufferSize,
                   uint64_t imageSize, uint32_t levels,
                   ConditionalMutex &readConditional,
                   std::atomic<int> &readSemaphore, bool &consumerStop) {

  uint8_t *readBuffer = nullptr;
  uint64_t bytesReadSoFar = 0;
  while(1) {
    // std::cerr << "[Read] Waiting to start read" << std::endl;
    std::unique_lock<std::mutex> lock = readConditional.waitFor([&]{
      // std::cerr << "[Read] Woken. Checking semaphore = "
      //   << readSemaphore << std::endl;
      return (readSemaphore < 0);
    });

    if (readBuffer == nullptr)
      readBuffer = buffer;

    uint64_t bytesToRead = std::min(uint64_t(chunkSize), imageSize - bytesReadSoFar);
    uint64_t bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

    // End of all images
    if (!bytesRead && bytesReadSoFar == 0) {
      consumerStop = true;
      return;
    }

    while (bytesRead < bytesToRead) {
      // std::cerr << "Warning: Chunk was not read in its entirety " << bytesRead
      //           << "/" << bytesToRead << "-- retrying" << std::endl;
      bytesRead += read(STDIN_FILENO, readBuffer + bytesRead,
        bytesToRead - bytesRead);
    }

    // Always go by chunkSize
    bytesReadSoFar += chunkSize;
    readBuffer += chunkSize;

    assert(bytesReadSoFar <= imageSize);
    if (bytesReadSoFar == imageSize)
      bytesReadSoFar = 0;
    if (readBuffer >= buffer + bufferSize)
      readBuffer = buffer;

    // std::cerr << "[Read] Read. Updating semaphore." << std::endl;
    readSemaphore += levels;
    lock.unlock();
    readConditional.notify_all();
  }
}
