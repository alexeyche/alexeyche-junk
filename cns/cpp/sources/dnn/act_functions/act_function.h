#pragma once

namespace dnn {


struct ActFunctionInterface {
	funDelegate prob;
	funDelegate probDeriv;
};


class ActFunctionBase {
public:
	typedef ActFunctionInterface interface;

	virtual double prob(const double &u) = 0;
    virtual double probDeriv(const double &u) = 0;

    virtual void provideInterface(ActFunctionInterface &i) = 0;

	static void provideDefaultInterface(ActFunctionInterface &i) {
    	cerr << "No default interface for act function\n";
    	terminate();
    }
};



template <typename Constants>
class ActFunction : public ActFunctionBase {

protected:
	const Constants c;
};

}
