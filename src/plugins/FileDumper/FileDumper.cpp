#include "plugins/IPlugin.h"
#include <fstream>

class FileDumper : public ISink, public IConfigurable {
protected:
	std::ofstream outputFile;

public:
	FileDumper(const string &name) : PluginBase(name) {
		logger.debug("FileDumper plugin created.");
		outputFile.open("output.bin", std::ios::binary | std::ios::app);
	};

	~FileDumper() override {
		logger.debug("FileDumper plugin destroyed.");
		
		if (outputFile.is_open()) {
			outputFile.close();
		}
	};

	int configure(const string &key, const string &value) override {
		logger.info("FileDumper configured with key: " + key + ", value: " + value);
		return 0;
	}

	void receive(const PluginEvent &event) override {
		logger.info("FileDumper processing event with topic: " + string(event.topic) + ", payload size: " + std::to_string(event.payload.size));
		
		if (outputFile.is_open()) {
			outputFile.write(reinterpret_cast<const char *>(event.payload.data), event.payload.size);
			outputFile.flush();
		} else {
			logger.error("Output file is not open.");
		}
	}
};

REGISTER_PLUGIN(FileDumper, (PluginBasicDescriptor{
	.name = "File Dumper",
	.id = "dev.builtin.file_dumper",
	.version = "1.0.0",
	.author = "YallonArs",
	.description = "A simple file dumper plugin."
}));
