#include "bathroom_problem.h"

#include <atomic>
#include <barrier>
#include <iostream>
#include <mutex>
#include <semaphore>
#include <vector>

#include "main.h"

// == Unisex Bathroom Problem ==
// An attempted solution.
// Both men and women may (non-politically) enter a unisex bathroom that contains space for N people total,
// but only people of a single gender may be inside at the same time.

constexpr auto SPACE = 3;
enum class Gender { unknown,
                    male,
                    female };

void bathroom_problem() {  // assuming n=3
    std::vector<std::jthread> threads;
    for (int i = 0; i < 100; i++) {
	threads.emplace_back(use_bathroom, i % 2 == 0 ? Gender::male : Gender::female);
    }
    for (auto& t : threads) t.join();
}

std::atomic<Gender> occupied_gender = Gender::unknown;
std::barrier queue_floodgate(25);

std::atomic<int> inside = 0;
uint32_t male_count, female_count;
std::mutex male_count_mut, female_count_mut;
std::binary_semaphore empty_sem(1);
std::binary_semaphore door(1);
std::counting_semaphore male_sem(SPACE);
std::counting_semaphore female_sem(SPACE);

void use_bathroom(Gender gender) {
    // wait for x threads to enter, creating a flood of emergency bathroom use / contention for sake of example
    // we will prioritize the same type of resource (M/F) entering first, because else we would have to wait
    // for all n resources of a type to finish before allowing a new one (MMMFFFMMMFFF = inefficient)
    // edit: if we do want to avoid starvation and balance resource type loads, enable the door mutex
    thread_sleep(100);
    queue_floodgate.arrive_and_wait();  // barrier of x

    std::string gender_name = gender == Gender::male ? "male" : "female";

    door.acquire();
    std::unique_lock count_lock(gender == Gender::male ? male_count_mut : female_count_mut);
    if (++(gender == Gender::male ? male_count : female_count) == 1) {
	empty_sem.acquire();
    }
    count_lock.unlock();
    (gender == Gender::male ? male_sem : female_sem).acquire();
    door.release();
    std::string s = "A " + gender_name + " has entered the bathroom (" + std::to_string(++inside) + ")\n";

    std::cout << s;

    thread_sleep(10);  // use bathroom

    count_lock.lock();
    s = "A " + gender_name + " has left the bathroom (" + std::to_string(--inside) + ")\n";
    if (--(gender == Gender::male ? male_count : female_count) == 0) {
	empty_sem.release();
    }
    count_lock.unlock();
    (gender == Gender::male ? male_sem : female_sem).release();
    std::cout << s;
}
