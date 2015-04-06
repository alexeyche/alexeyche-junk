#pragma once

#include <dnn/io/serialize.h>
#include <dnn/base/exceptions.h>

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

    template <typename T>
	void provideInterface(ActFunctionInterface &i) {
        i.prob = MakeDelegate(static_cast<T*>(this), &T::prob);
        i.probDeriv = MakeDelegate(static_cast<T*>(this), &T::probDeriv);
    }
	static double __default(const double &u) { 
		throw dnnException() << "Calling inapropriate default interface function\n";
	}
	
	static void provideDefaultInterface(ActFunctionInterface &i) {
    	i.prob = &ActFunctionBase::__default;
    	i.probDeriv = &ActFunctionBase::__default;
    }
};



template <typename Constants>
class ActFunction : public ActFunctionBase {
	void serial_process() {
		begin() << "Constants: " << c << Self::end;
	}
protected:
	Constants c;
};

}
