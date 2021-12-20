#pragma once

#include <memory>
#include <stdint.h>

class Foo {
public:
	int work(int32_t id, std::shared_ptr<int32_t> input);
};

void try_threads();
