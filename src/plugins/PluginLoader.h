#pragma once
#include "utils.h"
#include "plugin_api.h"
#include "logger/Logger.h"

struct LoadedPlugin {
	void *library;
	const CPluginApi *api;
	PluginHandle handle;

	template <typename CapStruct>
    const CapStruct* query(const char* interface_id) const {
        if (!api || !api->query_interface) return nullptr;
        return static_cast<const CapStruct*>(api->query_interface(handle, interface_id));
    }
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
