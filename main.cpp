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
    void operator()(int x)
    {
        CGRA_LOGD("%d", x);
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
    TTF::Task taskA, taskB, taskC, taskD;
    taskA.set_func(SimpleFunction, 1);
    taskB.set_func(simple_class, 2);
    taskC.set_func(simple_struct, 3);
    taskD.set_func(lambda, 4);


    taskA.rely_on(&taskB);
    taskA.rely_on(&taskC);
    taskA.rely_on(&taskD);

    TTF::Task taskE, taskF, taskG;
    taskE.set_func(SimpleFunction, 5);
    taskE.rely_on(&taskA);

    taskF.set_func(SimpleFunction, 6);
    taskF.rely_on(&taskB);
    taskF.rely_on(&taskC);


    taskG.set_func(SimpleFunction, 7);
    taskG.rely_on(&taskE);
    taskG.rely_on(&taskF);


    thread_pool.emplace(&taskA);
    thread_pool.emplace(&taskB);
    thread_pool.emplace(&taskC);
    thread_pool.emplace(&taskD);
    thread_pool.emplace(&taskE);
    thread_pool.emplace(&taskF);
    thread_pool.emplace(&taskG);

    thread_pool.run();

    thread_pool.wait_to_end();

    CGRA_LOGD("main thread reaches the end");

    return 0;
}