#include "Broker.h"
#include "dataview.h"

template <typename T>
void Broker<T>::subscribe(Broker::SourceId stream, ISink<T> &rec) {
	subscribers[stream].push_back(&rec);
}

template <typename T>
void Broker<T>::broadcast(Broker::SourceId stream, const T &data) {
	if (subscribers.find(stream) == subscribers.end()) return;

	for (auto rec : subscribers[stream]) {
		if (rec) rec->receive(data);
	}
}

template class Broker<DataView>;
