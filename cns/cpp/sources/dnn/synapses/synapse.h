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
	SynapseBase() : _fired(false) {}
	typedef SynapseInterface interface;

	inline const size_t& idPre() { 
		return _idPre;
	}
	inline size_t& mutIdPre() { return _idPre; }

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
	static void __defaultPropagateSpike() { 
		throw dnnException() << "Calling inapropriate default interface function\n";
	}
	static void __defaultCalculateDynamics(const Time &t) { 
		throw dnnException() << "Calling inapropriate default interface function\n";
	}
	static double __defaultGetMembranePotential() { 
		throw dnnException() << "Calling inapropriate default interface function\n";
	}

	static void provideDefaultInterface(SynapseInterface &i) {
		i.propagateSpike = &SynapseBase::__defaultPropagateSpike;
        i.calculateDynamics = &SynapseBase::__defaultCalculateDynamics;
        i.getMembranePotential = &SynapseBase::__defaultGetMembranePotential;
	}
	Statistics& getStat() {
		return stat; 
	}
	inline void setFired(bool fired) {
		_fired = fired;
	}
	inline const double fired() const {
		return _fired;
	}
	inline double& mutWeight() {
		return _weight;
	}
	inline const double& weight() {
		return _weight;
	}
	inline const double& dendriteDelay() {
		return _dendriteDelay;
	}
	inline double& mutDendriteDelay() { 
		return _dendriteDelay; 
	}

protected:
	size_t _idPre;
	double _dendriteDelay;
	double _weight;
	bool _fired;

	Statistics stat;
};


/*@GENERATE_PROTO@*/
struct SynapseInfo : public Serializable<Protos::SynapseInfo> {
	void serial_process() {
		begin() << "idPre: " 		  << idPre 		<< ", " \
		        << "dendriteDelay: " << dendriteDelay << ", " \
		        << "weight: " 		  << weight 		<< Self::end;
	}
	size_t idPre;
	double dendriteDelay;
	double weight;
};



template <typename Constants, typename State>
class Synapse : public SynapseBase {

public:	
	SynapseInfo getInfo() {
		SynapseInfo info;
		info.idPre = _idPre;
		info.dendriteDelay = _dendriteDelay;
		info.weight = _weight;
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
		
		if (mode == ProcessingInput) {
			_weight = info.weight;
			_idPre = info.idPre;
			_dendriteDelay = info.dendriteDelay;
		}

		(*this) << Self::end;
	}
protected:
	State s;
	Constants c;
};


}
