#include "hal/SerialPort.h"
#include "plugins/PluginLoader.h"

int main() {
	// SerialPort serialPort({"/dev/ttyUSB0", 9600});
	// serialPort.open();

	// serialPort.write(reinterpret_cast<const byte *>("Hello, Serial Port!"), 20);

	PluginLoader::load_plugin("./plugins/ExamplePlugin.so");
	return 0;
}

