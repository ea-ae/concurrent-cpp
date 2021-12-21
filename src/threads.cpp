#include "threads.h"
#include "main.h"

#include <thread>
#include <future>
#include <vector>
#include <string>
#include <iostream>


int Foo::work(int32_t id, std::shared_ptr<int> input) {
	for (int32_t j = 0; j < 1'000'000; j++) {
		*input += (*input % 2) + (j % 10) + id;
	}
	return id;
}


void try_threads() {
	std::vector<std::thread> threads;
	for (uint32_t i = 0; i < g_processors; i++) { // create thread for every processor
		threads.emplace_back([i]() { // perform some calculations in each thread
			int32_t x = i;
			for (int32_t j = 0; j < 1'000'000; j++) { 
				x += (i % 2) + (j % 10);
			}
			std::string result = "Done at thread " + std::to_string(i) + ".\n"; // needed for thread-safe cout
			std::cout << result;
		});
	}
	for (auto &t : threads)
		t.join();

	std::vector<std::future<int32_t>> tasks;
	std::vector<std::shared_ptr<int32_t>> results; // just for the sake of it!
	Foo my_foo;
	for (uint32_t i = 0; i < g_processors; i++) {
		results.push_back(std::make_shared<int32_t>(0));
		tasks.push_back(std::async(&Foo::work, &my_foo, i, results[i]));
	}
	for (size_t i = 0; i < tasks.size(); i++)
		std::cout << "Done at thread " << i << ": <return " << tasks[i].get() << "> <result " << *results[i] << ">\n";

	std::packaged_task<int32_t(int32_t, int32_t)> addition_task([](int32_t a, int32_t b) { return a + b; });
	auto add_fut = addition_task.get_future();
	int32_t x = 3;
	std::packaged_task subtraction_task([x](int32_t a) -> int32_t { return a - x; }); // can we steal vars here?
	std::promise<int32_t> fnord;
	auto prom_fut = fnord.get_future();

	auto task_runner = std::thread([&addition_task, &subtraction_task, &fnord] { 
		fnord.set_value_at_thread_exit(666);
		thread_sleep(200);
		addition_task(10, 5);
		subtraction_task(10); // 10-3=7
	});
	task_runner.detach();

	auto sub_fut = subtraction_task.get_future();
	std::cout << "Add: " << add_fut.get() << " Sub: " << sub_fut.get() << " Promise: " << prom_fut.get() << "\n";

	// Let's crash! Replace std::jthread with std::thread, abort() will be called on thread destruction
	auto the_eater_of_souls = std::jthread([] { thread_sleep(1000); });
}
