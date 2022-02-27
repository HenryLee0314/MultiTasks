#include "log.h"

#include "function.hpp"
#include "task_queue.hpp"
#include "thread_pool.hpp"
#include "task.hpp"

#include <random>

int simulate_hard_computation()
{
    static std::uniform_int_distribution<int> distribution(0, 2000);
    static std::mt19937 generator;
    int sleep = distribution(generator);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    return sleep;
}

void SimpleFunction(int x) {
    CGRA_LOGD("%d", x);
    int sleep = simulate_hard_computation();
    CGRA_LOGD("%d, %d", x, sleep);
}

class SimpleClass
{
public:
    void operator()(int x, int y)
    {
        CGRA_LOGD("%d %d", x, y);
        int sleep = simulate_hard_computation();
        CGRA_LOGD("%d, %d", x, sleep);
    }
    SimpleClass() {};
};

struct SimpleStruct
{
    void operator()(int x)
    {
        CGRA_LOGD("%d", x);
        int sleep = simulate_hard_computation();
        CGRA_LOGD("%d, %d", x, sleep);
    }
    SimpleStruct() {};
};

auto lambda = [](int x) {
    CGRA_LOGD("%d", x);
    int sleep = simulate_hard_computation();
    CGRA_LOGD("%d, %d", x, sleep);
};

int main(int argc, char* argv[])
{
    CGRA_LOGD("main thread begins");

    SimpleClass simple_class;
    SimpleStruct simple_struct;

    TTF::ThreadPool thread_pool(3);
    TTF::TaskHolder holder;

    int i = 2;

    auto taskA = holder.emplace(SimpleFunction, 1);
    auto taskB = holder.emplace(simple_class, 2, i);
    auto taskC = holder.emplace(simple_struct, 3);
    auto taskD = holder.emplace(lambda, 4);
    auto taskE = holder.emplace(SimpleFunction, 5);
    auto taskF = holder.emplace(SimpleFunction, 6);
    auto taskG = holder.emplace(SimpleFunction, 7);

    TTF::Task::connect(taskB, taskA);
    TTF::Task::connect(taskC, taskA);
    TTF::Task::connect(taskD, taskA);

    TTF::Task::connect(taskA, taskE);

    TTF::Task::connect(taskB, taskF);
    TTF::Task::connect(taskC, taskF);

    TTF::Task::connect(taskE, taskG);
    TTF::Task::connect(taskF, taskG);

    thread_pool.run(holder);

    thread_pool.wait_to_end();

    CGRA_LOGD("main thread reaches the end");

    return 0;
}