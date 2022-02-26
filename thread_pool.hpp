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
            std::shared_ptr<Task> task;
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
                    task->operator()();
                    {
                        std::unique_lock<std::mutex> lock(_pool->_push_task_mutex);
                        task->check_following(_pool);
                        _pool->_holder->remove(task);
                    }
                    
                }
            }
        }
    };

    std::vector<std::thread> _threads;
    bool _stop;
    TaskQueue<std::shared_ptr<Task>> _queue;

    std::mutex _conditional_mutex;
    std::condition_variable _conditional_lock;

    std::mutex _push_task_mutex;

    std::shared_ptr<TaskHolder> _holder;


public:
    ThreadPool(const size_t thread_size = std::thread::hardware_concurrency())
        : _threads(std::vector<std::thread>(thread_size)), _stop(true)
    {
        
    }

    ~ThreadPool() {
        wait_to_end();
    }

    void wait_to_end() {
        if (!_stop) {
            while (!(_queue.empty() && (_holder->_tasks.size() == 0))) {}
        }

        _stop = true;
        _conditional_lock.notify_all();

        for (size_t i = 0; i < _threads.size(); ++i) {
            if (_threads.at(i).joinable()) _threads.at(i).join();
        }
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;


    void emplace(std::shared_ptr<Task> task) {
        if (task) {
            _queue.emplace(task);
            _conditional_lock.notify_one();
        }
        
    }

    void run(TaskHolder& holder) {
        _holder = std::make_shared<TaskHolder>(holder);
        _stop = false;
        for (size_t i = 0; i < _threads.size(); ++i)
        {
            _threads.at(i) = std::thread(ThreadWorker(this));
        }

        for (auto task : holder.executable()) {
            _queue.emplace(task);
            _conditional_lock.notify_one();
        }
    }

};


} // namespace TTF

#endif // THREAD_POOL_HPP