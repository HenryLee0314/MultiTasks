#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <thread>
#include <mutex>

#include "task.hpp"
#include "task_queue.hpp"

#include "log.h"

namespace TTF {

class ThreadPool
{
private:
    class ThreadWorker
    {
    private:
        ThreadPool* _pool;
    public:
        ThreadWorker(ThreadPool *pool) : _pool(pool)
        {
        }

        void operator()()
        {
            Task task;
            bool dequeued;
            while (!_pool->_stop)
            {
                {
                    std::unique_lock<std::mutex> lock(_pool->_conditional_mutex);

                    if (_pool->_queue.empty())
                    {
                        _pool->_conditional_lock.wait(lock);
                    }

                    dequeued = _pool->_queue.dequeue(task);
                }

                if (dequeued) {
                    task();
                    {
                        std::unique_lock<std::mutex> lock(_pool->_push_task_mutex);
                        task.solved(_pool);
                    }
                    
                }
            }
        }
    };

    std::vector<std::thread> _threads;
    bool _stop;
    TaskQueue<Task> _queue;
    std::vector<Task> _tasks;

    std::mutex _conditional_mutex;
    std::condition_variable _conditional_lock;

    std::mutex _push_task_mutex;

public:
    ThreadPool(const size_t thread_size = std::thread::hardware_concurrency())
        : _threads(std::vector<std::thread>(thread_size)), _stop(true)
    {
        
    }

    ~ThreadPool() {
        wait_to_end();
    }

    void wait_to_end() {
        // if (!_stop) {
        //     while (!_queue.empty()) {}
        // }

        // _stop = true;
        _conditional_lock.notify_all();

        for (size_t i = 0; i < _threads.size(); ++i) {
            if (_threads.at(i).joinable()) _threads.at(i).join();
        }
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    void emplace(Task* task) {
        if (task) {
            if (_stop && task->able()) {
                _tasks.push_back(*task);
            }
            else if (!_stop && task->able()) {
                _queue.emplace(*task);
                _conditional_lock.notify_one();
            }
            else {
                // nothing, task is not able to run
            }
        }
    }

    void run() {
        _stop = false;
        for (size_t i = 0; i < _threads.size(); ++i)
        {
            _threads.at(i) = std::thread(ThreadWorker(this));
        }
        for (size_t i = 0; i < _tasks.size(); i++) {
            _queue.emplace(_tasks[i]);
        }
        _conditional_lock.notify_one();
    }



};


} // namespace TTF

#endif // THREAD_POOL_HPP