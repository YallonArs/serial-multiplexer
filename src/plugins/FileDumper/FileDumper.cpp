#include "plugins/IPlugin.h"

class FileDumper : public ISink, public IConfigurable {
public:
	FileDumper(const string &name) : PluginBase(name) {
		logger.debug("FileDumper plugin created.");
	};
	~FileDumper() override {
		logger.debug("FileDumper plugin destroyed.");
	};

	int configure(const string &key, const string &value) override {
		logger.info("FileDumper configured with key: " + key + ", value: " + value);
		return 0;
	}

	void receive(const PluginEvent &event) override {
		logger.info("FileDumper received data of size: " + std::to_string(event.payload.size));
	}
};

REGISTER_PLUGIN(FileDumper, (PluginBasicDescriptor{
	.name = "File Dumper",
	.id = "dev.builtin.file_dumper",
	.version = "1.0.0",
	.author = "YallonArs",
	.description = "A simple file dumper plugin."
}));
