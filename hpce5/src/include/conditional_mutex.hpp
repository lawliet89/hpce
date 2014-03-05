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
	template <typename Callable> void lockUpdateAndNotify(Callable callable) {
	   std::lock_guard<std::mutex> lk(m);
	   callable();
	   cv.notify_all();
	}

	template <typename Callable> void updateAndNotify(Callable callable) {
	   callable();
	   cv.notify_all();
	}

	template <typename Callable> bool tryLockUpdateAndNotify(Callable callable) {
	   std::unique_lock<std::mutex> lk(m, std::defer_lock);
	   if (!lk.try_lock()) return false;
	   callable();
	   cv.notify_all();
	   return true;
	}

	void notify_all() {
		cv.notify_all();
	}
};
