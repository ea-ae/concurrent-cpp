#pragma once

#include <queue>
#include <stdint.h>

static void work_and_add_to_stack (int32_t value);
static void produce(std::queue<int32_t> &tasks);
static void consume(std::queue<int32_t> &tasks);
void try_locks();
