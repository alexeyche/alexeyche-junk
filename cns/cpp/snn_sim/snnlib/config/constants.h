#pragma once

#include <map>
#include <memory>
#include <vector>

#include <snnlib/util/matrix.h>
#include <snnlib/core.h>


template<typename T> Base * createInstance() { return new T; }

typedef map<std::string, Base*(*)()> map_type;


struct net_layers_t {
};

struct input_layers_t {
};

struct IaFLayerC: net_layers_t {
    double tau_refr;
    double amp_refr;
    double u_rest;
};

struct synapses_t {
    bool stochastic;
};

struct Synapse : synapses_t {
    double epsp_delay;
    double amp;
};

struct prob_funcs_t {
};

struct Determ : prob_funcs_t {
    double u_tr;
};

struct ExpHennequin : prob_funcs_t {
    double u_tr;
    double gain_factor;
    double p_rest;
    double beta;
    double r0;
};

struct learning_rules_t {
};

struct OptimalSTDP {
    double tau_c;
    double mean_p_dur;
    double target_rate;
    double target_rate_factor;
    double weight_decay;
};

struct sim_configuration_t {
    vector<size_t> input_sizes;
    vector<size_t> layers_sizes;
    Matrix<double> conn_matrix;
    Matrix<double> inh_frac_matrix;

    vector<string> input_layers;
    vector<string> net_layers;
    vector<string> learning_rules;
    vector<string> prob_funcs;
};

struct Constants {
    map<string, unique_ptr<net_layers_t> > net_layers;
    map<string, unique_ptr<input_layers_t> > input_layers;
    map<string, unique_ptr<synapses_t> > synapses;
    map<string, unique_ptr<prob_funcs_t> > prob_funcs;
    map<string, unique_ptr<learning_rules_t> > learning_rules;
    
    sim_configuration_t sim_conf;
};

Constants parseConstants(string filename);

