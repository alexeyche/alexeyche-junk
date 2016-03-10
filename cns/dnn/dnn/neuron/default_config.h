#pragma once

#include "config.h"

#include <dnn/synapse/basic_synapse.h>
#include <dnn/activation/determ.h>



namespace NDnn {

	using TDefaultConfig = TNeuronConfig<TBasicSynapse, TDeterm>;

} // namespace NDnn
