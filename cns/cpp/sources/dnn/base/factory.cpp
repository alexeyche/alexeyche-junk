#include "factory.h"

#include <dnn/base/dynamic_object.h>
#include <dnn/neurons/iaf_neuron.h>
#include <dnn/act_functions/determ.h>
#include <dnn/synapses/static_synapse.h>


namespace dnn {

Factory::Factory() {
    #include "type_map.h"
}


Factory& Factory::inst() {
    static Factory _inst;
    return _inst;
}

Factory::~Factory() {
    for(auto o: objects) {
        delete o;
    }
}


Factory::entity_map_type Factory::typemap;


}
