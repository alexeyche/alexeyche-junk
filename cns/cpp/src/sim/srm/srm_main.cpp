


#include <sim/core.h>
#include <sim/data/save.h>
#include <sim/data/load.h>
#include <sim/util/confParser.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "cfg.h"
#include "sim.h"
#include "connections.h"
#include "research.h"

#include "util.h"

using namespace srm;

PROGRAM_INFO("SRM SIM", "Spike Responce Model simulator"); 
PARAM_INT("seed", "seed", "", time(NULL));
PARAM_DOUBLE("rate", "learning rate", "r", 1);
PARAM_DOUBLE("simtime", "time to simulate(ms)", "t", 1100);
PARAM_STRING("mode", "modes for srm sim: \'run\', \'train\', \'stdp\'", "m", "run");
PARAM_INT("epoch", "Num of epochs in learning mode", "e", 1);
PARAM_STRING("stimuli-set", "Set of stimuli from config file", "s","train_set");
PARAM_STRING("pattern", "specific pattern", "p", "");
PARAM_DOUBLE("pattdur", "specific pattern duration", "d", 500);

PARAM_STRING("stimuli-dir", "directory with input patterns", "i", "stimuli");
PARAM_STRING("responces-dir", "directory with output patterns", "o", "responces");
PARAM_STRING("config-file", "config file", "c", "config.cfg");

int main(int argc, char** argv)
{
    CLI::ParseCommandLine(argc, argv);
	const int seed = CLI::GetParam<int>("seed");   
    const std::string mode = CLI::GetParam<std::string>("mode");
    const std::string specpattern = CLI::GetParam<std::string>("pattern");
    const double specpattdur = CLI::GetParam<double>("pattdur");

    const double learning_rate = CLI::GetParam<double>("rate");
    double simtime = CLI::GetParam<double>("simtime");
    const int epoch = CLI::GetParam<int>("epoch");
    const std::string stimset =  CLI::GetParam<std::string>("stimuli-set");  
    const std::string cfgFile =  CLI::GetParam<std::string>("config-file");  
    if(!fileExist(cfgFile)) {
        throw SrmException("Config file doesn't exists.\n");
    }

    TConfig c = parseStimCfg(cfgFile);
    c.print();
    std::string modelFName = c.models_dir+std::string("/") + formModelName(c, learning_rate);
    if(!dirExist(c.stimuli_dir)) {
        throw SrmException("Stimuli dir doesn't exists");
    }
    if(!dirExist(c.responces_dir)) {
        mkdir(c.responces_dir.c_str(),S_IRWXU);
        Log::Info << "Responce dir doesn't exists. Making one\n";
    }
    if(!dirExist(c.models_dir)) {
        mkdir(c.models_dir.c_str(),S_IRWXU);
        Log::Info << "Models dir doesn't exists. Making one\n";
    }

    std::srand(seed);
    
    Sim s(learning_rate);
    SrmNeuronGroup g(c.nneurons_out);
    mat start_w(g.size(),g.size(), fill::zeros);
    start_w.fill(c.neurons_out_start_weight);
    connect(&g, &g, TConnType::AllToAll, start_w);


    TimeSeriesGroup tsg(c.nneurons_in, 200*ms, 100); 
    if(specpattern == "") {
        std::map<std::string, TStimSet>::iterator it_ss = c.sets.ss.find(stimset);
        if(it_ss == c.sets.ss.end()) {
            Log::Fatal << "Can't find stimuli set " << stimset << "\n";
            throw SrmException("");
        }
        TStimSet &ss = it_ss->second;
        TClassSet::const_iterator it = ss.cs.begin();
        while(it != ss.cs.end()) {
            const std::vector<std::string> &files = it->second;
            Log::Info << "Loading set of class " << it->first << ":\n"; 
            for(size_t fi=0; fi<files.size(); fi++) {
                Log::Info << "  | " << std::string(c.stimuli_dir)+"/"+files[fi] << "\n";
                tsg.loadPatternFromFile(std::string(c.stimuli_dir)+"/"+files[fi], c.sets.duration*ms, 1);
            }                
            it++;
        }
        simtime = 0;
        for(size_t pi=0; pi<tsg.patterns.size(); pi++) {
            simtime += tsg.patterns[pi].pattDur + tsg.refrTime;
        }
    } else {
        tsg.loadPatternFromFile(specpattern, specpattdur*ms, 1);
    }

    mat start_w_ff(tsg.size(),g.size(), fill::zeros);
    start_w_ff.fill(c.neurons_in_start_weight);
    connect(&tsg, &g, TConnType::FeedForward, start_w_ff);
                
//    for(size_t ni=0; ni<g.size(); ni++) {
//        Log::Info << "neuron " << g.group[ni]->id() << " is connected to : ";    
//        for(size_t wi=0; wi<g.group[ni]->w.size(); wi++) {
//            Log::Info << g[ni]->in[wi]->id() << ";w=" << g.group[ni]->w[wi] << ", ";
//        }
//        Log::Info << "\n";
//    }   
//    return 0;
    s.addNeuronGroup(&tsg);
    s.addNeuronGroup(&g);
//    s.addStatListener(&g);
    TRunType::ERunType rt;   
    if(mode == "run") {    
        rt = TRunType::Run;
        mat saved_weights(g.size(), g.group[0]->w.size());
        if (fileExist(modelFName)) {
            data::Load(modelFName, saved_weights);
            for(size_t ni=0; ni<g.group.size(); ni++) {
                SrmNeuron *n =  dynamic_cast<SrmNeuron*>(g.group[ni]);
                if(n) {
                    Log::Info << "neuron " << ni << "\n";
                    for(size_t wi=0; wi<n->w.size(); wi++) { Log::Info << saved_weights(ni,wi) << ", "; n->w[wi] = saved_weights(ni,wi); }
                    Log::Info << "\n";
                }            
            }
        } else {
            Log::Fatal << "Model file " << modelFName << " doesn't exists\n";
            throw SrmException("");
        }

    } 
    if(mode == "stdp") {    
        rt = TRunType::RunAndLearnSTDP;
    } 
    if(mode == "train") {    
        rt = TRunType::RunAndLearnLogLikelyhood;
    }        
    vec weights(g.group.size(), g.group[0]->w.size(), fill::zeros);
    for(size_t ep=0; ep<epoch; ep++) {
        for(size_t ni=0; ni<g.group.size(); ni++) { g.group[ni]->y.clean(); }
        bool send=false;
        if((ep == epoch-1)||(ep == 0)) {
            send=true;
        }
        s.run(simtime*ms, 0.5, rt, true, send);
        Log::Info << "weight after:\n";
        for(size_t ni=0; ni<g.group.size(); ni++) {
            SrmNeuron *n =  dynamic_cast<SrmNeuron*>(g.group[ni]);
            if(n) {
                Log::Info << "neuron " << ni << "\n";
                for(size_t wi=0; wi<n->w.size(); wi++) { Log::Info << n->w[wi] << ", "; weights(ni,wi) = n->w[wi]; }
                Log::Info << "\n";
            }            
        }        
        if((rt == TRunType::Run)&&(specpattern != "")) { 
            std::vector<std::string> filename_spl = split(specpattern, '/');
            std::string basename = filename_spl.back();
            std::vector<std::string> basename_spl = split(basename, '.');
            data::Save(c.responces_dir+"/"+basename_spl.front()+std::string("_ep_")+std::to_string(ep)+std::string("_resp.csv"), s.raster, false, false);
            std::srand(seed+ep);
        }            
//        std::vector<mat> responces;
//        for(size_t ti=0; ti<s.raster.n_cols; ti++) {
//            for(size_t pi=0; pi<tsg.patterns.size(); pi++) {
//                uvec patt_firing = find(s.raster.row(ti) >= patterns[pi].dedicatedTime.first);
//            }
//        }            
    }        
    if((rt == TRunType::RunAndLearnSTDP)||(rt == TRunType::RunAndLearnLogLikelyhood)) {
        data::Save(modelFName, weights);
    }        
    send_arma_mat(weights, "weights");
}

