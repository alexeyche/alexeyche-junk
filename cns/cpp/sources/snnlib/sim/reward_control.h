#pragma once

#include <snnlib/core.h>
#include <snnlib/config/constants.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/serialize/proto_rw.h>
#include <snnlib/protos/model.pb.h>
#include <snnlib/protos/stat.pb.h>

class Sim;
class Layer;
class Reward;

class RewardStat : public Serializable<Protos::RewardStat> {
public:
    RewardStat() : Serializable<Protos::RewardStat>(ERewardStat) { }
    
    void collect(Reward *rew);
    ProtoPack serialize();
    void deserialize();

    vector<double> r;
    vector<double> mean_r;

    void print(std::ostream& str) const {}
};

class Factory;

class Reward : public Serializable<Protos::Reward> {
protected:
    Reward() : Serializable(EReward) { }
    friend class Factory;
public:
	Reward(Layer *_l, RewardModConf _conf) : Serializable(EReward), r(0.0), mean_r(0.0), l(_l), conf(_conf), stat(nullptr), collectStatistics(false) {}
	
	ProtoPack serialize() {
		Protos::Reward *r_serial = getNewMessage();
	    r_serial->set_r(r);
	    r_serial->set_mean_r(mean_r);
	    return ProtoPack({r_serial});
	}
	void deserialize() {
        Protos::Reward *r_serial = getSerializedMessage();
        r = r_serial->r();
        mean_r = r_serial->mean_r();
	}

	void print(std::ostream& str) const {
		str << "r: " << r << " mean_r: " << mean_r;
	}

	void enableCollectStatistics() {
		stat = Factory::inst().registerObj<RewardStat>(new RewardStat());
		collectStatistics = true;
	}
	
	void saveStat(SerialPack &p) {
		p.push_back(stat);
	}
	inline void calculateDynamics(const double &dt) {
		mean_r += dt * ( - (mean_r-r)/conf.tau_mean_rew );
		r += dt * ( - (r - dr)/conf.tau_rew );
		if(collectStatistics) {
			stat->collect(this);
		}
	}

	double dr;
	double r;
	double mean_r;

	Layer *l;
	RewardModConf conf;

	RewardStat *stat;
	
	bool collectStatistics;	
};


class RewardControl : public Printable {
public:
	RewardControl() {}
	void init(Sim *s, const RewardConnectionMap &map);
	void sync();
	void simStep(const double &dt);
	void saveModel(ProtoRw &rw);
	void loadModel(ProtoRw &rw);
	void print(std::ostream& str) const;
	void enableCollectStatistics(const size_t &l_id);
	void saveStat(SerialPack &p);	

	vector<Reward> active_layers;
	
	// neurod addresed sim stuff, 
	vector<double> neuron_modulated_reward;
	vector<const Reward*> neuron_prepared_reward;	
};