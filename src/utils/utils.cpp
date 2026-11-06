#include <cassert>
#include "utils.h"

uint32_t Flags::get_mask(const uint8_t index) {
	assert(index < 32);
	return 1 << index;
}

void Flags::set(const uint8_t index) {
	storage |= get_mask(index);
}

void Flags::unset(const uint8_t index) {
	storage &= ~(get_mask(index));
}

void Flags::toogle(const uint8_t index) {
	storage ^= get_mask(index);
}

bool Flags::check(const uint8_t index) {
	uint8_t mask = get_mask(index);
	return (storage & mask) == mask;
}
