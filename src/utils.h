#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "dataview.h"

using std::string;
using std::vector;
using byte	   = uint8_t;
using c_string = const char *;

template <typename T>
concept IntegralConverible = std::convertible_to<T, uint8_t> || std::is_integral_v<T> || (std::is_enum_v<T> && std::is_integral_v<std::underlying_type_t<T>>);

class Flags {
private:
	uint32_t storage;
	static uint32_t get_mask(const uint8_t index) { return 1u << index; }

public:
	Flags() = default;
	Flags(const uint32_t value) : storage(value) {}
	~Flags() = default;

	Flags &operator=(const uint32_t value) {
		storage = value;
		return *this;
	}
	operator uint32_t() const { return storage; }

	template <IntegralConverible T>
	void set(const T index) { storage |= get_mask(static_cast<uint8_t>(index)); }

	template <IntegralConverible T>
	void unset(const T index) { storage &= ~get_mask(static_cast<uint8_t>(index)); }

	template <IntegralConverible T>
	void toogle(const T index) { storage ^= get_mask(static_cast<uint8_t>(index)); }

	template <IntegralConverible T>
	bool check(const T index) const { return (storage & get_mask(static_cast<uint8_t>(index))) == get_mask(static_cast<uint8_t>(index)); }

	template <IntegralConverible T>
	bool check(std::initializer_list<const T> indices) const {
		for (const auto &index : indices) {
			if (!check(index)) return false;
		}
		return true;
	}
};
