#include "hal/SerialPort.h"
#include "broker/Broker.h"
#include "plugins/PluginLoader.h"

// Adapts a ByteSinkCap (C ABI) to ISink<DataView> for use with Broker
struct ByteSinkAdapter : IByteSink {
	const ByteSinkCap *cap;
	PluginHandle handle;
	ByteSinkAdapter(const ByteSinkCap *c, PluginHandle h) : cap(c), handle(h) {}
	void receive(const DataView &data) override { cap->receive(handle, data); }
};

int main() {
	SerialPort serialPort({"/dev/ttyUSB0", 9600});
	Broker<DataView> broker;

	LoadedPlugin dumper = PluginLoader::load_plugin("./plugins/FileDumper.so");
	ByteSinkAdapter adapter{dumper.query<ByteSinkCap>(CAP_BYTE_SINK), dumper.handle};
	broker.subscribe(&serialPort, adapter);

	serialPort.open();

	byte buffer[256] = {};
	size_t bytesRead = 0;
	serialPort.read(buffer, sizeof(buffer), bytesRead);
	if (bytesRead > 0) {
		broker.broadcast(&serialPort, {buffer, bytesRead});
	}

	PluginLoader::unload_plugin(dumper);
	serialPort.close();
	return 0;
}
