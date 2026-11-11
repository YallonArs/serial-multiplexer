#include "broker/Broker.h"
#include "hal/SerialPort.h"
#include "plugins/PluginLoader.h"
#include "logger/Logger.h"

int main() {
	Logger logger("Main");
	logger.info("Starting serial multiplexer...");

	SerialPort serialPort({"/dev/ttyUSB1", 9600});
	serialPort.setMode(SerialPort::Mode::Binary);
	serialPort.open();

	Broker broker;

	PluginWrapper dumper, printer;
	try {
		dumper = PluginLoader::load_plugin("./plugins/FileDumper.so");
		assert(dumper.has_capability({PluginCapability::Sink, PluginCapability::Configurable}));
	
		printer = PluginLoader::load_plugin("./plugins/Printer.so");
		assert(printer.has_capability(PluginCapability::Sink));
	} catch (std::runtime_error &e) {
		std::cerr << "Error loading plugins: " << e.what() << std::endl;
		return 1;
	}

	broker.subscribe(&serialPort, static_cast<ISink *>(&dumper));
	broker.subscribe(&serialPort, static_cast<ISink *>(&printer));

	while (true) {
		byte buffer[2]	 = {};
		ssize_t bytesRead = serialPort.read(buffer, 2);

		if (bytesRead > 0) {
			broker.broadcast(&serialPort, PluginEvent {.topic = "serial.data", .payload = DataView(buffer, bytesRead)});
		}
	}

	// if (dumper.has_capability({PluginCapability::Sink, PluginCapability::Configurable}) ) {
	dumper.configure("output_file", "output.txt");
	dumper.receive(PluginEvent {.topic = "serial.data", .payload = DataView(nullptr, 67)});
	PluginLoader::unload_all();
	return 0;
	// broker.subscribe(&serialPort, reinterpret_cast<ISink *>(dumper.handle));
	// } else {
	// 	PluginLoader::unload_all();
	// 	return 1;
	// }

	serialPort.open();

	byte buffer[256] = {};
	ssize_t bytesRead = serialPort.read(buffer, sizeof(buffer));

	PluginEvent event = {
		.topic	 = "serial.data",
		.payload = DataView(buffer, bytesRead)
	};

	if (bytesRead > 0) {
		broker.broadcast(&serialPort, event);
	}

	PluginLoader::unload_all();
	serialPort.close();
	return 0;
}
