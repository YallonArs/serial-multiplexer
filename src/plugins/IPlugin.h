#pragma once

#include <concepts>
#include <functional>

#include "plugin_api.h"
#include "utils.h"

class IConfigurable {
public:
	virtual ~IConfigurable()									  = default;
	virtual int configure(const string &key, const string &value) = 0;
};

class ISink {
public:
	virtual ~ISink()							   = default;
	virtual void receive(const PluginEvent &event) = 0;
};

class ISource {
public:
	using Callback = std::function<void(const PluginEvent &event)>;

	virtual ~ISource() = default;
	void set_callback(Callback cb) {
		callback = cb;
	}

protected:
	void emit(const PluginEvent &event) {
		if (callback) callback(event);
	}

private:
	Callback callback = nullptr;
};

template <typename T>
concept PluginC = std::derived_from<T, ISink> || std::derived_from<T, ISource> || std::derived_from<T, IConfigurable>;

enum class PluginCapability : uint8_t {
	None		 = 0,
	Configurable = 1,
	Sink		 = 2,
	Source		 = 3
};

template <PluginC T>
static const CPluginApi *get_api_table(c_string name) noexcept {
	Flags capabilities = 0;
	Methods methods {};
	if constexpr (std::derived_from<T, IConfigurable>) {
		capabilities.set(PluginCapability::Configurable);
		methods.configure = [](PluginHandle handle, c_string key, c_string value) -> decltype(auto) {
			return reinterpret_cast<T *>(handle)->configure(key, value);
		};
	}
	if constexpr (std::derived_from<T, ISink>) {
		capabilities.set(PluginCapability::Sink);
		methods.receive = [](PluginHandle handle, const PluginEvent &data) -> decltype(auto) {
			reinterpret_cast<T *>(handle)->receive(data);
		};
	}
	if constexpr (std::derived_from<T, ISource>) {
		capabilities.set(PluginCapability::Source);
		methods.set_callback = [](PluginHandle handle, SourceCap::Callback cb) -> decltype(auto) {
			reinterpret_cast<T *>(handle)->set_callback(cb);
		};
	}

	methods.create	= []() -> PluginHandle { return reinterpret_cast<PluginHandle>(new T()); };
	methods.destroy = [](PluginHandle handle) { delete reinterpret_cast<T *>(handle); };

	const CPluginApi *api = new CPluginApi {
		.descriptor	  = {.abi_version = PLUGIN_ABI_VERSION,
						 .name		  = name,
						 .id		  = typeid(T).name()},
		.capabilities = capabilities,
		.methods	  = methods};

	return api;
}

#define REGISTER_PLUGIN(T, name) \
	extern "C" const CPluginApi *get_api_table() noexcept { return get_api_table<T>(name); }
