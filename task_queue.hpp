#ifndef TASK_QUEUE_HPP
#define TASK_QUEUE_HPP

#include <mutex>
#include <queue>
#include "function.hpp"

namespace TTF {

template <typename T>
class TaskQueue
{
private:
    std::queue<T> _queue;
    std::mutex    _mutex;
public:
    TaskQueue() {}
    TaskQueue(TaskQueue &&other) {}
    ~TaskQueue() {}
    bool empty()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.empty();
    }
    int size()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.size();
    }
    void emplace(T &t)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.emplace(forward<T>(t));
    }
    bool dequeue(T &t)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_queue.empty()) return false;
        t = forward<T>(_queue.front());
        _queue.pop();
        return true;
    }
};

} // namespace TTF

#endif // TASK_QUEUE_HPP