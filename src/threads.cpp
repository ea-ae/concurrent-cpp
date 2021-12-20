#include "threads.h"
#include "main.h"

#include <thread>
#include <future>
#include <vector>
#include <iostream>
#include <string>


int Foo::work(int32_t id, std::shared_ptr<int> input) {
	for (int32_t j = 0; j < 1'000'000; j++) {
		*input += (*input % 2) + (j % 10) + id;
	}
	return id;
}


void run_threads() {
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
	for (auto &thread : threads)
		thread.join();

	std::vector<std::future<int32_t>> tasks;
	std::vector<std::shared_ptr<int32_t>> results; // just for the sake of it!
	Foo my_foo;
	for (uint32_t i = 0; i < g_processors; i++) {
		results.push_back(std::make_shared<int32_t>(0));
		tasks.push_back(std::async(&Foo::work, &my_foo, i, results[i]));
	}
	for (size_t i = 0; i < tasks.size(); i++)
		std::cout << "Done at thread " << i << ": <return " << tasks[i].get() << "> <result " << *results[i] << ">\n";
}
