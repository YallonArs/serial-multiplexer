#pragma once
#include <functional>
#include <map>

#include "IPlugin.h"
#include "logger/Logger.h"
#include "plugin_api.h"

struct PluginWrapper : public IConfigurable, public ISink, public ISource {
	PluginDescriptor descriptor;
	PluginHandle handle;
	CPluginMethods methods;

	int configure(const string &key, const string &value) override {
		if (methods.configure) {
			return methods.configure(handle, key.c_str(), value.c_str());
		}

		throw std::runtime_error(std::format("Plugin {} does not implement configure method", descriptor.id));
	}

	void receive(const PluginEvent &event) override {
		if (!methods.receive) {
			throw std::runtime_error(std::format("Plugin {} does not implement receive method", descriptor.id));
		}
		
		methods.receive(handle, event);
	}

	void destroy() {
		if (!methods.destroy) {
			throw std::runtime_error(std::format("Plugin {} does not implement destroy method", descriptor.id));
		}
		methods.destroy(handle);
	}

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

	static std::map<string, LoadedPlugin> loaded_plugins;
	static void validate_methods(const CPluginApi *api);
	static void unload_plugin(LoadedPlugin &plugin);

public:
	static PluginWrapper load_plugin(const string &path);
	static vector<PluginWrapper> load_plugins(const string &directory);
	static void unload_all();
};
