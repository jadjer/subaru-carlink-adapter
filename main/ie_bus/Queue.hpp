//
// Created by jadjer on 03.12.2025.
//

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Queue {
public:
    auto push(T item) -> void;
    [[nodiscard]] auto pop() -> T;
    [[nodiscard]] auto isEmpty() -> bool;

private:
    std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_condition;
};

template <typename T>
auto Queue<T>::push(T item) -> void {
    std::unique_lock lock(m_mutex);

    m_queue.push(item);
    m_condition.notify_one();
}

template <typename T>
auto Queue<T>::pop() -> T {
    std::unique_lock lock(m_mutex);

    m_condition.wait(lock, [this] { return not m_queue.empty(); });

    T const item = m_queue.front();
    m_queue.pop();

    return item;
}

template <typename T>
auto Queue<T>::isEmpty() -> bool {
    std::unique_lock lock(m_mutex);

    auto const isEmpty = m_queue.empty();

    return isEmpty;
}
