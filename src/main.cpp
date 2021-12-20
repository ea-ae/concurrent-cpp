#include "main.h"
#include "threads.h"
#include "locks.h"

#include <thread>
#include <iostream>


const uint32_t g_processors = std::thread::hardware_concurrency();

int main()
{
    std::cout << "Hello World!\n";
    try_threads(); // Threads and futures
    try_locks(); // Mutexes and more
}
