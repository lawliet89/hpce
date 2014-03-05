#include "include/housekeeping.hpp"
#include "include/conditional_mutex.hpp"
#include "include/window_1d.hpp"
#include <unistd.h>

#include <thread>
#include <atomic>

uint64_t readInput(uint8_t *buffer, uint32_t chunkSize, uint64_t bufferSize,
                   uint64_t imageSize, uint32_t levels,
                   ConditionalMutex &readConditional,
                   std::atomic<int> &readSemaphore);

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
    ConditionalMutex readConditional;
    std::atomic<int> readSemaphore;
    bool stop = false;

    processArgs(argc, argv, w, h, bits, levels);

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
                            std::ref(stop), std::ref(readConditional),
                            std::ref(readSemaphore));

    uint64_t chunkRead;
    while ((chunkRead =
                readInput(bufferPass1, chunkSize, bufferSize, imageSize,
                  levels, readConditional, readSemaphore))) {
    }
    // std::cerr << "[Read] Finished." << std::endl;
    stop = true;
    pass1Thread.join();

    deallocateBuffer(bufferPass1);
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception : " << e.what() << "\n";
    return 1;
  }
}

uint64_t readInput(uint8_t *buffer, uint32_t chunkSize, uint64_t bufferSize,
                   uint64_t imageSize, uint32_t levels,
                   ConditionalMutex &readConditional,
                   std::atomic<int> &readSemaphore) {
  static uint8_t *readBuffer = nullptr;
  static uint64_t bytesReadSoFar = 0;

  // std::cerr << "[Read] Waiting to start read" << std::endl;
  std::unique_lock<std::mutex> lock = readConditional.waitFor([&]{
    // std::cerr << "[Read] Woken. Checking semaphore = "
    //   << readSemaphore << std::endl;
    return (readSemaphore < 0);
  });

  if (readBuffer == nullptr)
    readBuffer = buffer;

  // uint8_t *outputStart = readBuffer;

  uint64_t bytesToRead = std::min(uint64_t(chunkSize), imageSize - bytesReadSoFar);
  uint64_t bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

  uint64_t finalBytesRead = bytesRead;
  bytesReadSoFar += bytesRead;
  readBuffer += bytesRead;

  // End of all images
  if (!bytesRead && bytesReadSoFar == 0)
    return 0u;

  while (bytesRead != bytesToRead) {
    std::cerr << "Warning: Chunk was not read in its entirety " << bytesRead
              << "/" << bytesToRead << "-- retrying" << std::endl;
    bytesToRead = bytesToRead - bytesRead;
    bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

    bytesReadSoFar += bytesRead;
    readBuffer += bytesRead;
    finalBytesRead += bytesRead;
  }

  if (bytesReadSoFar >= imageSize)
    bytesReadSoFar = 0;
  if (readBuffer >= buffer + bufferSize)
    readBuffer = buffer;

  // Test write to stdout
  // uint64_t written = 0;
  // while (written < finalBytesRead) {
  //   written += write(STDOUT_FILENO, outputStart + written,
  //     finalBytesRead - written);
  // }

  // std::cerr << "[Read] Read. Updating semaphore." << std::endl;
  readSemaphore += levels;
  lock.unlock();

  return finalBytesRead;
}
