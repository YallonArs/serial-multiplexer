#include "PluginLoader.h"

#include <filesystem>

#include <dlfcn.h>
#include <functional>

#include "IPlugin.h"
#include "logger/Logger.h"
#include "utils.h"

using GetPluginApiTableFunc = const CPluginApi *(*)();

const Logger PluginLoader::logger("PluginLoader");
vector<LoadedPlugin> PluginLoader::loaded_plugins;

void PluginLoader::validate_methods(const CPluginApi *api) {
	if (api == nullptr) {
		throw std::runtime_error("Plugin API is null");
	}

	if (api->methods.create == nullptr) {
		logger.error(std::format("Plugin {} has no create method", api->descriptor.name));
		throw std::runtime_error("Plugin " + string(api->descriptor.name) + " has no create method");
	}

	if (api->methods.destroy == nullptr) {
		logger.error(std::format("Plugin {} has no destroy method", api->descriptor.name));
		throw std::runtime_error("Plugin " + string(api->descriptor.name) + " has no destroy method");
	}
}

vector<PluginWrapper> PluginLoader::load_plugins(const string &directory) {
	logger.info(std::format("Loading plugins from directory: {}", directory));

	vector<PluginWrapper> plugins;

	for (const auto &entry : std::filesystem::directory_iterator(directory)) {
		if (entry.is_regular_file() && entry.path().extension() == ".so") {
			try {
				plugins.push_back(load_plugin(entry.path()));
			} catch (const std::exception &e) {
				continue;
			}
		}
	}

	return plugins;
}

PluginWrapper PluginLoader::load_plugin(const string &path) {
	void *library = dlopen(
		path.c_str(),
		RTLD_NOW | RTLD_LOCAL
	);
	logger.debug(std::format("Loading plugin: {}", path));

	if (!library) {
		logger.error(std::format("Failed to load plugin {}: {}", path, dlerror()));
		throw std::runtime_error("Failed to load plugin: " + path);
	}

	auto get_plugin = reinterpret_cast<GetPluginApiTableFunc>(
		dlsym(library, "get_api_table")
	);

	if (!get_plugin) {
		logger.error(std::format("Failed to get plugin \"{}\" function: {}", path, dlerror()));
		dlclose(library);
		throw std::runtime_error("Failed to get plugin \"" + path + "\" function: " + path);
	}

	const CPluginApi *api = get_plugin();

	if (api == nullptr) {
		dlclose(library);
		throw std::runtime_error("Failed to get plugin API: " + path);
	}

	if (api->descriptor.abi_version != PLUGIN_ABI_VERSION) {
		dlclose(library);
		throw std::runtime_error("Incompatible plugin: " + path);
	}

	validate_methods(api);

	PluginHandle handle = api->methods.create();
	
	if (handle == nullptr) {
		dlclose(library);
		logger.error(std::format("Plugin {} failed to create instance", api->descriptor.name));
		throw std::runtime_error("Plugin " + string(api->descriptor.name) + " failed to create instance");
	}

	PluginWrapper wrapper;
	wrapper.descriptor = api->descriptor;
	wrapper.capabilities = Flags(api->capabilities);

	wrapper.configure = [api, handle](const string &key, const string &value) {
		if (api->methods.configure == nullptr) {
			throw std::runtime_error("Plugin does not implement configure method");
		}
		return api->methods.configure(handle, key.c_str(), value.c_str());
	};

	wrapper.receive = [api, handle](const PluginEvent &event) {
		if (api->methods.receive == nullptr) {
			throw std::runtime_error("Plugin does not implement receive method");
		}
		api->methods.receive(handle, event);
	};

	wrapper.destroy = [api, handle]() {
		api->methods.destroy(handle);
	};

	LoadedPlugin loaded_plugin {library, wrapper, handle, [api, handle]() { api->methods.destroy(handle); }};

	logger.info(std::format("Loaded plugin: {}", api->descriptor.name));

	loaded_plugins.push_back(loaded_plugin);
	return wrapper;
}

void PluginLoader::unload_plugin(const LoadedPlugin &plugin) {
	if (plugin.handle == nullptr) {
		logger.warning(std::format("Plugin {} handle is null, cannot unload plugin", plugin.wrapper.descriptor.name));
		return;
	}

	if (plugin.wrapper.destroy == nullptr) {
		logger.warning(std::format("Plugin {} destroy method is null, cannot unload plugin", plugin.wrapper.descriptor.name));
		return;
	}

	plugin.wrapper.destroy();

	if (plugin.library != nullptr) {
		dlclose(plugin.library);
	}
}

void PluginLoader::unload_all() {
	for (const auto &plugin : loaded_plugins) {
		unload_plugin(plugin);
	}
	
	loaded_plugins.clear();
}
