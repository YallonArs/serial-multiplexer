#include <iostream>
#include "plugins/IPlugin.h"

class FileDumper : public ISink, public IConfigurable {
public:
	FileDumper() {
		std::cout << "FileDumper plugin created." << std::endl;
	};
	~FileDumper() override {
		std::cout << "FileDumper plugin destroyed." << std::endl;
	};

	int configure(const string &key, const string &value) override {
		std::cout << "FileDumper configured with key: " << key << ", value: " << value << std::endl;
		return 0;
	}

	void receive(const PluginEvent &event) override {
		std::cout << "FileDumper processing data of size: " << event.payload.size << std::endl;
	}
};

REGISTER_PLUGIN(FileDumper, "FileDumper")
