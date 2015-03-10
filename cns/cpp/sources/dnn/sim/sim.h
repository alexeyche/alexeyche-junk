#pragma once

#include <dnn/base/factory.h>

namespace dnn {

class Sim {
public:
	Sim(const Constants &c) {
		SpikeNeuronBase *lif = Factory::inst().createSpikeNeuron("LeakyIntegrateAndFire");
		ActFunctionBase *act = Factory::inst().createActFunction("Determ");
		lif->setActFunction(act);
		lif->reset();

		// {
		// 	ofstream f("out.pb");
		// 	Stream s(f, Stream::Binary);
		// 	s.writeObject(lif);
		// }
		{
			ifstream f("../in.json");
			Stream s(f, Stream::Text);
			SerializableBase* b = s.readObject();
			SerializableBase* b2 = s.readObject();
			
			Stream(cout, Stream::Text).writeObject(b);
			Stream(cout, Stream::Text).writeObject(b2);
		}

		//cout <<
		//Serializer sr(s);
		//sr.write(lif);
	}
};

}
