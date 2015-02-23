#pragma once

#include <dnn/base/dynamic_object.h>
#include <dnn/base/abstract_factory.h>

namespace dnn {

using SpikeNeuronCtr = Object* (*)();

class SpikeNeuronFactory : public AbstractFactory<Object, SpikeNeuronCtr> {
public:

};




}
