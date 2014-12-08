
#include "runtime_globals.h"


#include "sim.h"

void RuntimeGlobals::initInputNeuronsFiringDelivery(Sim *s) {
    if(s->input_neurons_count != s->layers.back()->N) {
        cerr << "For learning on input neurons firings need last layer with same size as input neurons\n";
        terminate();
    }
    last_layer_id_offset = 0;
    for(size_t li=0; li < (s->layers.size()-1); li++) {
        last_layer_id_offset += s->layers[li]->N;
    }
    for(size_t in=0; in < s->input_neurons_count; in++) {
        input_spikes_iterators.push_back(0);
        input_neurons_firing.push_back(0);
    }
}


void RuntimeGlobals::setInputNeuronsFiring(const size_t &last_layer_neuron_id, const double &t) {
    size_t input_neuron_id = last_layer_neuron_id - last_layer_id_offset;
    if( (input_spikes_iterators[input_neuron_id] < net->spikes_list[input_neuron_id].size()) &&
        (net->spikes_list[input_neuron_id][ input_spikes_iterators[input_neuron_id] ] <= (t+dt))
      ) {

        input_neurons_firing[input_neuron_id] = 1;
        input_spikes_iterators[input_neuron_id] += 1;
    } else {
        input_neurons_firing[input_neuron_id] = 0;
    }
}
