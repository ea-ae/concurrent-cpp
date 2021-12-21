#include "main.h"
#include "threads.h"
#include "locks.h"
#include "sync.h"

#include <thread>
#include <iostream>


const uint32_t g_processors = std::thread::hardware_concurrency();

void thread_sleep(int32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main()
{
    std::cout << "Hello World!\n";
    // try_threads(); // Threads, futures, promises, tasks
    //try_locks(); // Mutexes, control variables
    try_syncing();
}
