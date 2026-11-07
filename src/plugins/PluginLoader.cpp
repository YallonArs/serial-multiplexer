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

	validate_capabilities(api);
}

void PluginLoader::validate_capabilities(const CPluginApi *api) {
	if (api == nullptr) {
		throw std::runtime_error("Plugin API is null");
	}

	if (api->capabilities == 0) {
		logger.error(std::format("Plugin {} has no capabilities", api->descriptor.name));
		throw std::runtime_error("Plugin " + string(api->descriptor.name) + " has no capabilities");
	}

	const std::map<PluginCapability, void*> capabilities_map = {
		{PluginCapability::None, nullptr},
		{PluginCapability::Configurable, reinterpret_cast<void*>(api->methods.configure)},
		{PluginCapability::Sink, reinterpret_cast<void*>(api->methods.receive)},
		{PluginCapability::Source, reinterpret_cast<void*>(api->methods.set_callback)}
	};
	
	for (const auto &[capability, method] : capabilities_map) {
		if (Flags(api->capabilities).check(static_cast<uint8_t>(capability))) {
			if (method == nullptr) {
				logger.error(std::format("Plugin {} has capability {} but does not implement the required method", api->descriptor.name, static_cast<uint8_t>(capability)));
				throw std::runtime_error("Plugin " + string(api->descriptor.name) + " has capability " + std::to_string(static_cast<uint8_t>(capability)) + " but does not implement the required method");
			}
		}
	}
}

void PluginLoader::load_plugins(const string &directory) {
	logger.info(std::format("Loading plugins from directory: {}", directory));

	for (const auto &entry : std::filesystem::directory_iterator(directory)) {
		if (entry.is_regular_file() && entry.path().extension() == ".so") {
			try {
				LoadedPlugin plugin = load_plugin(entry.path());
				loaded_plugins.push_back(plugin);
			} catch (const std::exception &e) {
				continue;
			}
		}
	}
}

LoadedPlugin PluginLoader::load_plugin(const string &path) {
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

	const CPluginApi *plugin = get_plugin();

	if (plugin == nullptr || plugin->descriptor.abi_version != PLUGIN_ABI_VERSION) {
		dlclose(library);
		throw std::runtime_error("Incompatible plugin: " + path);
	}

	validate_methods(plugin);

	PluginHandle handle = nullptr;
	handle = plugin->methods.create();

	if (handle == nullptr) {
		dlclose(library);
		throw std::runtime_error("Failed to create plugin instance: " + path);
	}

	LoadedPlugin loaded_plugin {library, plugin, handle};

	logger.info(std::format("Loaded plugin: {}", plugin->descriptor.name));

	loaded_plugins.push_back(loaded_plugin);
	return loaded_plugin;
}

void PluginLoader::unload_plugin(const LoadedPlugin &plugin) {
	if (plugin.api == nullptr) {
		logger.warning("Plugin API is null, cannot unload plugin");
		return;
	}

	if (plugin.handle == nullptr) {
		logger.warning(std::format("Plugin {} handle is null, cannot unload plugin", plugin.api->descriptor.name));
		return;
	}

	if (plugin.api->methods.destroy == nullptr) {
		logger.warning(std::format("Plugin {} destroy method is null, cannot unload plugin", plugin.api->descriptor.name));
		return;
	}

	plugin.api->methods.destroy(plugin.handle);

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
