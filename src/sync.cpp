#include "sync.h"

#include <atomic>
#include <barrier>
#include <iostream>
#include <latch>
#include <string>
#include <thread>
#include <vector>

#include "main.h"

void try_syncing() {
    // Latches and barriers

    std::latch lt(3);
    std::barrier br(2, []() noexcept { std::cout << "Barrier open!\n"; });

    auto example = [&](std::string name) {
	std::string pr = name + " is waiting\n";
	std::cout << pr;

	if (name == "B") {
	    br.arrive_and_drop();
	    // br.arrive_and_wait(); // in case we used arrive_and_wait above, wait again together with C
	} else {
	    br.arrive_and_wait();
	}
	pr = name + " is past barrier\n";
	std::cout << pr;

	lt.arrive_and_wait();  // single-use, most cases have another thread waiting for latch to reach 0
	pr = name + " is past latch\n";
	std::cout << pr;
    };

    std::vector<std::jthread> threads;
    for (auto name : std::string("ABC")) {
	threads.emplace_back(example, std::string(1, name));
	thread_sleep(30);
    }

    // Atomic operations

    uint32_t a = 0, b = 0;
    std::atomic<uint_fast32_t> c = 0, d = 0;
    std::barrier floodgate(100);

    for (int32_t i = 0; i < 200; i++) {  // non-atomic
	std::jthread([&] { floodgate.arrive_and_wait(); a++; b++; }).detach();  // sometimes finishes with 199
    }
    for (int32_t i = 0; i < 200; i++) {  // atomic
	std::jthread([&] { floodgate.arrive_and_wait(); c++; d++; }).detach();
    }

    thread_sleep(250);  // joins? lazy.
    std::cout << "Non-atomic result is " << a << " & " << b << " and atomic result is " << c << " & " << d << "\n";
}
