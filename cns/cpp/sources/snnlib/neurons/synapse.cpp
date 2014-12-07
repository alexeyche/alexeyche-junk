
#include "synapse.h"

// // serialize

void Synapse::deserialize() {
	Protos::Synapse *syn = getSerializedMessage();
	w = syn->w();
    id_pre = syn->id_pre();
    dendrite_delay = syn->dendrite_delay();
    c = castType<SynapseC>((*constGlobalInstance)[syn->const_name()]);
}
ProtoPack Synapse::serialize() {
	Protos::Synapse *syn = getNewMessage();
    syn->set_w(w);
    syn->set_id_pre(id_pre);
    syn->set_dendrite_delay(dendrite_delay);
    syn->set_const_name(c->getName());
    return ProtoPack({syn});
}

