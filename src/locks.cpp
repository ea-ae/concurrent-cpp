#include "main.h"

#include <mutex>
#include <thread>
#include <chrono>
#include <stack>
#include <queue>
#include <iostream>


void work_and_add_to_stack(int32_t value) {
	static std::mutex mut;
	static std::stack<int32_t> values;
	std::this_thread::sleep_for(std::chrono::milliseconds(300)); // do some hard work
	{
		std::scoped_lock<std::mutex> lock(mut); // lock the mutex with a lock guard, RAII-style
		values.push(value);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(300)); // more work after accessing shared data
}

void try_locks() {
	std::queue<std::thread> threads;
	auto start = std::chrono::system_clock::now();

	for (uint32_t i = 0; i < g_processors; i++) { // create thread for every processor
		threads.emplace(work_and_add_to_stack, i);
	}
	while (threads.size() > 0) {
		threads.front().join();
		threads.pop();
	}

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Mutex time elapsed: " << elapsed.count() << "ms\n";
}
