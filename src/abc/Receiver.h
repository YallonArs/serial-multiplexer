#pragma once

#include "dataview.h"

class Receiver {
public:
	virtual ~Receiver() = default;
	virtual void receive(const DataView &data) const noexcept = 0;
};
