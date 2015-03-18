#pragma once

#include <dnn/io/serialize.h>

namespace dnn {


struct SynapseInterface {
	stateDelegate propagateSpike;
	calculateDynamicsDelegate calculateDynamics;
	getDoubleDelegate getMembranePotential;
};

class Network;

class SynapseBase : public SerializableBase {
friend class Network;
public:
	typedef SynapseInterface interface;

	void setIdPre(size_t _id_pre) {
		id_pre = _id_pre;
	}
	void setDendriteDelay(double _dendrite_delay) {
		dendrite_delay = _dendrite_delay;
	}
	void setWeight(double w) {
		weight = w;
	}	
	inline const size_t& getIdPre() { 
		return id_pre;
	}

	virtual void propagateSpike() = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	virtual const double& getMembranePotential() = 0;
	virtual void provideInterface(SynapseInterface &i) = 0;

	static void provideDefaultInterface(SynapseInterface &i) {
		cerr << "No default interface for Synapse\n";
		terminate();
	}
protected:
	size_t id_pre;
	double dendrite_delay;
	double weight;
};


/*@GENERATE_PROTO@*/
struct SynapseInfo : public Serializable<Protos::SynapseInfo> {
	void serial_process() {
		begin() << "id_pre: " 		  << id_pre 		<< ", " \
		        << "dendrite_delay: " << dendrite_delay << ", " \
		        << "weight: " 		  << weight 		<< Self::end;
	}
	size_t id_pre;
	double dendrite_delay;
	double weight;
};



template <typename Constants, typename State>
class Synapse : public SynapseBase {

public:	
	SynapseInfo getInfo() {
		SynapseInfo info;
		info.id_pre = id_pre;
		info.dendrite_delay = dendrite_delay;
		info.weight = weight;
		return info;
	}

	void serial_process() {
		begin() << "Constants: " << c;

		if (messages->size() == 0) {
			(*this) << Self::end;
			return;
		}

		(*this) << "State: " << s;

		if (messages->size() == 0) {
			(*this) << Self::end;
			return;
		}
		SynapseInfo info;
		if (mode == ProcessingOutput) {
			info = getInfo();
		}

		(*this) << "SynapseInfo: "   << info;
		(*this) << Self::end;
	}
protected:
	State s;
	Constants c;
};


}
