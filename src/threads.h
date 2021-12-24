#pragma once

#include <stdint.h>

#include <memory>

class Foo {
   public:
    int work(int32_t id, std::shared_ptr<int32_t> input);
};

void try_threads();
