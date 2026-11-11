#include "Broker.h"
#include "dataview.h"

const Logger Broker::logger("Broker");

void Broker::subscribe(Broker::SourceId stream, ISink *rec) {
	subscribers[stream].push_back(rec);
}

void Broker::broadcast(Broker::SourceId stream, const PluginEvent &data) {
	if (subscribers.find(stream) == subscribers.end()) {
		logger.warning(std::format("No subscribers for stream {}", stream));
	};

	for (auto rec : subscribers[stream]) {
		if (rec) rec->receive(data);
	}
}
