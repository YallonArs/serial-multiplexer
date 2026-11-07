#pragma once
#include <map>
#include "logger/Logger.h"
#include "plugin_api.h"

struct LoadedPlugin {
	void *library;
	const CPluginApi *api;
	PluginHandle handle;
};

class PluginLoader {
private:
	PluginLoader()	= delete;
	~PluginLoader() = delete;

	static const Logger logger;

	static vector<LoadedPlugin> loaded_plugins;
	static void validate_methods(const CPluginApi *api);
	static void validate_capabilities(const CPluginApi *api);
	static void unload_plugin(const LoadedPlugin &plugin);

public:
	static LoadedPlugin load_plugin(const string &path);
	static void load_plugins(const string &directory);
	static void unload_all();
};
