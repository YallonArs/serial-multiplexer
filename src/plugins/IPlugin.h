#ifdef __cplusplus
#include <exception>
#include <string_view>

#include "dataview.h"
#include "plugin_api.h"

class IPlugin {
public:
	virtual ~IPlugin()					= default;
	virtual bool process(DataView data) = 0;

	template <typename PluginT>
		requires std::derived_from<PluginT, IPlugin>
	static const CPluginApi *get_api_table(const char *name) noexcept {
		static const CPluginApi api = {
			.descriptor = {.abi_version = PLUGIN_ABI_VERSION,
						   .name		= name,
						   .id			= typeid(PluginT).name()},
			.create		= []() noexcept -> PluginHandle {
				try {
					return reinterpret_cast<PluginHandle>(new PluginT());
				} catch (...) {
					return nullptr;
				}
			},
			.destroy	  = [](PluginHandle h) noexcept { delete reinterpret_cast<IPlugin *>(h); },
			.process_data = [](PluginHandle h, const DataView data) noexcept -> int {
				try {
					auto *plugin = reinterpret_cast<IPlugin *>(h);
					return plugin->process(data) ? 0 : -1;
				} catch (...) {
					return -2;
				}
			}};
		return &api;
	}
};

#define REGISTER_PLUGIN(PluginClass, PluginName)                                           \
	extern "C" __attribute__((visibility("default"))) const CPluginApi *get_plugin_api() { \
		return IPlugin::get_api_table<PluginClass>(PluginName);                            \
	}
#endif
