
#include "neuron_model.h"

#include "neuron.h"


Protos::NeuronModel *NeuronModel::serialize() {
    Protos::NeuronModel *stat = getNew();
    for(auto it=n->syns.begin(); it != n->syns.end(); ++it) {
        Synapse *s = *it;
        Protos::NeuronModel::Syn *syn = stat->add_syns();
        syn->set_w(s->w);
        syn->set_id_pre(s->id_pre);
        syn->set_dendrite_delay(s->dendrite_delay);
    }
    stat->set_axon_delay(n->axon_delay);
    return stat;
}

void NeuronModel::deserialize() {
    Protos::NeuronModel *m = castProtoMessage<Protos::NeuronModel>(serialized_message);
    n->syns.clear();
    for(size_t si=0; si<m->syns_size(); si++) {
        
 //       n->syns
    }    
}