#pragma once
#include "utils.h"
#include "plugin_api.h"
#include "logger/logger.h"

class PluginLoader {
private:
	PluginLoader() = delete;
	~PluginLoader() = delete;

	static const Logger logger;
	static vector<CPluginApi> plugins_loaded;
public:
	static void load_plugin(const string path);
	// void unload_plugin(const string path);
};
