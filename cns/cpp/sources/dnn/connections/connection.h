#pragma once

#include <dnn/io/serialize.h>

namespace dnn {

struct ConnectionRecipe {
	ConnectionRecipe() : inhibitory(false), amplitude(1.0), exists(false) {}
	bool inhibitory;
	double amplitude;
	bool exists;
};

class ConnectionBase : public SerializableBase {
public:
	ConnectionBase() {}

	virtual ConnectionRecipe getConnectionRecipe(const SpikeNeuronBase &left, const SpikeNeuronBase &right) = 0;	
};

template <typename Constants>
class Connection : public ConnectionBase {
	void serial_process() {
		begin() << "Constants: " << c << Self::end;
	}
protected:
	Constants c;
};



}