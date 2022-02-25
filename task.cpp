#include "task.hpp"
#include "thread_pool.hpp"

#include "log.h"

namespace TTF {

Task::Task()
    : _dependence(0)
{

}

Task::~Task()
{

}

bool Task::able()
{
    return _dependence == 0;
}

void Task::solved(ThreadPool* the_pool) {
    for (int i = 0; i < _next.size(); i++) {
        _next[i]->_dependence -= 1;
        if (_next[i]->able()) {
            the_pool->emplace(_next[i]);
        }
    }
}


} // namespace TTF