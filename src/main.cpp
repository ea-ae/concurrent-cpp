#include "main.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "bathroom_problem.h"
#include "locks.h"
#include "modern.h"
#include "sync.h"
#include "threads.h"

const uint32_t g_processors = std::thread::hardware_concurrency();

void thread_sleep(int32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main() {
    std::cout << "Hello World!\n";
    Modern::modern();  // Modern C++ features (non-concurrency)
    // try_threads(); // Threads, futures, promises, tasks
    // try_locks(); // Mutexes, control variables
    // try_syncing(); // Latches, barriers, atomics
    // bathroom_problem(); // Unisex bathroom problem
    // work_and_add_to_stack(64);
}
