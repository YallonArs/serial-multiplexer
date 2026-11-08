#pragma once
#include <map>
#include <functional>
#include "IPlugin.h"
#include "logger/Logger.h"
#include "plugin_api.h"

struct PluginWrapper {
	PluginDescriptor descriptor;
	std::function<int(const string &key, const string &value)> configure;
	std::function<void(const PluginEvent &event)> receive;
	std::function<void()> destroy;

	Flags capabilities;

	bool has_capability(const PluginCapability cap) const {
		return capabilities.check(cap);
	}

	bool has_capability(std::initializer_list<const PluginCapability> caps) const {
		return capabilities.check(caps);
	}
};


struct LoadedPlugin {
	void *library;
	PluginWrapper wrapper;
	PluginHandle handle;
	std::function<void()> destroy;
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
	static PluginWrapper load_plugin(const string &path);
	static vector<PluginWrapper> load_plugins(const string &directory);
	static void unload_all();
};
