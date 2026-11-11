#pragma once

#include <unordered_map>

#include "utils.h"
#include "plugins/IPlugin.h"
#include "logger/Logger.h"

class Broker {
public:
	using SourceId = const void *;

	Broker()  = default;
	~Broker() = default;

	void subscribe(SourceId stream, ISink *rec);
	void broadcast(SourceId stream, const PluginEvent &data);

private:
	static const Logger logger;
	std::unordered_map<SourceId, vector<ISink *>> subscribers;
};
