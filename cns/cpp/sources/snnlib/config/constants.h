#pragma once

#include <snnlib/util/matrix.h>
#include <snnlib/core.h>
#include <snnlib/util/json/json_box.h>
#include <snnlib/base.h>
#include <snnlib/util/util.h>

class ConstObj: public Entity {
public:
    virtual void fill_structure(JsonBox::Value v) = 0;
};




class SRMNeuronC: public ConstObj {
public:
    double tau_refr;
    double amp_refr;
    double u_rest;

    void fill_structure(JsonBox::Value v) {
        tau_refr = v["tau_refr"].getDouble();
        amp_refr = v["amp_refr"].getDouble();
        u_rest = v["u_rest"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "tau_refr: " << tau_refr << ", amp_refr: " << amp_refr << ", u_rest: " << u_rest <<"\n";
    }
};

class AdExNeuronC: public ConstObj {
public:
    double C;
    double t_ref;
    double gL;
    double EL;
    double slope;
    double tau_a;
    double a;
    double b;
    double u_rest;
    double u_tr;

    void fill_structure(JsonBox::Value v) {
        C = v["C"].getDouble();
        t_ref = v["t_ref"].getDouble();
        gL = v["gL"].getDouble();
        EL = v["EL"].getDouble();
        slope = v["slope"].getDouble();
        tau_a = v["tau_a"].getDouble();
        a = v["a"].getDouble();
        b = v["b"].getDouble();
        u_rest = v["u_rest"].getDouble();
        u_tr = v["u_tr"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "C: " << C;
        str << ", t_ref: " << t_ref;
        str << ", gL: " << gL;
        str << ", EL: " << EL;
        str << ", slope: " << slope;
        str << ", tau_a: " << tau_a;
        str << ", a: " << a;
        str << ", b: " << b;
        str << ", u_rest: " << u_rest;
        str << ", u_tr: " << u_tr;
        str << "\n";
    }
};


pair<double,double> readLowAndHigh(JsonBox::Array a);
void printDoublePair(pair<double,double> p, std::ostream &str);

class SigmaTuningCurveC: public ConstObj {
public:
    pair<double,double> sigma;
    pair<double,double> intercept;
    pair<double,double> gain;

    void fill_structure(JsonBox::Value v) {
        sigma = readLowAndHigh(v["sigma"].getArray());
        intercept = readLowAndHigh(v["intercept"].getArray());
        gain = readLowAndHigh(v["gain"].getArray());
    }
    void print(std::ostream &str) const {
        str << "sigma: "; printDoublePair(sigma, str);
        str << ", intercept: "; printDoublePair(intercept, str);
        str << ", gain: "; printDoublePair(gain, str);
        str << "\n";
    }
};


class SynapseC : public ConstObj {
public:
    double epsp_decay;
    double amp;

    void fill_structure(JsonBox::Value v) {
        epsp_decay = v["epsp_decay"].getDouble();
        amp = v["amp"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "epsp_decay: " << epsp_decay << ", " << "amp: " << amp << "\n";
    }

};


class DetermC : public ConstObj {
public:
    double u_tr;

    void fill_structure(JsonBox::Value v) {
        u_tr = v["u_tr"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "u_tr: " << u_tr << "\n";
    }

};

class ExpHennequinC : public ConstObj {
public:
    double u_tr;
    double gain_factor;
    double p_rest;
    double beta;
    double r0;

    void fill_structure(JsonBox::Value v) {
        u_tr        = v["u_tr"].getDouble();
        gain_factor = v["gain_factor"].getDouble();
        p_rest      = v["p_rest"].getDouble();
        beta        = v["beta"].getDouble();
        r0          = v["r0"].getDouble();
    }
    void print(std::ostream &str) const {
        str <<
            "u_tr: "        <<       u_tr << ", " <<
            "gain_factor: " <<       gain_factor << ", " <<
            "p_rest: "      <<       p_rest << ", " <<
            "beta: "        <<       beta << ", " <<
            "r0: "          <<       r0 << "\n";

    }
};


class OptimalStdpC: public ConstObj {
public:
    double tau_c;
    double mean_p_dur;
    double target_rate;
    double target_rate_factor;
    double weight_decay;

    void fill_structure(JsonBox::Value v) {
        tau_c               = v["tau_c"].getDouble();
        mean_p_dur          = v["mean_p_dur"].getDouble();
        target_rate         = v["target_rate"].getDouble();
        target_rate_factor  = v["target_rate_factor"].getDouble();
        weight_decay        = v["weight_decay"].getDouble();
    }
    void print(std::ostream &str) const {
        str <<
        "tau_c: " << tau_c << ", " <<
        "mean_p_dur: " << mean_p_dur << ", " <<
        "target_rate: " << target_rate << ", " <<
        "target_rate_factor: " << target_rate_factor << ", " <<
        "weight_decay: " << weight_decay << "\n";
    }
};

class NeuronConf : public ConstObj {
public:
    string neuron;
    string act_func;
    string tuning_curve;
    string learning_rule;
    double axon_delay_gain;
    double axon_delay_rate;

    void fill_structure(JsonBox::Value v) {
        act_func = v["act_func"].getString();
        tuning_curve = v["tuning_curve"].getString();
        learning_rule = v["learning_rule"].getString();
        neuron = v["neuron"].getString();
        JsonBox::Array a = v["axon_delay_distr"].getArray();
        axon_delay_gain = a[0].getDouble();
        axon_delay_rate = a[1].getDouble();
    }
    void print(std::ostream &str) const {
        str << "NeuronConf(neuron: " << neuron << ", learning_rule: "  << learning_rule << ", tuning_curve : " << tuning_curve <<  ", act_func: " << act_func <<
            ", axon_delay_gain: " << axon_delay_gain  << ", axon_delay_rate: " << axon_delay_rate << ")";
    }
};

class LayerConf : public ConstObj {
public:
    size_t size;

    NeuronConf nconf;

    void fill_structure(JsonBox::Value v) {
        size = v["size"].getInt();
        nconf.fill_structure(v["neuron_conf"]);
    }
    void print(std::ostream &str) const {
        str << "LayerConf(size: " << size << ", " << nconf << ")";
    }
};


class ConnectionConf: public ConstObj {
public:
    double prob;
    double weight_per_neuron;
    string type;
    double dendrite_delay_gain;
    double dendrite_delay_rate;


    void fill_structure(JsonBox::Value v) {
        prob = v["prob"].getDouble();
        type = v["type"].getString();
        weight_per_neuron = v["weight_per_neuron"].getDouble();
        JsonBox::Array ad = v["dendrite_delay_distr"].getArray();
        dendrite_delay_gain = ad[0].getDouble();
        dendrite_delay_rate = ad[1].getDouble();
    }

    void print(std::ostream &str) const {
        str << "ConnectionConf(" << "prob: " << prob << ", weight_per_neuron: " << weight_per_neuron << ", type: " <<   type <<
            ", dendrite_delay_gain: " << dendrite_delay_gain  << ", dendrite_delay_rate: " << dendrite_delay_rate << ")";

    }
};

class TimeSeriesMapConf : public ConstObj {
public:
    double dt;
    void print(std::ostream &str) const {
        str << "dt : " << dt;
    }
    void fill_structure(JsonBox::Value v) {
        dt = v["dt"].getDouble();
    }
};
class SimRunConf: public ConstObj {
public:
    double dt;
    int seed;

    void fill_structure(JsonBox::Value v) {
        dt = v["dt"].getDouble();
        seed = v["seed"].getInt();
    }

    void print(std::ostream &str) const {
        str << "dt: " << dt << " seed: " << seed << "\n";
    }
};



typedef map< pair<size_t, size_t>, vector<ConnectionConf> > ConnectionMap;

class SimConfiguration: public ConstObj {
public:
    vector<LayerConf> input_layers_conf;
    vector<LayerConf> net_layers_conf;

    ConnectionMap conn_map;
    TimeSeriesMapConf ts_map_conf;
    vector<size_t> neurons_to_listen;
    SimRunConf sim_run_c;

    void fill_structure(JsonBox::Value v) {
        auto a_input_sizes = v["input_layers_conf"].getArray();
        for(auto it=a_input_sizes.begin(); it!=a_input_sizes.end(); ++it) {
            JsonBox::Value v = *it;
            LayerConf conf;
            if(v["neuron_conf"]["tuning_curve"].getString().empty()) {
                cerr << "Input layer must have a tuning curve\n";
                terminate();
            }
            conf.fill_structure(v);
            input_layers_conf.push_back(conf);
        }
        auto a_net_sizes = v["net_layers_conf"].getArray();
        for(auto it=a_net_sizes.begin(); it!=a_net_sizes.end(); ++it) {
            JsonBox::Value v = *it;
            LayerConf conf;
            conf.fill_structure(v);
            net_layers_conf.push_back(conf);
        }
        JsonBox::Object o = v["conn_map"].getObject();
        for(auto it=o.begin(); it!=o.end(); ++it) {
            vector<string> aff = split(it->first, '-');
            if(aff.size() != 2) {
                cerr << "conn_map configuration not right: need 2 afferent layers separated by \"-\"\n";
                terminate();
            }
            size_t aff_pre = stoi(aff[0].c_str());
            size_t aff_post = stoi(aff[1].c_str());
            pair<size_t, size_t> aff_p(aff_pre, aff_post);

            vector<ConnectionConf> conn_conf_vec;
            JsonBox::Array conn_array = it->second.getArray();
            for(auto conn_it=conn_array.begin(); conn_it != conn_array.end(); ++conn_it) {
                ConnectionConf conn_conf;
                conn_conf.fill_structure(*conn_it);
                conn_conf_vec.push_back(conn_conf);
            }
            if(conn_conf_vec.size() == 0) {
                cerr << "undefined conn configuration at " << it->first << "\n";
                terminate();
            }
            conn_map[aff_p] = conn_conf_vec;
        }
        ts_map_conf.fill_structure(v["time_series_map_conf"]);
        JsonBox::Array a = v["neurons_to_listen"].getArray();
        for(auto it=a.begin(); it!=a.end(); ++it) {
            neurons_to_listen.push_back(it->getInt());
        }
        sim_run_c.fill_structure(v["sim_run_conf"]);
    }


    void print(std::ostream &str) const {
        str << "input_layers_conf: \n";  print_vector<LayerConf>(input_layers_conf, str, ",\n");
        str << "net_layers_conf: \n"; print_vector<LayerConf>(net_layers_conf, str, ",\n");
        for(auto it=conn_map.begin(); it!=conn_map.end(); ++it) {
            pair<size_t,size_t> aff = it->first;
            str << aff.first << "-" << aff.second << ":\n";
            print_vector<ConnectionConf>(it->second, str, "\n");
        }
        str << "time_series_map_conf: " << ts_map_conf << "\n";
        str << "sim_run_conf: " << sim_run_c << "\n";
        str << "neurons_to_listen: "; print_vector<size_t>(neurons_to_listen, str, ","); str << "\n";
    }
};




typedef map<string, const ConstObj *> const_map;

class Constants {
public:
    Constants(string filename);

    const_map neurons;
    const_map layers;

    const_map globals;
    const_map tuning_curves;
    const_map synapses;
    const_map act_funcs;
    const_map learning_rules;

    SimConfiguration sim_conf;

    static string blank_prefix;

    const ConstObj *operator[](const string &key) const {
        if(globals.count(key)) return globals.at(key);
        if(neurons.count(key)) return neurons.at(key);
        if(layers.count(key)) return layers.at(key);
        if(tuning_curves.count(key)) return tuning_curves.at(key);
        if(synapses.count(key)) return synapses.at(key);
        if(act_funcs.count(key)) return act_funcs.at(key);
        if(learning_rules.count(key)) return learning_rules.at(key);

        if(key.substr(0, blank_prefix.size()) == blank_prefix) { // starts with Blank -- ignore
            return nullptr;
        }
        cerr << "Couldn't find instance with key in constants: " << key << "\n";
        terminate();
    }

    static void print_constants_map(const const_map &m) {
        for(auto it = m.cbegin(); it != m.cend(); ++it ) {
            cout << it->first << " == " << *it->second;
        }
    }
    friend std::ostream& operator<<(std::ostream& str, Constants const& data) {
        str << "== Sim Constants ==\n";
        print_constants_map(data.globals);
        print_constants_map(data.neurons);
        print_constants_map(data.layers);
        print_constants_map(data.tuning_curves);
        print_constants_map(data.synapses);
        print_constants_map(data.act_funcs);
        print_constants_map(data.learning_rules);
        str << "\n== Sim Configuration ==\n";
        str << data.sim_conf;
        return str;
    }
};


