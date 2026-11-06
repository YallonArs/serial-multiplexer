#pragma once

#ifdef __cplusplus
#include <exception>
#include <string_view>

#include "dataview.h"
#include "plugin_api.h"
#include "utils.h"

class IConfigurable {
public:
	virtual ~IConfigurable()									  = default;
	virtual int configure(const string &key, const string &value) = 0;
};

template <typename T>
class ISink {
public:
	virtual ~ISink()					= default;
	virtual void receive(const T &data) = 0;
};

template <typename T>
class ISource {
public:
	using Callback = void (*)(void *host_ctx, T data);

	virtual ~ISource() = default;
	void set_output_callback(Callback cb, void *ctx) {
		cb_	 = cb;
		ctx_ = ctx;
	}

protected:
	void emit(T data) {
		if (cb_) cb_(ctx_, data);
	}

private:
	Callback cb_ = nullptr;
	void *ctx_	 = nullptr;
};

using IByteSink	   = ISink<DataView>;
using IByteSource  = ISource<DataView>;
using IEventSink   = ISink<PluginEvent>;
using IEventSource = ISource<PluginEvent>;

template <typename T>
concept AnySink = std::derived_from<T, IByteSink> || std::derived_from<T, IEventSink>;

template <typename T>
concept AnySource = std::derived_from<T, IByteSource> || std::derived_from<T, IEventSource>;

template <typename T>
concept Plugin = AnySink<T> || AnySource<T> || std::derived_from<T, IConfigurable>;

namespace detail {

	template <typename T, typename Data>
	const void *try_query_sink(std::string_view requested_id, std::string_view cap_id) {
		if constexpr (std::derived_from<T, ISink<Data>>) {
			if (requested_id == cap_id) {
				static constexpr SinkCap<Data> cap = {
				.receive = [](PluginHandle h, const Data &d) -> void {
						static_cast<ISink<Data> *>(reinterpret_cast<T *>(h))->receive(d);
					}};
				return &cap;
			}
		}
		return nullptr;
	}

	template <typename T, typename Data>
	const void *try_query_source(std::string_view requested_id, std::string_view cap_id) {
		if constexpr (std::derived_from<T, ISource<Data>>) {
			if (requested_id == cap_id) {
				static constexpr SourceCap<Data> cap = {
					.set_callback = [](PluginHandle h, typename SourceCap<Data>::Callback cb, void *ctx) {
						static_cast<ISource<Data> *>(reinterpret_cast<T *>(h))->set_output_callback(cb, ctx);
					}};
				return &cap;
			}
		}
		return nullptr;
	}

	template <typename T>
	const void *query_interface_impl(PluginHandle handle, const char *iid) {
		if (!handle || !iid) return nullptr;
		std::string_view id(iid);

		// все поддерживаемые виды ISink<Data>
		if (auto *p = try_query_sink<T, DataView>(id, CAP_BYTE_SINK)) return p;
		if (auto *p = try_query_sink<T, PluginEvent>(id, CAP_EVENT_SINK)) return p;

		// все поддерживаемые виды ISource<Data>
		if (auto *p = try_query_source<T, DataView>(id, CAP_BYTE_SOURCE)) return p;
		if (auto *p = try_query_source<T, PluginEvent>(id, CAP_EVENT_SOURCE)) return p;

		if constexpr (std::derived_from<T, IConfigurable>) {
			if (id == CAP_CONFIGURABLE) {
				static constexpr ConfigurableCap cap = {
					.configure = [](PluginHandle h, const char *k, const char *v) -> int {
						return reinterpret_cast<T *>(h)->configure(k, v) ? 0 : -1;
					}};
				return &cap;
			}
		}

		return nullptr;
	}

} // namespace detail

template <Plugin PluginT>
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
		.destroy		 = [](PluginHandle h) noexcept { delete reinterpret_cast<PluginT *>(h); },
		.query_interface = &detail::query_interface_impl<PluginT>};
	return &api;
}

#define REGISTER_PLUGIN(PluginClass, PluginName)                                           \
	extern "C" __attribute__((visibility("default"))) const CPluginApi *get_plugin_api() { \
		return get_api_table<PluginClass>(PluginName);                                     \
	}
#endif
