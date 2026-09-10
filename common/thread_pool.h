#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <utility>

class thread_pool {
public:
  explicit thread_pool(unsigned int thread_count) {
    if (thread_count == 0)
      thread_count = 1;

    try {
      for (unsigned int i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this]() { worker_loop(); });
      }
    } catch (...) {
      stop_and_join();
      throw;
    }
  }

  ~thread_pool() { stop_and_join(); }

  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

  std::size_t size() const { return workers_.size(); }

  void parallel_for(std::size_t count, std::function<void(std::size_t)> task) {
    if (count == 0)
      return;

    {
      std::lock_guard<std::mutex> lock(mutex_);

      task_ = std::move(task);
      task_count_ = count;
      next_task_ = 0;
      remaining_ = count;
      error_ = nullptr;
    }

    work_ready_.notify_all();

    std::exception_ptr error;

    {
      std::unique_lock<std::mutex> lock(mutex_);

      all_done_.wait(lock, [this]() { return remaining_ == 0; });

      error = error_;
      task_ = nullptr;
    }

    if (error)
      std::rethrow_exception(error);
  }

private:
  std::vector<std::thread> workers_;

  std::mutex mutex_;
  std::condition_variable work_ready_;
  std::condition_variable all_done_;

  std::function<void(std::size_t)> task_;
  std::size_t task_count_ = 0;
  std::size_t next_task_ = 0;
  std::size_t remaining_ = 0;

  bool stopping_ = false;
  std::exception_ptr error_;

  void worker_loop() {
    while (true) {
      std::size_t index;

      {
        std::unique_lock<std::mutex> lock(mutex_);

        work_ready_.wait(lock, [this]() { return stopping_ || next_task_ < task_count_; });

        if (stopping_)
          return;

        index = next_task_++;
      }

      std::exception_ptr error;

      try {
        // 무거운 계산은 mutex를 잡지 않은 상태에서 수행.
        task_(index);
      } catch (...) {
        error = std::current_exception();
      }

      {
        std::lock_guard<std::mutex> lock(mutex_);

        if (error && !error_)
          error_ = error;

        --remaining_;

        if (remaining_ == 0)
          all_done_.notify_one();
      }
    }
  }

  void stop_and_join() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = true;
    }

    work_ready_.notify_all();

    for (auto& worker : workers_) {
      if (worker.joinable())
        worker.join();
    }
  }
};

#endif