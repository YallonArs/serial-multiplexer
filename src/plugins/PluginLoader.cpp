#include "PluginLoader.h"

#include <dlfcn.h>

#include "logger/Logger.h"
#include "utils.h"

const Logger PluginLoader::logger("PluginLoader");
LoadedPlugin PluginLoader::load_plugin(const string path) {
	void *library = dlopen(
		path.c_str(),
		RTLD_NOW | RTLD_LOCAL
	);
	logger.debug(std::format("Loading plugin: {}", path));

	if (!library) {
		logger.error(std::format("Failed to load plugin {}: {}", path, dlerror()));
		throw std::runtime_error("Failed to load plugin: " + path);
	}

	auto get_plugin = reinterpret_cast<GetPluginApiFunc>(
		dlsym(library, "get_plugin_api")
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

	PluginHandle handle = nullptr;
	if (plugin->create != nullptr) {
		handle = plugin->create();
	}

	if (handle == nullptr) {
		dlclose(library);
		throw std::runtime_error("Failed to create plugin instance: " + path);
	}

	logger.info(std::format("Loaded plugin: {}", plugin->descriptor.name));
	return LoadedPlugin{library, plugin, handle};
}


void PluginLoader::unload_plugin(const LoadedPlugin &plugin) {
	if (plugin.api != nullptr && plugin.handle != nullptr && plugin.api->destroy != nullptr) {
		plugin.api->destroy(plugin.handle);
	}

	if (plugin.library != nullptr) {
		dlclose(plugin.library);
	}
}
