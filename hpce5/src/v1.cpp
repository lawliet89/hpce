#include "include/housekeeping.hpp"
#include <unistd.h>

/*
  Global Parameters
*/
uint32_t w, h, bits = 8, levels = 1;
uint64_t bufferSize, chunkSize, imageSize;

/*
  Global Buffers
*/
uint8_t *bufferPass1 = nullptr;

bool readInput(uint8_t *buffer, uint64_t chunkSize, uint64_t bufferSize,
               uint64_t imageSize);

int main(int argc, char *argv[]) {
  try {
    processArgs(argc, argv, w, h, bits, levels);
    bufferSize = calculateBufferSize(w, h, bits, levels);
    chunkSize = calculateChunkSize(w, h, bits, levels, bufferSize);
    imageSize = calculateImageSize(w, h, bits);
    bufferPass1 = allocateBuffer(bufferSize);

    std::cerr << "Pass Buffer Size: " << bufferSize << std::endl;
    std::cerr << "Chunk Size: " << chunkSize << std::endl;

    while (readInput(bufferPass1, chunkSize, bufferSize, imageSize)) {
    }

    deallocateBuffer(bufferPass1);
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception : " << e.what() << "\n";
    return 1;
  }
}

bool readInput(uint8_t *buffer, uint64_t chunkSize, uint64_t bufferSize,
               uint64_t imageSize) {
  static uint8_t *readBuffer = nullptr;
  static uint64_t bytesReadSoFar = 0;

  if (readBuffer == nullptr)
    readBuffer = buffer;

  uint64_t bytesToRead = std::min(chunkSize, imageSize - bytesReadSoFar);
  uint64_t bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

  bytesReadSoFar += bytesRead;
  readBuffer += bytesRead;

  // End of all images
  if (!bytesRead && readBuffer == buffer)
    return false;

  while (bytesRead != bytesToRead) {
    std::cerr << "Warning: Chunk was not read in its entirety " << bytesRead
              << "/" << bytesToRead << "-- retrying" << std::endl;
    bytesToRead = bytesToRead - bytesRead;
    bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

    bytesReadSoFar += bytesRead;
    readBuffer += bytesRead;
  }

  if (bytesReadSoFar >= imageSize)
    bytesReadSoFar = 0;
  if (readBuffer >= buffer + bufferSize)
    readBuffer = buffer;

  return true;
}
