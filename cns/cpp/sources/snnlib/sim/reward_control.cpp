

#include "reward_control.h"
#include "sim.h"

#include <snnlib/layers/layer.h>

void RewardStat::collect(Reward *rew) {
	r.push_back(rew->r);
	mean_r.push_back(rew->mean_r);
}
ProtoPack RewardStat::serialize() {
    Protos::RewardStat *stat = getNewMessage();
    for(auto it=r.begin(); it != r.end(); ++it) {
        stat->add_r(*it);
    }
    for(auto it=mean_r.begin(); it != mean_r.end(); ++it) {
        stat->add_mean_r(*it);
    }
    return ProtoPack({stat});
}

void RewardStat::deserialize() {
    Protos::RewardStat * m = getSerializedMessage();
    for(size_t i=0; i<m->r_size(); i++) {
        r.push_back(m->r(i));
    }
    for(size_t i=0; i<m->mean_r_size(); i++) {
        mean_r.push_back(m->mean_r(i));
    }

}



void RewardControl::init(Sim *s, const RewardConnectionMap &map) {
    size_t total_size = s->input_neurons_count + s->net_neurons_count;
    neuron_modulated_reward.resize(total_size);
    neuron_prepared_reward.resize(total_size);
    for(size_t i=0; i<total_size; i++) {
    	neuron_modulated_reward[i] = 0.0;
    	neuron_prepared_reward[i] = nullptr;
    }


    for(auto it=map.cbegin(); it != map.cend(); ++it) {
        pair<size_t, vector<size_t> > l_ids = it->first;
        Layer *pre = nullptr;
        if(l_ids.first < s->layers.size()) {
            pre = s->layers[l_ids.first];
        }
        active_layers.push_back(Reward(pre, it->second));
        for(size_t ni=0; ni<pre->neurons.size(); ni++) {
            neuron_modulated_reward[pre->neurons[ni]->id] = 0.0;
        }

        for(auto post_id_it = l_ids.second.begin(); post_id_it != l_ids.second.end(); ++post_id_it) {
        	Layer *post = s->layers[*post_id_it];
        	for(size_t ni=0; ni<post->neurons.size(); ni++) {
        		neuron_prepared_reward[post->neurons[ni]->id] = &active_layers.back();
        	}
        }
    }

}

void RewardControl::sync() {
	for(auto it=active_layers.begin(); it != active_layers.end(); ++it) {
        Layer *l = it->l;
        it->dr = 0.0;
        //cout << "\t" << l->id << ":   ";
        for(size_t ni=0; ni<l->neurons.size(); ni++) {
			it->dr += neuron_modulated_reward[l->neurons[ni]->id];
            neuron_modulated_reward[l->neurons[ni]->id] = 0.0;
            //cout << ni << ":" <<  neuron_modulated_reward[l->neurons[ni]->id] << ", ";
		}
        //cout << "\n";        
	}
}
void RewardControl::simStep(const double &dt) {
	for(auto it=active_layers.begin(); it != active_layers.end(); ++it) {
		it->calculateDynamics(dt);
	}
}
void RewardControl::saveModel(ProtoRw &rw) {
	for(auto it = active_layers.begin(); it != active_layers.end(); ++it) {
    	rw.write(&*it);
    }
}
void RewardControl::loadModel(ProtoRw &rw) {
	for(auto it = active_layers.begin(); it != active_layers.end(); ++it) {
        rw.readAllocated(&*it);
    }
}

void RewardControl::print(std::ostream& str) const {
	for(auto it = active_layers.begin(); it != active_layers.end(); ++it) {
		str << "Layer " << it->l->id << " modulated reward: \n\t" << *it << "\n";
	}
}
void RewardControl::enableCollectStatistics(const size_t &l_id) {
	bool found = false;
	for(auto it = active_layers.begin(); it != active_layers.end(); ++it) {
		if(it->l->id == l_id) {
			it->enableCollectStatistics();
			found = true;
		}
	}
	if(!found) {
		cerr << "Can't find layer with id " << l_id << " in RewardControl center\n";
		terminate();
	}
}

void RewardControl::saveStat(SerialPack &p) {
	for(auto it = active_layers.begin(); it != active_layers.end(); ++it) {
		if(it->collectStatistics) {
			it->saveStat(p);
		}
	}
}
