#include "Broker.h"

void Broker::subscribe(const Stream &stream, const Receiver &rec) {
	subscribers[&stream].push_back(&rec);
}

void Broker::broadcast(const Stream &stream, const DataView &data) {
	for (auto rec : subscribers[&stream]) {
		if (rec) rec->receive(data);
	}
}
