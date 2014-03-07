#include "include/housekeeping.hpp"
#include "include/read_write_sync.hpp"
#include "include/window_1d.hpp"
#include <unistd.h>

#include <thread>
#include <atomic>

void readInput(uint8_t *const buffer, const uint32_t chunkSize,
               const uint64_t bufferSize, const uint64_t imageSize,
               ReadWriteSync &sync, uint8_t *const intermediateBuffer,
               const Operation firstOp);

void writeOutput(uint8_t *const buffer, const uint32_t chunkSize,
                 const uint64_t bufferSize, const uint64_t imageSize,
                 ReadWriteSync &sync);

int main(int argc, char *argv[]) {
  try {
    /*
      Global Parameters
    */
    uint32_t w, h, bits = 8, levels = 1;
    uint64_t bufferSize, imageSize;
    uint32_t chunkSize;
    Operation firstOp;

    /*
      Global Buffers
    */
    uint8_t *stdinBuffer = nullptr;
    uint8_t *intermediateBuffer = nullptr;
    uint8_t *stdoutBuffer = nullptr;
    /*
      Global Synchronisation Primitives
    */
    ReadWriteSync stdinSync, passSync, stdoutSync;

    stdinSync.setName("stdin");
    passSync.setName("intermediate");
    stdoutSync.setName("stdout");

    processArgs(argc, argv, w, h, bits, levels, firstOp);

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
    stdinBuffer = allocateBuffer(bufferSize, firstOp == Operation::ERODE);
    intermediateBuffer =
        allocateBuffer(bufferSize, firstOp == Operation::DILATE);
    stdoutBuffer = allocateBuffer(bufferSize);

    std::cerr << "Image Size: " << imageSize << std::endl;
    std::cerr << "Pass Buffer Size: " << bufferSize << std::endl;
    std::cerr << "Chunk Size: " << chunkSize << std::endl;

    // Set up concurrency
    std::thread pass1Thread(window_1d, stdinBuffer, stdoutBuffer, bufferSize,
                            chunkSize, w, h, levels, bits, std::ref(stdinSync),
                            std::ref(stdoutSync));
    // std::thread pass2Thread(window_1d, intermediateBuffer, stdoutBuffer,
    //                         bufferSize, chunkSize, w, h, levels, bits,
    //                         std::ref(passSync), std::ref(stdoutSync));
    std::thread writeThread(writeOutput, stdoutBuffer, chunkSize, bufferSize,
                            imageSize, std::ref(stdoutSync));

    readInput(stdinBuffer, chunkSize, bufferSize, imageSize, stdinSync,
              intermediateBuffer, firstOp);

    pass1Thread.join();
    // pass2Thread.join();
    writeThread.join();

    deallocateBuffer(stdinBuffer);
    deallocateBuffer(intermediateBuffer);
    deallocateBuffer(stdoutBuffer);
    return 0;
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception : " << e.what() << "\n";
    return 1;
  }
}

void readInput(uint8_t *const buffer, const uint32_t chunkSize,
               const uint64_t bufferSize, const uint64_t imageSize,
               ReadWriteSync &sync, uint8_t *const intermediateBuffer,
               const Operation firstOp) {

  uint8_t *readBuffer = buffer;
  uint64_t bytesReadSoFar = 0;
  while (1) {
    std::unique_lock<std::mutex> lock = sync.producerWait();

    uint64_t bytesToRead =
        std::min(uint64_t(chunkSize), imageSize - bytesReadSoFar);
    uint64_t bytesRead = read(STDIN_FILENO, readBuffer, bytesToRead);

    // End of all images
    if (!bytesRead && bytesReadSoFar == 0) {
      sync.signalEof();
      return;
    }

    while (bytesRead < bytesToRead) {
      std::cerr << "Warning: Chunk was not read in its entirety " << bytesRead
                << "/" << bytesToRead << "-- retrying" << std::endl;
      bytesRead +=
          read(STDIN_FILENO, readBuffer + bytesRead, bytesToRead - bytesRead);
    }

    // Always go by chunkSize
    bytesReadSoFar += chunkSize;
    readBuffer += chunkSize;

    if (readBuffer >= buffer + bufferSize)
      readBuffer = buffer;

    sync.produce(std::move(lock));

    if (bytesReadSoFar >= imageSize) {
      std::cerr << "[Read] Image Boundary. Waitng for reset..." << std::endl;
      std::unique_lock<std::mutex> resetLock = sync.waitForReset();

      // Perform resets
      bytesReadSoFar = 0;
      readBuffer = buffer;

      if (firstOp == Operation::ERODE) {
        oneiseBuffer(buffer, bufferSize);
        zeroiseBuffer(intermediateBuffer, bufferSize);
      } else {
        oneiseBuffer(intermediateBuffer, bufferSize);
        zeroiseBuffer(buffer, bufferSize);
      }

      std::cerr << "[Read] Reset" << std::endl;
      sync.resetDone(std::move(resetLock));
    }
  }
}

void writeOutput(uint8_t *const buffer, const uint32_t chunkSize,
                 const uint64_t bufferSize, const uint64_t imageSize,
                 ReadWriteSync &sync) {

  uint8_t *current = buffer;
  uint64_t bytesSoFar = 0u;

  while (1) {
    sync.consumerWait();
    if (sync.eof()) {
      return;
    }

    uint64_t writeBytes = std::min(uint64_t(chunkSize), imageSize - bytesSoFar);

    // TODO: Retry
    write(STDOUT_FILENO, current, writeBytes);

    bytesSoFar += chunkSize;
    current += chunkSize;

    if (current >= buffer + bufferSize)
      current = buffer;

    sync.consume();
    sync.hintProducer();

    if (bytesSoFar >= imageSize) {
      bytesSoFar = 0;
      current = buffer;
      std::cerr << "[Write] Image Boundary. Signalling reset." << std::endl;
      sync.signalReset();
    }
  }
}
