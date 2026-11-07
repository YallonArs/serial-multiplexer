#include "broker/Broker.h"
#include "hal/SerialPort.h"
#include "plugins/PluginLoader.h"

int main() {
	SerialPort serialPort({"/dev/ttyUSB0", 9600});
	Broker broker;

	LoadedPlugin dumper = PluginLoader::load_plugin("./plugins/FileDumper.so");

	if (Flags(dumper.api->capabilities).check(static_cast<uint8_t>(PluginCapability::Sink))) {
		dumper.api->methods.receive(dumper.handle, PluginEvent {.topic = "serial.data", .payload = DataView(nullptr, 67)});
		PluginLoader::unload_all();
		return 0;
		// broker.subscribe(&serialPort, reinterpret_cast<ISink *>(dumper.handle));
	} else {
		PluginLoader::unload_all();
		return 1;
	}

	serialPort.open();

	byte buffer[256] = {};
	size_t bytesRead = 0;
	serialPort.read(buffer, sizeof(buffer), bytesRead);

	PluginEvent event = {
		.topic	 = "serial.data",
		.payload = DataView(buffer, bytesRead)};

	if (bytesRead > 0) {
		broker.broadcast(&serialPort, event);
	}

	PluginLoader::unload_all();
	serialPort.close();
	return 0;
}
