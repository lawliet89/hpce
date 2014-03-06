#include "include/read_write_sync.hpp"
#include <iostream>

void ReadWriteSync::setName(std::string name) {
  this -> name = name;
}

std::unique_lock<std::mutex> ReadWriteSync::producerWait() {
  if (debug)
    std::cerr << "[" << name << " Read] Waiting to start read" << std::endl;

  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, [&] {
    if (debug) {
      std::cerr << "[" << name << " Read] Woken. Checking semaphore = "
         << semaphore << std::endl;
    }
    return semaphore < 0;
  });
  return lk;
}

void ReadWriteSync::produce(std::unique_lock<std::mutex> &&lk) {
  if (debug)
    std::cerr << "[" << name << " Read] Read done. Updating semaphore."
              << std::endl;
  semaphore += quanta;
  lk.unlock();
  cv.notify_all();
}

void ReadWriteSync::signalEof() {
  _eof = true;
}

// spin and spin
void ReadWriteSync::consumerWait() {
  if (debug)
    std::cerr << "[" << name << " Consume] Waiting for read to be done..."
              << std::endl;
  while (semaphore != 0 && !_eof);
}

void ReadWriteSync::consume() {
  std::unique_lock<std::mutex> lk(m);
  semaphore -= quanta;
  lk.unlock();
  if (debug)
    std::cerr << "[" << name << " Consume] Consumed" << std::endl;
}

void ReadWriteSync::hintProducer() {
  if (debug)
    std::cerr << "[" << name << " Consume] Hinting read... " << std::endl;
  cv.notify_all();
}

bool ReadWriteSync::eof() {
  return _eof;
}

void ReadWriteSync::waitForReset() {

}
