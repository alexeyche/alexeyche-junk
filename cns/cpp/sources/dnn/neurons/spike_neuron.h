#pragma once


#include <dnn/base/base.h>
#include <dnn/util/interfaced_ptr.h>
#include <dnn/act_functions/act_function.h>
#include <dnn/synapses/synapse.h>
#include <dnn/inputs/input.h>
#include <dnn/io/serialize.h>
#include <dnn/util/statistics.h>
#include <dnn/learning_rules/learning_rule.h>
#include <dnn/util/act_vector.h>
#include <dnn/util/spikes_list.h>

namespace dnn {


struct SpikeNeuronInterface {
	calculateNeuronDynamicsDelegate calculateDynamics;
	propSynSpikeDelegate propagateSynapseSpike;
	getDoubleDelegate getFiringProbability;	
};
extern size_t global_neuron_index;

class Builder;
class Network;

class SpikeNeuronBase : public SerializableBase {
friend class Builder;
friend class Network;
public:
	SpikeNeuronBase() : input_queue_lock(ATOMIC_FLAG_INIT), _fired(false) {
		_id = global_neuron_index++;		
	}

	typedef SpikeNeuronInterface interface;

	inline const size_t& id() const {
		return _id;
	}
	inline void setCoordinates(size_t xi, size_t yi, size_t colSize) {
		_xi = xi;
		_yi = yi;
		_colSize = colSize;
	}
	inline const size_t localId() const {
		return xi() + colSize()*yi();
	}
	inline const size_t& xi() const {
		return _xi;
	}
	inline const size_t& yi() const {
		return _yi;
	}
	inline const size_t& colSize() const {
		return _colSize;
	}
	inline const double& axonDelay() const {
		return _axonDelay;
	}
	inline double& mutAxonDelay() {
		return _axonDelay;
	}
	inline const bool& fired() const {
		return _fired;
	}
	
	inline void setFired(const bool& f) {
		_fired = f;
	}

	template <typename T>
	void provideInterface(SpikeNeuronInterface &i) {
        i.calculateDynamics = MakeDelegate(static_cast<T*>(this), &T::calculateDynamics);        
        i.getFiringProbability = MakeDelegate(static_cast<T*>(this), &T::getFiringProbability);
        i.propagateSynapseSpike = MakeDelegate(static_cast<T*>(this), &T::propagateSynapseSpike);
        ifc = i;
	}
	inline void resetInternal() {
		reset();
		if(lrule.isSet()) {
			lrule.ref().reset();
		}
		for(auto &s: syns) {
			s.ref().reset();
		}
	}
	virtual void reset() = 0;

	// runtime	
	virtual void propagateSynapseSpike(const SynSpike &sp) {
        syns[ sp.syn_id ].ifc().propagateSpike();
        lrule.ifc().propagateSynapseSpike(sp);
    }
	virtual void calculateDynamics(const Time& t, const double &Iinput, const double &Isyn) = 0;
	virtual const double& getFiringProbability() = 0;	


	static void __calculateDynamicsDefault(const Time &t, const double &Iinput, const double &Isyn) {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	static void __propagateSynapseSpikeDefault(const SynSpike &s) {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	static const double& __getFiringProbabilityDefault() {
		throw dnnException()<< "Calling inapropriate default interface function\n";
	}
	
	static void provideDefaultInterface(SpikeNeuronInterface &i) {
		i.calculateDynamics = &SpikeNeuronBase::__calculateDynamicsDefault;
		i.getFiringProbability = &SpikeNeuronBase::__getFiringProbabilityDefault;
		i.propagateSynapseSpike =  &SpikeNeuronBase::__propagateSynapseSpikeDefault;
	}

	void setLearningRule(LearningRuleBase *_lrule) { 
		lrule.set(_lrule); 
		lrule.ref().linkWithNeuron(*this);
	}
	void setActFunction(ActFunctionBase *_act_f) { 
		act_f.set(_act_f);
	}

	void setInput(InputBase *_input) { input.set(_input); }
	bool inputIsSet() {
		return input.isSet();
	}
	InputBase& getInput() {
		if(!input.isSet()) {
			throw dnnException() << "Trying to get input which is not set\n";
		}
		return input.ref();
	}
	
	void addSynapse(InterfacedPtr<SynapseBase> syn) {
		syns.push_back(syn);
	}
	inline ActVector<InterfacedPtr<SynapseBase>>& getSynapses() {
		return syns;
	}
	Statistics getStat() {
		Statistics statc = stat;
		auto& rstat = statc.getStats();
		auto it = rstat.begin();
		while(it != rstat.end()) {
			if(!strStartsWith(it->first, name())) {
				rstat[name() + "_" + it->first ] = it->second;
				it = rstat.erase(it);
			} else {
				++it;
			}
		}

		size_t syn_id = 0;
		for(auto &s: syns) {
			if(s.ref().getStat().on()) {
				Statistics& syn_st = s.ref().getStat();
				for(auto it=syn_st.getStats().begin(); it != syn_st.getStats().end(); ++it) {
					rstat[s.ref().name() + "_" +  it->first + std::to_string(syn_id)] = it->second;	
				}						
			}
			++syn_id;
		}
		if((lrule.isSet())&&(lrule.ref().getStat().on())) {
			Statistics &lrule_st = lrule.ref().getStat();
			for(auto it=lrule_st.getStats().begin(); it != lrule_st.getStats().end(); ++it) {
				rstat[ lrule.ref().name() + "_" + it->first ] = it->second;
			}
		}
		return statc;
	}

	inline void enqueueSpike(const SynSpike && sp) {
		while (input_queue_lock.test_and_set(std::memory_order_acquire));
		input_spikes.push(sp);
		input_queue_lock.clear(std::memory_order_release);
	}

	inline void readInputSpikes(const Time &t) {
		while (input_queue_lock.test_and_set(std::memory_order_acquire)) {}
        while(!input_spikes.empty()) {
            const SynSpike& sp = input_spikes.top();
            if(sp.t >= t.t) break;
            auto &s = syns[sp.syn_id];
            s.ref().setFired(true);
            ifc.propagateSynapseSpike(sp);
            input_spikes.pop();   
        }
        input_queue_lock.clear(std::memory_order_release);        
	}
	inline void calculateDynamicsInternal(const Time &t) {
        readInputSpikes(t);

        const double& Iinput = input.ifc().getValue(t);

		double Isyn = 0.0;
        auto syn_id_it = syns.ibegin();
        while(syn_id_it != syns.iend()) {
            auto &s = syns[syn_id_it];
            double x = s.ifc().getMembranePotential();
            if(fabs(x) < 0.0001) {
            	syns.setInactive(syn_id_it);
            } else {
            	Isyn += x;
	        	++syn_id_it;	
            }
        }
        ifc.calculateDynamics(t, Iinput, Isyn);
        
        lrule.ifc().calculateDynamics(t);
        if(stat.on()) {
       		for(auto &s: syns) {
       			s.ifc().calculateDynamics(t);
            	s.ref().setFired(false);	
       		}
        } else {
	        for(auto syn_id_it = syns.ibegin(); syn_id_it != syns.iend(); ++syn_id_it) {
	            auto &s = syns[syn_id_it];
	            s.ifc().calculateDynamics(t);
	            s.ref().setFired(false);
	        }
	    }
	}
	
	double getSimDuration() {
		if(input.isSet()) {
			return input.ref().getSimDuration();
		}
		return 0.0;
	}

protected:
	bool _fired;
	size_t _id;
	size_t _xi;
	size_t _yi;
	size_t _colSize;

	double _axonDelay;
	ActVector<InterfacedPtr<SynapseBase>> syns;

	InterfacedPtr<ActFunctionBase> act_f;
	InterfacedPtr<InputBase> input;
	InterfacedPtr<LearningRuleBase> lrule;

	Statistics stat;

	priority_queue<SynSpike> input_spikes;
	std::atomic_flag input_queue_lock;

	SpikeNeuronInterface ifc;
};

/*@GENERATE_PROTO@*/
struct SpikeNeuronInfo : public Serializable<Protos::SpikeNeuronInfo> {
	void serial_process() {
		begin() << "id: " << id << ", " \
				<< "xi: " << xi << ", " \
				<< "yi: " << yi << ", " \
				<< "colSize: " << colSize << ", " \
		        << "axonDelay: " << axonDelay << ", " \
		        << "num_of_synapses: " << num_of_synapses << ", " \
		        << "act_function_is_set: " << act_function_is_set << ", " \
		        << "input_is_set: " << input_is_set
		        << "lrule_is_set: " << lrule_is_set << Self::end;
	}
	size_t id;
	size_t xi;
	size_t yi;
	size_t colSize;
	double axonDelay;
	size_t num_of_synapses;
	bool act_function_is_set;
	bool input_is_set;
	bool lrule_is_set;
};

template <typename Constants, typename State>
class SpikeNeuron : public SpikeNeuronBase {
public:
	SpikeNeuronInfo getInfo() {
		SpikeNeuronInfo info;
		info.id = id();
		info.xi = xi();
		info.yi = yi();
		info.colSize = colSize();
		info.axonDelay = axonDelay();
		info.num_of_synapses = syns.size();
		info.act_function_is_set = act_f.isSet();
		info.input_is_set = input.isSet();
		info.lrule_is_set = lrule.isSet();
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

		SpikeNeuronInfo info;
		if (mode == ProcessingOutput) {
			info = getInfo();
		}

		(*this) << "SpikeNeuronInfo: "   << info;

		if (info.act_function_is_set) {
			(*this) << "ActFunction: " << act_f;
		}
		if (info.input_is_set) {
			(*this) << "Input: " << input;
		}
		if (info.lrule_is_set) {
			(*this) << "LearningRule: " << lrule;
			lrule.ref().linkWithNeuron(*this);
		}
		if(mode == ProcessingInput) {
			syns.resize(info.num_of_synapses);
		}
		for (size_t i = 0; i < info.num_of_synapses; ++i) {
			(*this) << "Synapse: " << syns[i];
		}
		if(mode == ProcessingInput) {
			_xi = info.xi;
			_yi = info.yi;
			_id = info.id;
			_colSize = info.colSize;
			_axonDelay = info.axonDelay;
		}
		(*this) << Self::end;
	}

protected:
	State s;
	Constants c;
};


}
