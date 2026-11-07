#pragma once

#include <unordered_map>

#include "plugins/IPlugin.h"

class Broker {
public:
	using SourceId = const void *;

	Broker()  = default;
	~Broker() = default;

	void subscribe(SourceId stream, ISink *rec);
	void broadcast(SourceId stream, const PluginEvent &data);

private:
	std::unordered_map<SourceId, vector<ISink *>> subscribers;
};
