#ifndef TASK_H
#define TASK_H

#include <vector>
#include <set>
#include <mutex>

#include "function.hpp"

namespace TTF {

class ThreadPool;

class Task
{
public:
    Task();

    ~Task();

    void operator()() {
        _func();
    }

    template <typename F, typename... Args>
    void set_func(F &&f, Args &&...args) {
        _func = [&f{forward<F>(f)}, &...args{forward<Args>(args)}]() {
            (f)(args...);
        };
    }

    static void connect(std::shared_ptr<Task> task_last, std::shared_ptr<Task> task_next) {
        if (task_last && task_next) {
            task_last->_next.push_back(task_next);
            task_next->_dependence += 1;
        }
    }

    bool able();

    void check_following(ThreadPool* the_pool);

private:
    Function<void()>                    _func;
    std::vector<std::shared_ptr<Task>>  _next;
    int                                 _dependence;
};

class TaskHolder
{
    public:
    TaskHolder()
    {

    }

    ~TaskHolder()
    {

    }

    template <typename F, typename... Args>
    std::shared_ptr<Task> emplace(F &&f, Args &&...args) {
        auto task = std::shared_ptr<Task>(new Task());
        task->set_func(f, args...);
        _tasks.insert(task);
        return task;
    }

    void remove(std::shared_ptr<Task> task) {
        auto iter = _tasks.find(task);
        if (iter != _tasks.end()) {
            _tasks.erase(iter);
        }
    }
    

    std::vector<std::shared_ptr<Task>> executable() {
        std::vector<std::shared_ptr<Task>> ret;
        for (auto i : _tasks) {
            if (i->able()) ret.push_back(i);
        }
        return ret;
    }

    std::set<std::shared_ptr<Task>> _tasks;
};

} // namespace TTF

#endif // TASK_H