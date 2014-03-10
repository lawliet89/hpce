#include "include/read_write_sync.hpp"
#include <iostream>

void ReadWriteSync::setName(std::string name) { this->name = name; }

std::unique_lock<std::mutex> ReadWriteSync::producerWait()
{
  if (debug)
    std::cerr << "[" << name << " Consume] Waiting to start read" << std::endl;

  std::unique_lock<std::mutex> lk(m);
  cv.wait(lk, [&] {
    if (debug) {
      std::cerr << "[" << name
                << " Consume] Woken. Checking semaphore = " << semaphore
                << std::endl;
    }
    return semaphore < 0;
  });
  return lk;
}

void ReadWriteSync::produce(std::unique_lock<std::mutex> &&lk)
{
  if (debug)
    std::cerr << "[" << name << " Consume] Read done. Updating semaphore."
              << std::endl;
  semaphore += quanta;
  lk.unlock();
  cv.notify_all();
}

void ReadWriteSync::signalEof()
{
  if (debug)
    std::cerr << "[" << name << " Consume] EOF reached" << std::endl;
  _eof = true;
}

// spin and spin
void ReadWriteSync::consumerWait()
{
  if (debug)
    std::cerr << "[" << name << " Consume] Waiting for read to be done..."
              << std::endl;
  while (semaphore != 0 && !_eof)
    ;
}

void ReadWriteSync::consume()
{
  std::unique_lock<std::mutex> lk(m);
  semaphore -= quanta;
  lk.unlock();
  if (debug)
    std::cerr << "[" << name << " Consume] Consumed" << std::endl;
}

void ReadWriteSync::hintProducer()
{
  if (debug)
    std::cerr << "[" << name << " Consume] Hinting read... " << std::endl;
  cv.notify_all();
}

bool ReadWriteSync::eof() { return _eof; }

std::unique_lock<std::mutex> ReadWriteSync::waitForReset()
{
  std::unique_lock<std::mutex> lk(resetMutex);
  if (debug)
    std::cerr << "[" << name << " Consume] Waiting for reset.. " << std::endl;
  resetCv.wait(lk, [&] { return reset; });
  if (debug)
    std::cerr << "[" << name << " Consume] Resetting " << std::endl;
  return lk;
}

void ReadWriteSync::resetDone(std::unique_lock<std::mutex> &&lk)
{
  reset = false;
  lk.unlock();
  resetCv.notify_all();
}

void ReadWriteSync::signalReset()
{
  std::unique_lock<std::mutex> lk(resetMutex);
  if (debug)
    std::cerr << "[" << name << " Consume] Signalling Reset.. " << std::endl;
  reset = true;
  lk.unlock();
  resetCv.notify_all();
}
