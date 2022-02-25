#ifndef TASK_H
#define TASK_H

#include <vector>
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
        _func = [&f{forward<F>(f)}, &...args{forward<Args>(args...)}]() {
            (f)(args...);
        };
    }

    void rely_on(Task* task) {
        if (task) {
            task->_next.push_back(this);
            _dependence += 1;
        }
    }

    bool able();

    void solved(ThreadPool* the_pool);

private:
    Function<void()>   _func;
    std::vector<Task*> _next;
    int                _dependence;
};

} // namespace TTF

#endif // TASK_H