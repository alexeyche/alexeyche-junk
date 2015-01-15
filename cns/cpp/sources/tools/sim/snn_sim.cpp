

#include <iostream>
#include <stdlib.h>

#include <snnlib/util/optionparser/opt.h>
#include <snnlib/config/constants.h>

#include <snnlib/sim/sim.h>

#include <snnlib/util/time_series.h>
#include <snnlib/util/fastapprox/fastpow.h>

enum  optionIndex { ARG_UNKNOWN, ARG_HELP, ARG_CONSTANTS, ARG_INPUT,
                    ARG_OUT_STAT, ARG_OUT_SPIKES, ARG_JOBS ,ARG_PRECALC,
                    ARG_MODEL_SAVE, ARG_MODEL_LOAD, ARG_T_MAX, ARG_OUT_P_STAT, ARG_NO_LEARNING};
const option::Descriptor usage[] =
{
 {ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {ARG_INPUT, 0,"i","input",Arg::NonEmpty, "  --input, -i  \tInput protobuf file with precalculated spikes or labeled time series." },
 {ARG_OUT_SPIKES, 0,"o","output",Arg::NonEmpty, "  --output, -o  \tOutput file with serialized spikes list" },
 {ARG_CONSTANTS, 0,"c","constants",Arg::NonEmpty, "  --constants, -c  \tConstants filename." },
 {ARG_MODEL_LOAD, 0,"l","load",Arg::NonEmpty, "  --load, -l  \tLoad model." },
 {ARG_MODEL_SAVE, 0,"s","save",Arg::NonEmpty, "  --save, -s  \tSave model." },
 {ARG_T_MAX, 0,"T","T-max",Arg::NonEmpty, "  --T-max, -T  \tMaximum simulation time (default: max input length)." },
 {ARG_OUT_STAT, 0,"","stat",Arg::NonEmpty, "  --stat  \tFile name to save detailed statistics." },
 {ARG_OUT_P_STAT, 0,"","p-stat",Arg::NonEmpty, "  --p-stat  \tFile name to save probabilites of model." },
 {ARG_JOBS, 0,"j","jobs",Arg::NonEmpty, "  --jobs  -j \tParallel jobs to run (default 1)" },
 {ARG_PRECALC, 0,"", "precalc",Arg::None, "  --precalc  \tOnly precalculate spike on time series." },
 {ARG_NO_LEARNING, 0,"", "no-learning",Arg::None, "  --no-learning  \tTurning off all learning dynamics despite of const.json content. Good for statistics collecting" },
 {ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};

struct SnnSimOpts {
    SnnSimOpts() : jobs(1), precalc(false), Tmax(0.0) {}
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

SnnSimOpts parseOptions(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max];
    option::Option buffer[stats.buffer_max];

    option::Parser parse(usage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);

    if ((options[ARG_HELP]) || (argc == 0)) {
        option::printUsage(cout, usage);
        exit(0);
    }

    if(options[ARG_INPUT].count() == 0) {
        cerr << "Need input argument\n";
        exit(1);
    }
    if(options[ARG_OUT_SPIKES].count() == 0) {
        cerr << "Need output spikes argument\n";
        exit(1);
    }

    for (option::Option* opt = options[ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[ARG_UNKNOWN].count() > 0) exit(1);

    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);


    SnnSimOpts sopt;
    sopt.input = options[ARG_INPUT].last()->arg;
    sopt.out_spikes = options[ARG_OUT_SPIKES].last()->arg;
    if(options[ARG_CONSTANTS].count()>0) {
        sopt.const_file = options[ARG_CONSTANTS].last()->arg;
    }
    if(options[ARG_JOBS].count()>0) {
        sopt.jobs = atoi(options[ARG_JOBS].last()->arg);
    }
    if(options[ARG_OUT_STAT].count()>0) {
        sopt.out_stat_file = options[ARG_OUT_STAT].last()->arg;
    }
    if(options[ARG_OUT_P_STAT].count()>0) {
        sopt.out_p_stat_file = options[ARG_OUT_P_STAT].last()->arg;
    }
    if(options[ARG_PRECALC].count()>0) {
        sopt.precalc = true;
    }
    if(options[ARG_MODEL_SAVE].count()>0) {
        sopt.model_save = options[ARG_MODEL_SAVE].last()->arg;
    }
    if(options[ARG_MODEL_LOAD].count()>0) {
        sopt.model_load = options[ARG_MODEL_LOAD].last()->arg;
    }
    if(options[ARG_T_MAX].count()>0) {
        sopt.Tmax = atof(options[ARG_T_MAX].last()->arg);
        cout << sopt.Tmax <<"\n";
    }
    if(options[ARG_NO_LEARNING].count()>0) {
        sopt.no_learning = true;
    }

    if((!sopt.out_p_stat_file.empty())&&(!sopt.out_stat_file.empty())) {
        cout << "Need to choose on of the mode of collecting statistics\n";
        terminate();
    }
    return sopt;
}


int main(int argc, char **argv) {
    SnnSimOpts sopt = parseOptions(argc, argv);

    Sim s(sopt.jobs, !sopt.no_learning);
    Constants *c = nullptr;
    if(!sopt.const_file.empty()) {
        c = new Constants(sopt.const_file);
        s.construct(*c);
        cout << *c;
    } else {
        if(sopt.model_load.empty()) {
            cerr << "Need const.json or model for load\n";
            terminate();
        }
    }
    if(!sopt.model_load.empty()) {
        s.loadModel(sopt.model_load);
    }

    s.setOutputSpikesFile(sopt.out_spikes);

    ProtoRw prw(sopt.input, ProtoRw::Read);
    SerializableBase* inp = prw.read();
    if(inp->getName() == "LabeledTimeSeriesList") {
        LabeledTimeSeriesList *lst = static_cast<LabeledTimeSeriesList*>(inp);
        s.setInputTimeSeries(*lst);
    } else
    if(inp->getName() == "LabeledSpikesList") {
        LabeledSpikesList *sl = static_cast<LabeledSpikesList*>(inp);
        s.setInputSpikesList(*sl);
    } else {
        cerr << "Inappropriate input file " << inp->getName() << "\n";
        terminate();
    }

    s.setTlimit(sopt.Tmax);

    if(!sopt.out_stat_file.empty()) {
        s.monitorStat(sopt.out_stat_file);
    }
    if(!sopt.out_p_stat_file.empty()) {
        s.monitorPStat(sopt.out_p_stat_file);
    }

    if(sopt.precalc) {
        s.precalculateInputSpikes();
    } else {
        s.run();
    }

    if(!sopt.model_save.empty()) {
        s.saveModel(sopt.model_save);
    }
    if(c) {
        delete c;
    }
}
