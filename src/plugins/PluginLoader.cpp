#include "PluginLoader.h"

#include <dlfcn.h>
#include <logger/logger.h>

#include "utils.h"

const Logger PluginLoader::logger("PluginLoader");
vector<CPluginApi> PluginLoader::plugins_loaded;

using GetPluginFn = const CPluginApi *(*)(
	uint32_t
);

void PluginLoader::load_plugin(const string path) {
	void *library = dlopen(
		path.c_str(),
		RTLD_NOW | RTLD_LOCAL
	);
	logger.debug(std::format("Loading plugin: {}", path));

	if (!library) {
		logger.error(std::format("Failed to load plugin {}: {}", path, dlerror()));
		throw std::runtime_error("Failed to load plugin: " + path);
	}

	auto get_plugin = reinterpret_cast<GetPluginFn>(
		dlsym(library, "get_plugin_api")
	);

	if (!get_plugin) {
		logger.error(std::format("Failed to get plugin \"{}\" function: {}", path, dlerror()));
		dlclose(library);
		throw std::runtime_error("Failed to get plugin \"" + path + "\" function: " + path);
	}

	const CPluginApi *plugin = get_plugin(PLUGIN_ABI_VERSION);

	if (plugin == nullptr || plugin->descriptor.abi_version != PLUGIN_ABI_VERSION) {
		dlclose(library);
		throw std::runtime_error("Incompatible plugin: " + path);
	}

	logger.info(std::format("Loaded plugin: {}", plugin->descriptor.name));

	const byte* data = reinterpret_cast<const byte*>(plugin->descriptor.name);
	PluginHandle handle = plugin->create();
	plugin->process_data(handle, {data, 67});
	
	plugins_loaded.push_back(*plugin);
}
