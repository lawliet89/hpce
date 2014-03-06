#ifndef READ_WRITE_SYNC_H_
#define READ_WRITE_SYNC_H_
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

class ReadWriteSync {
  const bool debug = false;
  const int quanta = 1;
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> semaphore;
  bool _eof = false;

public:
  ReadWriteSync() : semaphore(quanta * -1) { }

  std::unique_lock<std::mutex> producerWait() {
    if (debug)
      std::cerr << "[Read] Waiting to start read" << std::endl;
    return waitFor([&] {
      if (debug) {
        std::cerr << "[Read] Woken. Checking semaphore = "
           << semaphore << std::endl;
      }
      return semaphore < 0;
    });
  }

  void produce(std::unique_lock<std::mutex> &&lk) {
    if (debug)
      std::cerr << "[Read] Read done. Updating semaphore." << std::endl;
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
      std::cerr << "[Window] Waiting for read to be done..." << std::endl;
    while (semaphore != 0 && !_eof);
  }

  void consume() {
    lockAndUpdate([&] { semaphore -= quanta; });
    if (debug)
      std::cerr << "[Window] Consumed" << std::endl;
  }

  void hintProducer() {
    if (debug)
      std::cerr << "[Window] Hinting read... " << std::endl;
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
