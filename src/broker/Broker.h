#pragma once

#include <unordered_map>

#include "plugins/IPlugin.h"
#include "utils.h"

template <typename T>
class Broker {
public:
	using SourceId = const void *;

	Broker()  = default;
	~Broker() = default;

	void subscribe(SourceId stream, ISink<T> &rec);
	void broadcast(SourceId stream, const T &data);

private:
	std::unordered_map<SourceId, vector<ISink<T> *>> subscribers;
};
