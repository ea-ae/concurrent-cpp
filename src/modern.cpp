#include "modern.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace Modern;


Modern::Container::Container(size_t size) : size(size), data(std::make_unique<int32_t[]>(size)) {}

Modern::Container::~Container() {}

void Modern::swap(Container& first, Container& second) {
	std::swap(first.data, second.data);
}

Modern::Container::Container(const Container& other) : size(other.size), data(std::make_unique<int32_t[]>(other.size)) {
	std::copy(other.data.get(), other.data.get() + size, data.get());
}

Modern::Container::Container(Container&& other) noexcept : size(other.size) {
	swap(*this, other);
}

Container& Modern::Container::operator=(Container other) {
	swap(*this, other);
	return *this;
}

int32_t& Modern::Container::operator[](int32_t i) {
	return data[i];
}


void Modern::move_semantics() {
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

void Modern::modern() {
	move_semantics();
}
