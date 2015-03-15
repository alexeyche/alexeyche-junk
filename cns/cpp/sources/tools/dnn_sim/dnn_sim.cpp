
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/act_functions/determ.h>
#include <dnn/synapses/static_synapse.h>
#include <dnn/inputs/input_time_series.h>
#include <dnn/io/stream.h>
#include <dnn/base/constants.h>
#include <dnn/sim/sim.h>
#include <dnn/util/option_parser.h>

using namespace dnn;

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
	DnnSimOpts sopt;

	OptionParser optp(argc, argv);
	optp.option("--const", "-c", sopt.const_file, true);
	optp.option("--output", "-o", sopt.jobs, true);
	optp.option("--jobs", "-j", sopt.jobs, false);


	vector<string> rest_opts = optp.getRawOptions();
	for(size_t i=0; i<rest_opts.size(); i+=2) {
		if ((i+1) >= rest_opts.size()) {
            cerr << "Free option without an argument: " << rest_opts[i] << "\n";
            terminate();
        }
        const string& optname = rest_opts[i];
		const string& optvalue =  rest_opts[i+1];
		
        vector<string> s = dnn::split(optname, '-');

        if ((s.size() != 4) || (s[0] != "") || (s[1] != "")) {
            cerr << "Free option must be like that: --free-option\n";
            cerr << "\t got " << optname << "\n";
            terminate();
        }
        sopt.add_opts.insert(
            std::make_pair(
                "@" + string(s[2]) + "-" + string(s[3]), 
                optvalue
            )
        );
        sopt.add_opts[rest_opts[i]] = rest_opts[i+1];
	}
	
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
