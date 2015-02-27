
#include <dnn/util/optionparser/opt.h>
#include <dnn/neurons/leaky_integrate_and_fire.h>
#include <dnn/act_functions/determ.h>
#include <dnn/synapses/static_synapse.h>
#include <dnn/inputs/ts_input.h>
#include <dnn/io/stream.h>
#include <dnn/base/constants.h>



struct opts {
	enum  optionIndex { ARG_UNKNOWN, ARG_HELP, ARG_CONSTANTS, ARG_INPUT,
    	                ARG_OUT_STAT, ARG_OUT_SPIKES, ARG_JOBS ,ARG_PRECALC,
        	            ARG_MODEL_SAVE, ARG_MODEL_LOAD, ARG_T_MAX, ARG_OUT_P_STAT, ARG_NO_LEARNING};
};

const option::Descriptor usage[] =
{
 {opts::ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {opts::ARG_INPUT, 0,"i","input",Arg::NonEmpty, "  --input, -i  \tInput protobuf file with precalculated spikes or labeled time series." },
 {opts::ARG_OUT_SPIKES, 0,"o","output",Arg::NonEmpty, "  --output, -o  \tOutput file with serialized spikes list" },
 {opts::ARG_CONSTANTS, 0,"c","constants",Arg::NonEmpty, "  --constants, -c  \tConstants filename." },
 {opts::ARG_MODEL_LOAD, 0,"l","load",Arg::NonEmpty, "  --load, -l  \tLoad model." },
 {opts::ARG_MODEL_SAVE, 0,"s","save",Arg::NonEmpty, "  --save, -s  \tSave model." },
 {opts::ARG_T_MAX, 0,"T","T-max",Arg::NonEmpty, "  --T-max, -T  \tMaximum simulation time (default: max input length)." },
 {opts::ARG_OUT_STAT, 0,"","stat",Arg::NonEmpty, "  --stat  \tFile name to save detailed statistics." },
 {opts::ARG_OUT_P_STAT, 0,"","p-stat",Arg::NonEmpty, "  --p-stat  \tFile name to save probabilites of model." },
 {opts::ARG_JOBS, 0,"j","jobs",Arg::NonEmpty, "  --jobs  -j \tParallel jobs to run (default 1)" },
 {opts::ARG_PRECALC, 0,"", "precalc",Arg::None, "  --precalc  \tOnly precalculate spike on time series." },
 {opts::ARG_NO_LEARNING, 0,"", "no-learning",Arg::None, "  --no-learning  \tTurning off all learning dynamics despite of const.json content. Good for statistics collecting" },
 {opts::ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {opts::ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};

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
};

DnnSimOpts parseOptions(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option *options = new option::Option[stats.options_max];
    option::Option *buffer = new option::Option[stats.buffer_max];

    option::Parser parse(usage, argc, argv, options, buffer);


    if (parse.error()) {
    	delete [] buffer;
    	delete [] options;
    	cerr << "Errors while parsing options\n";
    	terminate();
    }

	delete [] buffer;

    if ((options[opts::ARG_HELP]) || (argc == 0)) {
        option::printUsage(cout, usage);
        exit(0);
    }

    if(options[opts::ARG_INPUT].count() == 0) {
        cerr << "Need input argument\n";
        exit(1);
    }
    if(options[opts::ARG_OUT_SPIKES].count() == 0) {
        cerr << "Need output spikes argument\n";
        exit(1);
    }

    for (option::Option* opt = options[opts::ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[opts::ARG_UNKNOWN].count() > 0) exit(1);

    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);


    DnnSimOpts sopt;
    sopt.input = options[opts::ARG_INPUT].last()->arg;
    sopt.out_spikes = options[opts::ARG_OUT_SPIKES].last()->arg;
    if(options[opts::ARG_CONSTANTS].count()>0) {
        sopt.const_file = options[opts::ARG_CONSTANTS].last()->arg;
    }
    if(options[opts::ARG_JOBS].count()>0) {
        sopt.jobs = atoi(options[opts::ARG_JOBS].last()->arg);
    }
    if(options[opts::ARG_OUT_STAT].count()>0) {
        sopt.out_stat_file = options[opts::ARG_OUT_STAT].last()->arg;
    }
    if(options[opts::ARG_OUT_P_STAT].count()>0) {
        sopt.out_p_stat_file = options[opts::ARG_OUT_P_STAT].last()->arg;
    }
    if(options[opts::ARG_PRECALC].count()>0) {
        sopt.precalc = true;
    }
    if(options[opts::ARG_MODEL_SAVE].count()>0) {
        sopt.model_save = options[opts::ARG_MODEL_SAVE].last()->arg;
    }
    if(options[opts::ARG_MODEL_LOAD].count()>0) {
        sopt.model_load = options[opts::ARG_MODEL_LOAD].last()->arg;
    }
    if(options[opts::ARG_T_MAX].count()>0) {
        sopt.Tmax = atof(options[opts::ARG_T_MAX].last()->arg);
        cout << sopt.Tmax <<"\n";
    }
    if(options[opts::ARG_NO_LEARNING].count()>0) {
        sopt.no_learning = true;
    }
    delete [] options;

    if((!sopt.out_p_stat_file.empty())&&(!sopt.out_stat_file.empty())) {
        cout << "Need to choose on of the mode of collecting statistics\n";
        terminate();
    }
    return sopt;
}

using namespace dnn;
int main(int argc, char **argv) {
	DnnSimOpts sopt = parseOptions(argc, argv);
    Constants c(sopt.const_file);
    Stream s(cout, Stream::Text);
    LeakyIntegrateAndFire lif;
    s << lif;
}
