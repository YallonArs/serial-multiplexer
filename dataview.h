#pragma once

extern "C" {

#include <cstddef>
#include <cstdint>
using byte = uint8_t;

struct DataView {
	const byte *data;
	const size_t size;
};

}
