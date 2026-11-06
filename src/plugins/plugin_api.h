#pragma once

#include "dataview.h"

extern "C" {
#define PLUGIN_ABI_VERSION 2

typedef struct PluginHandle_t *PluginHandle;

struct PluginDescriptor {
	uint32_t abi_version;
	const char *name;
	const char *id;
};

struct PluginEvent {
	const char *topic; // тип или тема события, например "protocol.frame" или "tg.alert"
	DataView payload;
};

#define CAP_CONFIGURABLE "configurable"
struct ConfigurableCap {
	int (*configure)(PluginHandle handle, const char *key, const char *value);
};

struct CPluginApi {
	PluginDescriptor descriptor;
	PluginHandle (*create)();
	void (*destroy)(PluginHandle handle);
	const void *(*query_interface)(PluginHandle handle, const char *interface_id);
};

typedef const CPluginApi *(*GetPluginApiFunc)();
}

template <typename Data>
struct SinkCap {
	void (*receive)(PluginHandle handle, const Data &data);
};

template <typename Data>
struct SourceCap {
	using Callback = void (*)(void *host_ctx, Data data);
	void (*set_callback)(PluginHandle handle, Callback cb, void *host_ctx);
};

using ByteSinkCap	 = SinkCap<DataView>;
using EventSinkCap	 = SinkCap<PluginEvent>;
using ByteSourceCap	 = SourceCap<DataView>;
using EventSourceCap = SourceCap<PluginEvent>;

inline constexpr const char *CAP_BYTE_SINK	  = "byte_sink";
inline constexpr const char *CAP_EVENT_SINK	  = "event_sink";
inline constexpr const char *CAP_BYTE_SOURCE  = "byte_source";
inline constexpr const char *CAP_EVENT_SOURCE = "event_source";
