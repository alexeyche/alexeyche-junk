#pragma once

#include <dnn/io/serialize.h>

namespace dnn {


struct ActFunctionInterface {
	funDelegate prob;
	funDelegate probDeriv;
};


class ActFunctionBase : public SerializableBase {
public:
	typedef ActFunctionInterface interface;

	virtual double prob(const double &u) = 0;
    virtual double probDeriv(const double &u) = 0;

    virtual void provideInterface(ActFunctionInterface &i) = 0;

	static double __default(const double &u) { 
		cerr << "Calling inapropriate default interface function\n";
		terminate();
	}
	
	static void provideDefaultInterface(ActFunctionInterface &i) {
    	i.prob = &ActFunctionBase::__default;
    	i.probDeriv = &ActFunctionBase::__default;
    }
};



template <typename Constants>
class ActFunction : public ActFunctionBase {
	void serial_process() {
		begin() << "Constants: " << c << end();
	}
protected:
	Constants c;
};

}
