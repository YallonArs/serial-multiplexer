#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "dataview.h"

using std::string;
using std::vector;
using byte = uint8_t;
using c_string = const char *;

class Flags {
private:
	uint32_t storage;
	static uint32_t get_mask(const uint8_t index) { return 1u << index; }

public:
	Flags() = default;
	Flags(const uint32_t value) : storage(value) {}
	~Flags() = default;

	Flags &operator=(const uint32_t value) { storage = value; return *this; }
	operator uint32_t() const { return storage; }

	void set(const uint8_t index)    { storage |=  get_mask(index); }
	void unset(const uint8_t index)  { storage &= ~get_mask(index); }
	void toogle(const uint8_t index) { storage ^=  get_mask(index); }
	bool check(const uint8_t index)  { return (storage & get_mask(index)) == get_mask(index); }
};
