#include <iostream>
#include "plugins/IPlugin.h"

class ExamplePlugin : public IPlugin {
public:
	ExamplePlugin() = default;
	~ExamplePlugin() override = default;

	void receive(const DataView &data) const noexcept {
		std::cout << "ExamplePlugin processing data of size: " << data.size << std::endl;
	}
};

REGISTER_PLUGIN(ExamplePlugin, "ExamplePlugin");
