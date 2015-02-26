#pragma once

namespace dnn {


struct SynapseInterface {
	stateDelegate propagateSpike;
	calculateDynamicsDelegate calculateDynamics;
};


class SynapseBase {
public:
	typedef SynapseInterface interface;

	virtual void propagateSpike() = 0;
	virtual void calculateDynamics(const Time &t) = 0;

	virtual void provideInterface(SynapseInterface &i) = 0;

	static void provideDefaultInterface(SynapseInterface &i) {
		cerr << "No default interface for Synapse\n";
		terminate();
	}
};


template <typename Constants, typename State>
class Synapse : public SynapseBase {

protected:
	State s;
	const Constants c;
};


}
