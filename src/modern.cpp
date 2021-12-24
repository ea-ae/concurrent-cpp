#include "modern.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <iterator>
#include <iostream>


struct Container {
	Container(size_t size) : size(size), data(std::make_unique<int32_t[]>(size)) {}
	~Container() {}

	friend void swap(Container& first, Container& second) {
		std::swap(first.data, second.data);
	}

	Container(const Container& other) : size(other.size), data(std::make_unique<int32_t[]>(other.size)) {
		std::copy(other.data.get(), other.data.get() + size, data.get());
	}

	Container(Container&& other) noexcept : size(other.size) {
		swap(*this, other);
	}

	Container& operator=(Container other) {
		swap(*this, other);
		return *this;
	}
	

	int32_t& operator[](int32_t i) {
		return data[i];
	}

	std::unique_ptr<int32_t[]> data;
	const size_t size;
};


void move_semantics() {
	Container cont(100);
	cont[50] = 1234;
	Container cont2(cont);
	Container cont3 = cont;
	assert(cont[50] == cont2[50] && cont2[50] == cont3[50]);

	Container cont4(std::move(cont));
	std::cout << cont4[50] << '\n';

	cont4[0] = 4004;
	cont2 = std::move(cont4);
	std::cout << cont2[0] << '\n';
}

void modern() {
	move_semantics();
}
