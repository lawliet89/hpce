#ifndef READ_WRITE_SYNC_H_
#define READ_WRITE_SYNC_H_
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>
#include <cstring>

class ReadWriteSync {
  bool debug = false;
  const int quanta = 1;
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> semaphore;
  bool _eof = false;
  std::string name;

public:
  ReadWriteSync() : semaphore(quanta * -1), name("") {
    if (getenv("HPCE_DEBUG") && strcmp(getenv("HPCE_DEBUG"), "true")){
      debug = true;
    }
  }

  void setName(std::string name) {
    this -> name = name;
  }

  std::unique_lock<std::mutex> producerWait() {
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

  void produce(std::unique_lock<std::mutex> &&lk) {
    if (debug)
      std::cerr << "[" << name << " Read] Read done. Updating semaphore."
                << std::endl;
    return updateUnlockAndNotify(std::move(lk), [&]{
      semaphore += quanta;
    });
  }

  void signalEof() {
    _eof = true;
  }

  // spin and spin
  void consumerWait() {
    if (debug)
      std::cerr << "[" << name << " Consume] Waiting for read to be done..."
                << std::endl;
    while (semaphore != 0 && !_eof);
  }

  void consume() {
    lockAndUpdate([&] { semaphore -= quanta; });
    if (debug)
      std::cerr << "[" << name << " Consume] Consumed" << std::endl;
  }

  void hintProducer() {
    if (debug)
      std::cerr << "[" << name << " Consume] Hinting read... " << std::endl;
    cv.notify_all();
  }

  bool eof() {
    return _eof;
  }

private:

  template <typename Callable> std::unique_lock<std::mutex> waitFor(
      Callable callable) {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, callable);
    return lk;
  }

  template <typename Callable> void lockAndUpdate(Callable callable) {
     std::unique_lock<std::mutex> lk(m);
     callable();
     lk.unlock();
  }

  template <typename Callable> void lockUpdateAndNotify(Callable callable) {
     std::unique_lock<std::mutex> lk(m);
     callable();
     lk.unlock();
     cv.notify_all();
  }

  template <typename Callable> void updateUnlockAndNotify(
    std::unique_lock<std::mutex> &&lk, Callable callable) {

     callable();
     lk.unlock();
     cv.notify_all();
  }

  template <typename Callable> bool tryLockUpdateAndNotify(Callable callable) {
     std::unique_lock<std::mutex> lk(m, std::defer_lock);
     if (!lk.try_lock()) return false;
     callable();
     lk.unlock();
     cv.notify_all();
     return true;
  }
};
#endif
