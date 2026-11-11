#include "plugins/IPlugin.h"
#include <fstream>

class Printer : public ISink {
public:
	Printer(const string &name) : PluginBase(name) {
		logger.debug("Printer plugin created.");
	};

	~Printer() override {
		logger.debug("Printer plugin destroyed.");
	};

	void receive(const PluginEvent &event) override {
		// construct hex payload string
		string hexPayload;
		for (size_t i = 0; i < event.payload.size; ++i) {
			hexPayload += std::format("{:02X} ", event.payload.data[i]);
		}
		logger.info("Printer processing event with topic: " + string(event.topic) + ", payload size: " + std::to_string(event.payload.size) + ", data: " + hexPayload);
	}
};

REGISTER_PLUGIN(Printer, (PluginBasicDescriptor{
	.name = "Printer",
	.id = "dev.builtin.printer",
	.version = "1.0.0",
	.author = "YallonArs",
	.description = "A simple printer plugin."
}));
