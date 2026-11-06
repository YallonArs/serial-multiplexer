#include "dataview.h"

extern "C" {
#define PLUGIN_ABI_VERSION 1

typedef struct PluginHandle_t *PluginHandle;

struct PluginDescriptor {
	uint32_t abi_version;
	const char *name;
	const char *id;
};

struct CPluginApi {
	PluginDescriptor descriptor;

	PluginHandle (*create)();
	void (*destroy)(PluginHandle handle);
	int (*receive)(PluginHandle handle, const DataView data);
};

typedef const CPluginApi *(*GetPluginApiFunc)();
}
