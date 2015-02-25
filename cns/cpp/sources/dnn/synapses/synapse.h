#pragma once


struct SynapseInterface {
	stateDelegate propagateSpike;
	stateDelegate calculateDynamics;

};

class SynapseBase {
public:
	typedef SynapseInterface interface;

	virtual void propagateSpike() = 0;
	virtual void calculateDynamics() = 0;

	virtual void provideInterface(SynapseInterface &i) = 0;

	static void provideDefaultInterface(SynapseInterface &i) {
		cerr << "No default interface for Synapse\n";
		terminate();
	}
};


template <typename Constants, typename State>
class Synapse {
public:
	Synapse(const Constants _c) : c(_c) {}

protected:
	State s;
	const Constants c;
};

