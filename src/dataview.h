#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
using byte = uint8_t;

struct DataView {
	const byte *data;
	const size_t size;
};
}
