#include "include/housekeeping.hpp"
#include "include/window_1d.hpp"
#include <unistd.h>

#include <thread>
#include <mutex>
#include <condition_variable>

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
  Global Mutex
*/
bool start = false;
std::mutex readyMutex;
std::condition_variable readyCondition;

uint64_t readInput(uint8_t *buffer, uint32_t chunkSize, uint64_t bufferSize,
                   uint64_t imageSize);

void work() {
  std::unique_lock<std::mutex> lk(readyMutex);
  readyCondition.wait(lk, []{ return start; });
  std::cerr << "WORK WORK" << std::endl;
}

int main(int argc, char *argv[]) {
  try {
    std::thread readThread(work);

    processArgs(argc, argv, w, h, bits, levels);
    bufferSize = calculateBufferSize(w, h, bits, levels);
    chunkSize = calculateChunkSize(w, h, bits, levels, bufferSize);
    imageSize = calculateImageSize(w, h, bits);
    bufferPass1 = allocateBuffer(bufferSize);

    std::cerr << "Pass Buffer Size: " << bufferSize << std::endl;
    std::cerr << "Chunk Size: " << chunkSize << std::endl;

    {
      std::lock_guard<std::mutex> lk(readyMutex);
      start = true;
      std::cerr << "Ready!" << std::endl;
    }
    readyCondition.notify_all();

    readThread.join();
    // uint64_t chunkRead;
    // while ((chunkRead =
    //             readInput(bufferPass1, chunkSize, bufferSize, imageSize))) {
    //   window_1d(bufferPass1, nullptr, bufferSize, chunkSize, w, h,
    //             levels, bits);
    // }

    deallocateBuffer(bufferPass1);
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception : " << e.what() << "\n";
    return 1;
  }
}

uint64_t readInput(uint8_t *buffer, uint32_t chunkSize, uint64_t bufferSize,
                   uint64_t imageSize) {
  static uint8_t *readBuffer = nullptr;
  static uint64_t bytesReadSoFar = 0;

  if (readBuffer == nullptr)
    readBuffer = buffer;

  // uint8_t *outputStart = readBuffer;

  uint64_t bytesToRead = std::min(uint64_t(chunkSize), imageSize - bytesReadSoFar);
  uint64_t bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

  uint64_t finalBytesRead = bytesRead;
  bytesReadSoFar += bytesRead;
  readBuffer += bytesRead;

  // End of all images
  if (!bytesRead && readBuffer == buffer)
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

  return finalBytesRead;
}
