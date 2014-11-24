
#include "layer_info.h"

#include "layer.h"

Protos::LayerInfo *LayerInfo::serialize() {
    Protos::LayerInfo *info = getNew();
    Protos::NeuronInfo *ninfo = new Protos::NeuronInfo();
    ninfo->set_neuron(l->neuron_conf->neuron);
    ninfo->set_act_func(l->neuron_conf->act_func);
    if(!l->neuron_conf->learning_rule.empty()) {
        ninfo->set_learning_rule(l->neuron_conf->learning_rule);
    }
    if(!l->neuron_conf->tuning_curve.empty()) {
        ninfo->set_tuning_curve(l->neuron_conf->tuning_curve);
    }
    info->set_size(l->N);   
    info->set_id(l->id);
    info->set_allocated_neuron(ninfo);
    return info;
}
