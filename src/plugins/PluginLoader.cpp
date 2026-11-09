#include "PluginLoader.h"

#include <filesystem>

#include <dlfcn.h>
#include <functional>

#include "IPlugin.h"
#include "logger/Logger.h"
#include "utils.h"

using GetPluginApiTableFunc = const CPluginApi *(*)();

const Logger PluginLoader::logger("PluginLoader");
std::map<string, LoadedPlugin> PluginLoader::loaded_plugins;

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
		throw std::runtime_error(std::format("Failed to load plugin {}: {}", path, dlerror()));
	}

	auto get_plugin = reinterpret_cast<GetPluginApiTableFunc>(
		dlsym(library, "get_api_table")
	);

	if (!get_plugin) {
		logger.error(std::format("Failed to find API table for plugin \"{}\": {}", path, dlerror()));
		dlclose(library);
		throw std::runtime_error(std::format("Failed to find API table for plugin \"{}\": {}", path, dlerror()));
	}

	const CPluginApi *api = get_plugin();

	if (api == nullptr) {
		dlclose(library);
		throw std::runtime_error("Failed to get plugin API: " + path);
	}

	if (api->descriptor.abi_version != PLUGIN_ABI_VERSION) {
		logger.error(std::format("Incompatible plugin: {}. Required ABI version: {}, got: {}", path, PLUGIN_ABI_VERSION, api->descriptor.abi_version));
		dlclose(library);
		throw std::runtime_error(std::format("Incompatible plugin: {}. Required ABI version: {}, got: {}", path, PLUGIN_ABI_VERSION, api->descriptor.abi_version));
	}

	if (api->capabilities == 0) {
		logger.warning(std::format("Plugin {} ({} - {}) has no capabilities", api->descriptor.name, api->descriptor.id, api->descriptor.version));
	}

	if (loaded_plugins.find(api->descriptor.id) != loaded_plugins.end()) {
		logger.warning(std::format("Plugin {} ({} - {}) is already loaded ({})", api->descriptor.name, api->descriptor.id, api->descriptor.version, loaded_plugins.find(api->descriptor.id)->second.wrapper.descriptor.version));
		dlclose(library);
		throw std::runtime_error("Plugin " + string(api->descriptor.name) + " is already loaded");
	}

	validate_methods(api);

	PluginHandle handle = api->methods.create(api->descriptor.name);
	
	if (handle == nullptr) {
		dlclose(library);
		logger.error(std::format("Plugin {} failed to create instance", api->descriptor.id));
		throw std::runtime_error("Plugin " + string(api->descriptor.id) + " failed to create instance");
	}

	PluginWrapper wrapper;
	wrapper.descriptor = api->descriptor;
	wrapper.capabilities = Flags(api->capabilities);

	wrapper.configure = [api, handle](const string &key, const string &value) {
		if (api->methods.configure == nullptr) {
			throw std::runtime_error(std::format("Plugin {} does not implement configure method", api->descriptor.id));
		}
		return api->methods.configure(handle, key.c_str(), value.c_str());
	};

	wrapper.receive = [api, handle](const PluginEvent &event) {
		if (api->methods.receive == nullptr) {
			throw std::runtime_error(std::format("Plugin {} does not implement receive method", api->descriptor.id));
		}
		api->methods.receive(handle, event);
	};

	wrapper.destroy = [api, handle]() {
		api->methods.destroy(handle);
	};

	LoadedPlugin loaded_plugin {library, wrapper, handle, [api, handle]() { api->methods.destroy(handle); }};

	logger.info(std::format("Loaded plugin: {} ({} - {})", api->descriptor.name, api->descriptor.id, api->descriptor.version));

	loaded_plugins.insert({loaded_plugin.wrapper.descriptor.id, loaded_plugin});
	return wrapper;
}

void PluginLoader::unload_plugin(const LoadedPlugin &plugin) {
	if (plugin.handle == nullptr) {
		logger.warning(std::format("Plugin {} handle is null, cannot unload plugin", plugin.wrapper.descriptor.id));
		return;
	}

	if (plugin.wrapper.destroy == nullptr) {
		logger.warning(std::format("Plugin {} destroy method is null, cannot unload plugin", plugin.wrapper.descriptor.id));
		return;
	}

	plugin.wrapper.destroy();

	if (plugin.library != nullptr) {
		dlclose(plugin.library);
	}
}

void PluginLoader::unload_all() {
	for (const auto &[_, plugin] : loaded_plugins) {
		unload_plugin(plugin);
	}
	
	loaded_plugins.clear();
}
