#include <iostream>
#include "plugins/IPlugin.h"

class ExamplePlugin : public IPlugin {
public:
	ExamplePlugin() = default;
	~ExamplePlugin() override = default;

	bool process(DataView data) override {
		std::cout << "ExamplePlugin processing data of size: " << data.size << std::endl;
		return true;
	}
};

REGISTER_PLUGIN(ExamplePlugin, "ExamplePlugin");
