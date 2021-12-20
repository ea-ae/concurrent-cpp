#include "locks.h"
#include "main.h"

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <semaphore>
#include <thread>
#include <chrono>
#include <vector>
#include <stack>
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

static void produce(std::queue<int32_t> &tasks) {
	static std::vector<int32_t> chunks_todo(200, 10);
	int32_t chunk = chunks_todo.back();
	chunks_todo.pop_back();
	std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int32_t>(round(chunk / 2))));
}

static void consume(std::queue<int32_t> &tasks) {

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
	for (int32_t i = 0; i < 50; i++) {
		if (i % 5 == 0) { // writer
			threads.emplace([&s_mut, &value] {
				std::unique_lock<std::shared_mutex> lock(s_mut, std::defer_lock);
				lock.lock();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				value += 1;
			});
		} else { // reader
			auto t = std::thread([&s_mut, &value] {
				std::shared_lock<std::shared_mutex> lock(s_mut, std::defer_lock);
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

	// Producers and consumers
	start = std::chrono::system_clock::now();

	assert(threads.empty());
	std::mutex mut;
	std::condition_variable condition;
	std::queue<int32_t> tasks; // each task is an integer specifying how many ms to sleep (hard work!)


	for (int32_t i = 0; i < 5; i++) {
		threads.emplace([&mut, &condition, &tasks] { // consumer
			std::unique_lock<std::mutex> lock(mut, std::defer_lock);
			while (true) {
				lock.lock();
				condition.wait(lock, [&tasks] { return !tasks.empty(); });
				int32_t chunk = tasks.front();
				tasks.pop();
				lock.unlock();
				if (chunk == -1) { // poison pill
					return;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(chunk)); // process chunk
			}
		});

		auto tt = std::thread([&mut, &condition, &tasks] { // producer
			static std::vector<int32_t> chunks_todo(5 * 40, 10);
			std::unique_lock<std::mutex> lock(mut, std::defer_lock);
			while (true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(25)); // produce chunks
				lock.lock();
				if (chunks_todo.empty()) {
					tasks.push(-1); // sentinel
					lock.unlock();
					return;
				}
				for (int i = 0; i < 5; i++) {
					int32_t chunk = chunks_todo.back();
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
