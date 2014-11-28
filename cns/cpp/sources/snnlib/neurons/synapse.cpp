
#include "synapse.h"

// serialize
void loadModel(ProtoRw &rw) {
    Protos::Synapse syn;
    rw.readMessage(&syn);
    w = syn->w();
    id_pre = syn->id_pre();
    dendrite_delay = syn->dendrite_delay();
}

void saveModel(ProtoRw &rw) {
    Protos::Synapse syn;
    syn.set_w(w);
    syn.set_id_pre(id_pre);
    syn.set_dendrite_delay(dendrite_delay);
    syn.set_const_name(c->getName());
    rw.writeMessage(&syn);
}
