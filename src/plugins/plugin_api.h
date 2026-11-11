#pragma once

#include <stdint.h>

#include "dataview.h"
#include "utils.h"

extern "C" {
#define PLUGIN_ABI_VERSION 3

struct PluginEvent {
	c_string topic; // тип или тема события, например "protocol.frame" или "tg.alert"
	DataView payload;
};

struct PluginBasicDescriptor {
	c_string name;
	c_string id;
	c_string version;
	c_string author;
	c_string description;
};

struct PluginDescriptor : public PluginBasicDescriptor {
	uint32_t abi_version;
};

typedef struct PluginHandle_t *PluginHandle;

struct ConfigurableCap {
	int (*configure)(PluginHandle handle, c_string key, c_string value);
};

struct SinkCap {
	void (*receive)(PluginHandle handle, const PluginEvent &data);
};

struct SourceCap {
	using Callback = void (*)(const PluginEvent &data);
	void (*set_callback)(PluginHandle handle, Callback cb);
};

struct CPluginMethods {
	PluginHandle (*create)(c_string name);
	void (*destroy)(PluginHandle handle);
	
	int (*configure)(PluginHandle handle, c_string key, c_string value);
	void (*receive)(PluginHandle handle, const PluginEvent &data);
	void (*set_callback)(PluginHandle handle, SourceCap::Callback cb);
};

struct CPluginApi {
	PluginDescriptor descriptor;
	uint32_t capabilities;
	CPluginMethods methods;
};

}
