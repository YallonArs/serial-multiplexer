#include "hal/SerialPort.h"
#include "broker/Broker.h"
#include "plugins/PluginLoader.h"

int main() {
	SerialPort serialPort({"/dev/ttyUSB0", 9600});
	Broker broker;

	LoadedPlugin dumper = PluginLoader::load_plugin("./plugins/Dumper.so");
	broker.subscribe(serialPort, dumper.adapter);

	serialPort.open();

	byte buffer[256] = {};
	size_t bytesRead = 0;
	serialPort.read(buffer, sizeof(buffer), bytesRead);
	if (bytesRead > 0) {
		broker.broadcast(serialPort, {buffer, bytesRead});
	}

	PluginLoader::unload_plugin(dumper);
	serialPort.close();
	return 0;
}
