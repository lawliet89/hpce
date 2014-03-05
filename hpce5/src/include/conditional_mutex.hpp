#ifndef CONDITIONAL_MUTEX_H_
#define CONDITIONAL_MUTEX_H_
#include <mutex>
#include <condition_variable>

struct ConditionalMutex {
	std::mutex m;
	std::condition_variable cv;

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

	void notify_all() {
		cv.notify_all();
	}
};
#endif
