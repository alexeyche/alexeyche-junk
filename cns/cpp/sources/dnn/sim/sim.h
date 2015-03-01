#pragma once

#include <dnn/base/factory.h>

namespace dnn {

class Sim {
public:
	Sim(const Constants &c) {
		SerializableBase *lif = Factory::inst().createObject("LeakyIntegrateAndFire");
		Stream s(cout, Stream::Text);
		s.writeObject(lif);

		//Serializer sr(s);
		//sr.write(lif);
	}
};

}