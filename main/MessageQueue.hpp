//
// Created by jadjer on 06.01.2026.
//

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

template <class T> class MessageQueue {
private:
  std::queue<T> queue;
  std::mutex mtx;
  std::condition_variable cv;

public:
  auto push(T res) -> void {
    {
      std::scoped_lock lock(mtx);
      queue.push(res);
    }
    cv.notify_one();
  }

  auto pop() -> std::optional<T> {
    std::unique_lock<std::mutex> lock(mtx);

    if (queue.empty()) {
      return std::nullopt;
    }

    T res = queue.front();
    queue.pop();
    return res;
  }
};
