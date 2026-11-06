#include <iostream>
#include "plugins/IPlugin.h"
#include "dataview.h"

class Dumper : public IPlugin {
public:
	Dumper() = default;
	~Dumper() override = default;	

	void receive(const DataView &data) const noexcept {
		std::cout << "Dumper processing data of size: " << data.size << std::endl;
	}
};

REGISTER_PLUGIN(Dumper, "Dumper");
