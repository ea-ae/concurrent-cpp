#include "main.h"

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <stack>
#include <queue>
#include <cassert>
#include <iostream>


static void work_and_add_to_stack(int32_t value) {
	static std::mutex mut;
	static std::stack<int32_t> values;
	std::this_thread::sleep_for(std::chrono::milliseconds(250)); // do some hard work
	{
		std::scoped_lock<std::mutex> lock(mut); // lock the mutex with a lock guard, RAII-style
		values.push(value);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(250)); // more work after accessing shared data
}

void try_locks() {
	// Write lock
	auto start = std::chrono::system_clock::now();
	std::queue<std::thread> threads;

	for (uint32_t i = 0; i < g_processors; i++) { // create thread for every processor
		threads.emplace(work_and_add_to_stack, i);
	}
	while (threads.size() > 0) {
		threads.front().join();
		threads.pop();
	}

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Mutex time elapsed: " << elapsed.count() << "ms (500*4=2000ms)\n";

	// Readers and writers
	start = std::chrono::system_clock::now();

	assert(threads.empty());
	std::shared_mutex mut;
	int32_t value = 42;
	for (int32_t i = 0; i < 50; i++) {
		if (i % 5 == 0) { // writer
			threads.emplace([&mut, &value] {
				std::unique_lock<std::shared_mutex> lock(mut, std::defer_lock);
				lock.lock();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				value += 1;
			});
		} else { // reader
			auto t = std::thread([&mut, &value] {
				std::shared_lock<std::shared_mutex> lock(mut, std::defer_lock);
				lock.lock();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			});
			t.detach();
		}
	}
	while (threads.size() > 0) {
		threads.front().join();
		threads.pop();
	}

	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Final value is " << value << ". ";
	std::cout << "Read/write time elapsed: " << elapsed.count() << "ms (100*50=5000ms)\n"; // we get ~2000ms: makes sense!
}
