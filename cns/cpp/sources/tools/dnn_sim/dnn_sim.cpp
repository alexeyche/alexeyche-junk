
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/act_functions/determ.h>
#include <dnn/synapses/static_synapse.h>
#include <dnn/inputs/ts_input.h>
#include <dnn/io/stream.h>
#include <dnn/base/constants.h>
#include <dnn/sim/sim.h>

#include "opts.h"

int main(int argc, char **argv) {
	DnnSimOpts sopt = parseOptions(argc, argv);
    dnn::Constants c(sopt.const_file);
	dnn::Sim s(c);
	s.build();
}
