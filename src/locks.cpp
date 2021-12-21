#include "locks.h"
#include "main.h"

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <stack>
#include <cassert>
#include <iostream>


static void work_and_add_to_stack(int32_t value) {
	static std::mutex mut;
	static std::stack<int32_t> values;
	thread_sleep(250); // do some hard work
	{
		auto lock = std::scoped_lock(mut); // lock the mutex with a lock guard, RAII-style
		values.push(value);
	}
	thread_sleep(250); // more work after accessing shared data
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
	std::shared_mutex s_mut;
	int32_t value = 42;
	for (int32_t i = 0; i < 25; i++) {
		if (i % 5 == 0) { // writer
			threads.emplace([&s_mut, &value] {
				auto lock = std::unique_lock(s_mut, std::defer_lock);
				lock.lock();
				thread_sleep(100);
				value += 1;
			});
		} else { // reader
			auto t = std::thread([&s_mut, &value] {
				auto lock = std::shared_lock(s_mut, std::defer_lock);
				lock.lock();
				thread_sleep(100);
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
	std::cout << "Read/write time elapsed: " << elapsed.count() << "ms (100*25=2500ms)\n"; // we get ~2000ms: makes sense!

	// Producers and consumers
	start = std::chrono::system_clock::now();

	assert(threads.empty());
	std::mutex mut;
	std::condition_variable condition;
	std::queue<int32_t> tasks; // each task is an integer specifying how many ms to sleep (hard work!)


	for (int32_t i = 0; i < 5; i++) {
		threads.emplace([&] { // consumer
			auto lock = std::unique_lock(mut, std::defer_lock);
			while (true) {
				lock.lock();
				condition.wait(lock, [&tasks] { return !tasks.empty(); });
				int32_t chunk = tasks.front();
				tasks.pop();
				lock.unlock();
				if (chunk == -1) { // poison pill
					return;
				}
				thread_sleep(chunk); // process chunk
			}
		});

		auto tt = std::thread([&] { // producer
			static std::vector<int32_t> chunks_todo(5 * 40, 10);
			auto lock = std::unique_lock(mut, std::defer_lock);
			while (true) {
				thread_sleep(25); // produce chunks
				lock.lock();
				if (chunks_todo.empty()) {
					tasks.push(-1); // sentinel
					lock.unlock();
					return;
				}
				for (int i = 0; i < 5; i++) {
					tasks.push(chunks_todo.back());
					chunks_todo.pop_back();
				}
				lock.unlock();
				condition.notify_all();
			}	
		});
		tt.detach();
	}
	while (threads.size() > 0) {
		threads.front().join();
		threads.pop();
	}

	end = std::chrono::system_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Produce/consume time elapsed: " << elapsed.count() << "ms (seq. 4000ms)\n";
}
