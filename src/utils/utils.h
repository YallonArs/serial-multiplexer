#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "dataview.h"

using std::string;
using std::vector;
typedef uint8_t byte;

class Flags {
private:
	uint32_t storage;
	inline static uint32_t get_mask(const uint8_t index);

public:
	Flags() = default;
	~Flags() = default;

	void set(const uint8_t index);
	void unset(const uint8_t index);
	void toogle(const uint8_t index);
	bool check(const uint8_t index);
};
