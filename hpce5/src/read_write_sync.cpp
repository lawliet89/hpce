#include "include/read_write_sync.hpp"
#include <iostream>

void ReadWriteSync::setName(std::string name) {
  this -> name = name;
}

std::unique_lock<std::mutex> ReadWriteSync::producerWait() {
  if (debug)
    std::cerr << "[" << name << " Read] Waiting to start read" << std::endl;
  return waitFor([&] {
    if (debug) {
      std::cerr << "[" << name << " Read] Woken. Checking semaphore = "
         << semaphore << std::endl;
    }
    return semaphore < 0;
  });
}

void ReadWriteSync::produce(std::unique_lock<std::mutex> &&lk) {
  if (debug)
    std::cerr << "[" << name << " Read] Read done. Updating semaphore."
              << std::endl;
  return updateUnlockAndNotify(std::move(lk), [&]{
    semaphore += quanta;
  });
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
  lockAndUpdate([&] { semaphore -= quanta; });
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

std::unique_lock<std::mutex> ReadWriteSync::waitFor(std::function<bool()> callable) {
  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, callable);
  return lk;
}

void ReadWriteSync::lockAndUpdate(std::function<void()> callable) {
   std::unique_lock<std::mutex> lk(m);
   callable();
   lk.unlock();
}

void ReadWriteSync::lockUpdateAndNotify(std::function<void()> callable) {
   std::unique_lock<std::mutex> lk(m);
   callable();
   lk.unlock();
   cv.notify_all();
}

void ReadWriteSync::updateUnlockAndNotify(
  std::unique_lock<std::mutex> &&lk, std::function<void()> callable) {

   callable();
   lk.unlock();
   cv.notify_all();
}

bool ReadWriteSync::tryLockUpdateAndNotify(std::function<void()> callable) {
   std::unique_lock<std::mutex> lk(m, std::defer_lock);
   if (!lk.try_lock()) return false;
   callable();
   lk.unlock();
   cv.notify_all();
   return true;
}
