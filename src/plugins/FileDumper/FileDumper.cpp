#include <iostream>
#include "plugins/IPlugin.h"
#include "dataview.h"

class FileDumper : public IByteSink {
public:
	FileDumper() = default;
	~FileDumper() override = default;

	void receive(const DataView &data) override {
		std::cout << "FileDumper processing data of size: " << data.size << std::endl;
	}
};

REGISTER_PLUGIN(FileDumper, "FileDumper");
