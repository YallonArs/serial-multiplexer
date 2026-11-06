#pragma once

#ifdef __cplusplus
#include <exception>
#include <string_view>

#include "abc/Stream.h"
#include "abc/Receiver.h"
#include "dataview.h"
#include "plugin_api.h"

class IPlugin : public Receiver {
public:
	virtual ~IPlugin() = default;

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
			.destroy = [](PluginHandle h) noexcept { delete reinterpret_cast<IPlugin *>(h); },
			.receive = [](PluginHandle h, const DataView data) noexcept -> int {
				try {
					auto *plugin = reinterpret_cast<IPlugin *>(h);
					plugin->receive(data);
					return 0;
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
