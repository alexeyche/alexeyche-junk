
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/act_functions/determ.h>
#include <dnn/synapses/static_synapse.h>
#include <dnn/inputs/input_time_series.h>
#include <dnn/io/stream.h>
#include <dnn/base/constants.h>
#include <dnn/sim/sim.h>
#include <dnn/util/option_parser.h>

using namespace dnn;
const char * usage = R"USAGE(
USAGE: ./dnn_sim  [options]

Options:
  --input, -i      Input protobuf file with precalculated spikes or labeled time
                   series.
  --output, -o     Output file with serialized spikes list
  --const, -c  Constants filename.
  --load, -l       Load model.
  --save, -s       Save model.
  --stat 	       Save statistics to file.
  --T-max, -T      Maximum simulation time (default: max input length).
  --jobs  -j       Parallel jobs to run (default 1)
  --no-learning    Turning off all learning dynamics despite of const.json
                   content. Good for statistics collecting
  --help           Print usage and exit.

Examples:
% Need to fill %
)USAGE";

struct DnnSimOpts {
    DnnSimOpts() : jobs(1), precalc(false), Tmax(0.0) {}
    string input;
    string const_file;
    string out_spikes;
    string out_stat_file;
    string out_p_stat_file;
    string model_save;
    string model_load;
    int jobs;
    bool precalc;
    bool no_learning;
    double Tmax;
    dnn::OptMods add_opts;
};


int main(int argc, char **argv) {
	if(argc == 1) {
		cout << usage;
		return 0; 
	}
	DnnSimOpts sopt;

	bool need_help = false;
	
	OptionParser optp(argc, argv);
	optp.option("--const", "-c", sopt.const_file, true);
	optp.option("--output", "-o", sopt.jobs, true);
	optp.option("--jobs", "-j", sopt.jobs, false);
	optp.loption("--stat", sopt.jobs, false);
	optp.option("--help", "-h", need_help, false, true);
	if(need_help) { 
		cout << usage;
		return 0; 
	}

	vector<string> rest_opts = optp.getRawOptions();
	sopt.add_opts = parseArgOptionsPairs(rest_opts);
	
	Constants c(sopt.const_file, sopt.add_opts);

	Sim s(c);

	if (!sopt.model_load.empty()) {
		ifstream fstr(sopt.model_load);
		Stream str_in(fstr, Stream::Binary);
		s.build(&str_in);
	} else {
		s.build();
	}
	s.run(sopt.jobs);

	if (!sopt.model_save.empty()) {
		ofstream fstr(sopt.model_save);
		Stream str_out(fstr, Stream::Binary);
		s.serialize(str_out);
	}
}
