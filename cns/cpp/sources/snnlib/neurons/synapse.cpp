
#include "synapse.h"

// // serialize

void Synapse::deserialize() {
	Protos::Synapse *syn = getSerializedMessage();
    init((*constGlobalInstance)[syn->const_name()], syn->id_pre(), syn->w(), syn->dendrite_delay());
}

ProtoPack Synapse::serialize() {
	Protos::Synapse *syn = getNewMessage();
    syn->set_w(w);
    syn->set_id_pre(id_pre);
    syn->set_dendrite_delay(dendrite_delay);
    syn->set_const_name(c->getName());
    return ProtoPack({syn});
}

