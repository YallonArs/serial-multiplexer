#pragma once
#include "utils.h"
#include "plugin_api.h"
#include "logger/Logger.h"
#include "abc/Receiver.h"

class PluginReceiverAdapter final : public Receiver {
private:
	const CPluginApi *api;
	PluginHandle handle;

public:
	PluginReceiverAdapter(const CPluginApi *api, PluginHandle handle) : api(api), handle(handle) {}

	void receive(const DataView &data) const noexcept override {
		if (api != nullptr && handle != nullptr && api->receive != nullptr) {
			api->receive(handle, data);
		}
	}
};

struct LoadedPlugin {
	void *library;
	const CPluginApi *api;
	PluginHandle handle;
	PluginReceiverAdapter adapter;
};

class PluginLoader {
private:
	PluginLoader() = delete;
	~PluginLoader() = delete;

	static const Logger logger;
public:
	static LoadedPlugin load_plugin(const string path);
	static void unload_plugin(const LoadedPlugin &plugin);
};
