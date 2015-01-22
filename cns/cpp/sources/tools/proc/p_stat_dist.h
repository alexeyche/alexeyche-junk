#pragma once


#include "p_stat_calc.h"

enum  pStatDistOptionIndex { STAT_DIST_PROG_ARG_UNKNOWN, STAT_DIST_PROG_ARG_HELP, STAT_DIST_PROG_ARG_SPIKES, STAT_DIST_PROG_ARG_P_STAT, STAT_DIST_PROG_ARG_JOBS, STAT_DIST_PROG_ARG_OUT_JSON, STAT_DIST_PROG_ARG_TEST_SPIKES, STAT_DIST_PROG_ARG_TEST_P_STAT };
const option::Descriptor pStatDistUsage[] =
{
 {STAT_DIST_PROG_ARG_UNKNOWN, 0, "", "",Arg::None, "USAGE: example [options]\n\n"
                                        "Options:" },
 {STAT_DIST_PROG_ARG_SPIKES, 0,"s","spikes",Arg::NonEmpty, "  --spikes, -s  \tLabeled Spikes List to process" },
 {STAT_DIST_PROG_ARG_TEST_SPIKES, 0,"ts","test-spikes",Arg::NonEmpty, "  --test-spikes, -ts  \tTest set of Labeled Spikes List to process" },
 {STAT_DIST_PROG_ARG_P_STAT, 0,"p","p-stat",Arg::NonEmpty, "  --p-stat, -p  \tProbability statistics of model to proccess" },
 {STAT_DIST_PROG_ARG_TEST_P_STAT, 0,"tp","test-p-stat",Arg::NonEmpty, "  --test-p-stat, -tp  \tTest set of probability statistics of model to proccess" },
 {STAT_DIST_PROG_ARG_OUT_JSON, 0,"o","output",Arg::NonEmpty, "  --output, -o  \tOutput file in json format" },
 {STAT_DIST_PROG_ARG_JOBS, 0,"j","jobs",Arg::NonEmpty, "  --jobs  -j \tParallel jobs to run (default 1)" },
 {STAT_DIST_PROG_ARG_HELP, 0,"h", "help",Arg::None, "  --help  \tPrint usage and exit." },

 {STAT_DIST_PROG_ARG_UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
                               "% Need to fill %" },
 {0,0,0,0,0,0}
};


struct PStatDistanceOpts {
    PStatDistanceOpts() : jobs(1) {}
    string spikes;
    string test_spikes;
    string p_stat;
    string test_p_stat;
    string output;
    int jobs;
};

PStatDistanceOpts parsePStatDistOpts(int argc, char **argv) {
    argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
    option::Stats  stats(pStatDistUsage, argc, argv);
    option::Option options[stats.options_max];
    option::Option buffer[stats.buffer_max];

    option::Parser parse(pStatDistUsage, argc, argv, options, buffer);
    if (parse.error())
        exit(1);

    if ((options[STAT_DIST_PROG_ARG_HELP]) || (argc == 0)) {
        option::printUsage(cout, pStatDistUsage);
        exit(0);
    }

    if(options[STAT_DIST_PROG_ARG_SPIKES].count() != 1) {
        cerr << "Inappropriate spikes argument\n";
        exit(1);
    }
    if(options[STAT_DIST_PROG_ARG_P_STAT].count() != 1) {
        cerr << "Inappropriate --p-stat argument\n";
        exit(1);
    }
    for (option::Option* opt = options[STAT_DIST_PROG_ARG_UNKNOWN]; opt; opt = opt->next()) {
      cerr << "Unknown option: " << string(opt->name,opt->namelen) << "\n";
    }
    if(options[STAT_DIST_PROG_ARG_UNKNOWN].count() > 0) exit(1);
    for (int i = 0; i < parse.nonOptionsCount(); ++i) {
      cerr << "Non-options arguments are not supported (" << i << ": " << parse.nonOption(i) << ")\n";
    }
    if(parse.nonOptionsCount()>0) exit(1);

    PStatDistanceOpts opts;
    if(options[STAT_DIST_PROG_ARG_JOBS].count()>0) {
        opts.jobs = atoi(options[STAT_DIST_PROG_ARG_JOBS].arg);
    }
    if(options[STAT_DIST_PROG_ARG_SPIKES].count()>0) {
        opts.spikes = options[STAT_DIST_PROG_ARG_SPIKES].arg;
    }
    if(options[STAT_DIST_PROG_ARG_TEST_SPIKES].count()>0) {
        opts.test_spikes = options[STAT_DIST_PROG_ARG_TEST_SPIKES].arg;
    }    
    if(options[STAT_DIST_PROG_ARG_P_STAT].count()>0) {
        opts.p_stat = options[STAT_DIST_PROG_ARG_P_STAT].arg;
    }
    if(options[STAT_DIST_PROG_ARG_TEST_P_STAT].count()>0) {
        opts.test_p_stat = options[STAT_DIST_PROG_ARG_TEST_P_STAT].arg;
    }
    if(options[STAT_DIST_PROG_ARG_OUT_JSON].count()>0) {
        opts.output = options[STAT_DIST_PROG_ARG_OUT_JSON].arg;
    }
    if(((opts.test_spikes.empty())&&(!opts.test_p_stat.empty()))||((!opts.test_spikes.empty())&&(opts.test_p_stat.empty()))) {
        cerr << "Need test spikes and test p stat be pointed simultaniously\n";
        terminate();
    }
    return opts;
}



void p_stat_dist(int argc, char **argv) {
    PStatDistanceOpts opts = parsePStatDistOpts(argc, argv);

    vector<NeuronStat*> st;
    {
        ProtoRw prw(opts.p_stat, ProtoRw::Read);
        st = prw.readAll<NeuronStat>();
    }
    vector<NeuronStat*> test_st;
    if(!opts.test_p_stat.empty()) {
        ProtoRw prw(opts.test_p_stat, ProtoRw::Read);
        test_st = prw.readAll<NeuronStat>();   
    }

    LabeledSpikesList* sp_list;
    {
        ProtoRw prw(opts.spikes, ProtoRw::Read);
        sp_list = prw.read<LabeledSpikesList>();
    }
    
    
    LabeledSpikesList *test_sp_list = nullptr;
    if(!opts.test_spikes.empty()) {
        ProtoRw prw(opts.test_spikes, ProtoRw::Read);
        test_sp_list = prw.read<LabeledSpikesList>();
    }

    vector<IndexSlice> patterns = sp_list->getPatternSlices();
    vector<IndexSlice> test_patterns;
    if(test_sp_list) {
        test_patterns = test_sp_list->getPatternSlices();
    }
    
    DoubleMatrix dist = calcPStatDistance(st, test_st, patterns, test_patterns, opts.jobs);

    JsonBox::Value out;
    
    const SpikesList &sl = sp_list->sl;
    JsonBox::Array rates;
    for(size_t i = sl.N-1; i>=(sl.N-st.size()) ; i--) {
        rates.push_back(1000*sl[i].size()/sp_list->ptl.Tmax);        
    }
    out["distance_matrix"] = dist.serializeToJson();
    out["rates"] = rates;

    vector<string> labs = sp_list->ptl.getLabelsTimeline();
    JsonBox::Array labs_json;
    for(auto it=labs.begin(); it != labs.end(); ++it) {
        labs_json.push_back(*it);
    }
    out["labels"] = labs_json;
    if(test_sp_list) {
        vector<string> test_labs = test_sp_list->ptl.getLabelsTimeline();
        JsonBox::Array test_labs_json;
        for(auto it=test_labs.begin(); it != test_labs.end(); ++it) {
            test_labs_json.push_back(*it);
        }
        out["test_labels"] = test_labs_json;
    }
    

    if(!opts.output.empty()) {
        out.writeToFile(opts.output);
    } else {
        cout << out;
    }

}
