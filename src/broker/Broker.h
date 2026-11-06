#pragma once

#include <map>

#include "abc/Stream.h"
#include "abc/Receiver.h"
#include "utils.h"

class Broker {
private:
	std::map<const Stream *, vector<const Receiver *>> subscribers;

public:
	Broker() = default;
	~Broker() = default;

	void subscribe(const Stream &stream, const Receiver &rec);
	void broadcast(const Stream &stream, const DataView &data);
};
