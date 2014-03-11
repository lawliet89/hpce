#ifndef READ_WRITE_SYNC_H_
#define READ_WRITE_SYNC_H_
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstring>

class ReadWriteSync {
  bool debug = false;

  const int quanta = 1;
  std::mutex m;
  std::condition_variable cv;
  std::atomic<int> semaphore;

  bool _eof = false;

  std::mutex resetMutex;
  std::condition_variable resetCv;
  bool reset = false;
  
  std::string name;

public:
  ReadWriteSync() : semaphore(quanta * -1), name("") {
    if (getenv("HPCE_DEBUG") && strcmp(getenv("HPCE_DEBUG"), "true")) {
      debug = true;
    }
  }

  void setName(std::string name);
  std::unique_lock<std::mutex> producerWait();

  void produce(std::unique_lock<std::mutex> &&lk);

  void signalEof();

  // spin and spin
  void consumerWait();
  void consume();

  void hintProducer();

  bool eof();

  std::unique_lock<std::mutex> waitForReset();
  void resetDone(std::unique_lock<std::mutex> &&lk);
  void signalReset();
};

#endif
