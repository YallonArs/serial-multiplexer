#include <iostream>
#include "plugins/IPlugin.h"

class FileDumper : public ISink {
public:
	FileDumper() = default;
	~FileDumper() override = default;

	void receive(const PluginEvent &event) override {
		std::cout << "FileDumper processing data of size: " << event.payload.size << std::endl;
	}
};

REGISTER_PLUGIN(FileDumper, "FileDumper")
