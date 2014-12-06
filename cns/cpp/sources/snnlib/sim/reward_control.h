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

class Reward : public Serializable<Protos::Reward> {
public:
	Reward() : Serializable(EReward), r(0.0), mean_r(0.0), stat(nullptr), collectStatistics(false) {}
	
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
	void collectStat() {
		stat->collect(this);
	}
	void saveStat(SerialPack &p) {
		p.push_back(stat);
	}

	double r;
	double mean_r;

	RewardStat *stat;
	bool collectStatistics;	
};

typedef tuple<Layer*, Reward, RewardModConf> RewardConfTuple;

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

	vector<RewardConfTuple> active_layers;
	
	// neurod addresed sim stuff, 
	vector<double> neuron_modulated_reward;
	vector<const Reward*> neuron_prepared_reward;	
};