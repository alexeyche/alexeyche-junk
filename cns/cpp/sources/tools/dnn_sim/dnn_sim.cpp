
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/act_functions/determ.h>
#include <dnn/synapses/static_synapse.h>
#include <dnn/inputs/input_time_series.h>
#include <dnn/io/stream.h>
#include <dnn/base/constants.h>
#include <dnn/sim/sim.h>

#include "opts.h"

using namespace dnn;

int main(int argc, char **argv) {
	DnnSimOpts sopt = parseOptions(argc, argv);
	Constants c(sopt.const_file);
	Sim s(c);

	if (!sopt.model_load.empty()) {
		ifstream fstr(sopt.model_load);
		Stream str_in(fstr, Stream::Binary);
		s.build(&str_in);
	} else {
		s.build();
	}

	if (!sopt.model_save.empty()) {
		ofstream fstr(sopt.model_save);
		Stream str_out(fstr, Stream::Binary);
		s.serialize(str_out);
	}
}
