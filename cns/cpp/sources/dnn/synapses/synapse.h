#pragma once

#include <dnn/io/serialize.h>
#include <dnn/util/statistics.h>

namespace dnn {


struct SynapseInterface {
	stateDelegate propagateSpike;
	calculateDynamicsDelegate calculateDynamics;
	retDoubleDelegate getMembranePotential;
};

class Network;
class SpikeNeuronBase;
class Builder;

class SynapseBase : public SerializableBase {
friend class Network;
friend class SpikeNeuronBase;
friend class Builder;
public:
	typedef SynapseInterface interface;

	inline const size_t& getIdPre() { 
		return id_pre;
	}

	virtual void propagateSpike() = 0;
	virtual double getMembranePotential() = 0;
	virtual void calculateDynamics(const Time &t) = 0;
	virtual void reset() = 0;
		
	template <typename T>
	void provideInterface(SynapseInterface &i) {
        i.propagateSpike = MakeDelegate(static_cast<T*>(this), &T::propagateSpike);
        i.calculateDynamics = MakeDelegate(static_cast<T*>(this), &T::calculateDynamics);
        i.getMembranePotential = MakeDelegate(static_cast<T*>(this), &T::getMembranePotential);
    }

	static void provideDefaultInterface(SynapseInterface &i) {
		throw dnnException()<< "No default interface for Synapse\n";
	}
	Statistics& getStat() {
		return stat; 
	}
	inline bool& fired() {
		return _fired;
	}
	inline double& getMutWeight() {
		return weight;
	}
	inline const double& getWeight() {
		return weight;
	}
protected:
	size_t id_pre;
	double dendrite_delay;
	double weight;
	bool _fired;

	Statistics stat;
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
