#pragma once

#include <snnlib/util/matrix.h>
#include <snnlib/core.h>
#include <snnlib/util/json/json_box.h>
#include <snnlib/base.h>
#include <snnlib/util/util.h>
#include <snnlib/serialize/serialize.h>
#include <snnlib/protos/model.pb.h>

class ConfObj: public Entity {
public:
    virtual void fill_structure(JsonBox::Value v) = 0;
};


class ConstObj: public Entity {
public:
    ConstObj(string _name) : name(_name) {}
    virtual void fill_structure(JsonBox::Value v) = 0;
    virtual const string& getName() const { return name; }
private:
    string name;
};





class SRMNeuronC: public ConstObj {
public:
    SRMNeuronC(string name) : ConstObj(name) {}
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
    AdExNeuronC(string name) : ConstObj(name) {}
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
    SigmaTuningCurveC(string name) : ConstObj(name) {}
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
    SynapseC(string name) : ConstObj(name) {}
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
    DetermC(string name) : ConstObj(name) {}
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
    ExpHennequinC(string name) : ConstObj(name) {}
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
    OptimalStdpC(string name) : ConstObj(name) {}
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

class MaxLikelihoodC: public ConstObj {
public:
    MaxLikelihoodC(string name) : ConstObj(name) {}
    double tau_el;
    bool input_target;
    double learning_rate;

    void fill_structure(JsonBox::Value v) {
        tau_el               = v["tau_el"].getDouble();
        input_target         = v["input_target"].getBoolean();
        learning_rate         = v["learning_rate"].getDouble();    
    }
    void print(std::ostream &str) const {
        str << "tau_el: " << tau_el << ", learning_rate: " << learning_rate << ", input_target: " << input_target << "\n";
    }
};

class LikelihoodC: public ConstObj {
public:
    LikelihoodC(string name) : ConstObj(name) {}
    bool input_target;

    void fill_structure(JsonBox::Value v) {
        input_target         = v["input_target"].getBoolean();
    }
    void print(std::ostream &str) const {
        str << "input_target: " << input_target << "\n";
    }
};

class NeuronConf : public ConfObj {
public:
    string neuron;
    string act_func;
    string tuning_curve;
    string learning_rule;
    string reward_modulation;
    double axon_delay_gain;
    double axon_delay_rate;

    void fill_structure(JsonBox::Value v) {
        act_func = v["act_func"].getString();
        tuning_curve = v["tuning_curve"].getString();
        learning_rule = v["learning_rule"].getString();
        reward_modulation = v["reward_modulation"].getString();
        neuron = v["neuron"].getString();
        JsonBox::Array a = v["axon_delay_distr"].getArray();
        axon_delay_gain = a[0].getDouble();
        axon_delay_rate = a[1].getDouble();
    }
    void print(std::ostream &str) const {
        str << "NeuronConf(neuron: " << neuron << ", learning_rule: "  << learning_rule << ", tuning_curve : " << tuning_curve <<  ", act_func: " << act_func <<
            ", axon_delay_gain: " << axon_delay_gain  << ", axon_delay_rate: " << axon_delay_rate << ", reward_modulation: " << reward_modulation <<  ")";
    }
};

class LayerConf : public ConfObj {
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


class ConnectionConf: public ConfObj {
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

class TimeSeriesMapConf : public ConfObj {
public:
    double dt;
    void print(std::ostream &str) const {
        str << "dt : " << dt;
    }
    void fill_structure(JsonBox::Value v) {
        dt = v["dt"].getDouble();
    }
};
class SimRunConf: public ConfObj {
public:
    double dt;
    int seed;
    double start_rate;

    void fill_structure(JsonBox::Value v) {
        dt = v["dt"].getDouble();
        seed = v["seed"].getInt();
        start_rate = v["start_rate"].getDouble();
    }

    void print(std::ostream &str) const {
        str << "dt: " << dt << " seed: " << seed << " start_rate: " << start_rate << "\n";
    }
};

class RewardModConf: public ConfObj {
public:
    double tau_rew;
    double tau_mean_rew;

    void fill_structure(JsonBox::Value v) {
        tau_rew = v["tau_rew"].getDouble();
        tau_mean_rew = v["tau_mean_rew"].getDouble();
    }

    void print(std::ostream &str) const {
        str << "tau_rew: " << tau_rew << " tau_mean_rew: " << tau_mean_rew << "\n";
    }
};

typedef map< pair<size_t, vector<size_t>>, vector<ConnectionConf> > ConnectionMap;
typedef map< pair<size_t, vector<size_t>>, RewardModConf> RewardConnectionMap;

class SimConfiguration: public ConfObj {
public:
    vector<LayerConf> input_layers_conf;
    vector<LayerConf> net_layers_conf;

    ConnectionMap conn_map;
    TimeSeriesMapConf ts_map_conf;
    vector<size_t> neurons_to_listen;
    vector<size_t> reward_layers_to_listen;
    SimRunConf sim_run_c;
    RewardConnectionMap reinforce_map;

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
            vector<string> aff = splitBySubstr(it->first, "->");
            if(aff.size() != 2) {
                cerr << "conn_map configuration not right: need 2 afferents separated by \"->\"\n";
                terminate();
            }

            pair<size_t, vector<size_t> > aff_p;

            aff_p.first = stoi(aff[0].c_str());
            aff_p.second = vector<size_t>();
            
            vector<string> aff_post = split(aff[1],',');
            for(auto it=aff_post.begin(); it != aff_post.end(); ++it) {
                aff_p.second.push_back(stoi(it->c_str()));    
            }
            

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
        JsonBox::Object o_rf = v["reinforce_map"].getObject();
        try {
            for(auto it=o_rf.begin(); it!=o_rf.end(); ++it) {
                vector<string> aff = splitBySubstr(it->first, "->");
                if(aff.size() != 2) {
                    cerr << "rew_map configuration not right: need afferent layers separated by \"->\"\n";
                    terminate();
                }
                pair<size_t, vector<size_t> > aff_p(stoi(aff[0].c_str()), vector<size_t>());
                vector<string> aff_post = split(aff[1], ',');
                
                for(auto p_it=aff_post.begin(); p_it != aff_post.end(); ++p_it) {
                    aff_p.second.push_back(stoi(p_it->c_str()));                        
                }
                RewardModConf mod_conf;
                mod_conf.fill_structure(it->second);
                reinforce_map[aff_p] = mod_conf;
            }
        } catch (...) {
            cerr << "Cannot parse reinforce map configuration\n";
            cerr << "You need to point one pre layer from left side of \"->\" and multiple (or one) post layer(s) from right side, separated by commas\n";
            cerr << "Reinforce map doesn't support multiple configuration for one connection (like Connection Map)\n";
            cerr << "\n";
            throw;
        }


        ts_map_conf.fill_structure(v["time_series_map_conf"]);
        JsonBox::Array a = v["neurons_to_listen"].getArray();
        for(auto it=a.begin(); it!=a.end(); ++it) {
            neurons_to_listen.push_back(it->getInt());
        }
        JsonBox::Array a_l = v["reward_layers_to_listen"].getArray();
        for(auto it=a_l.begin(); it!=a_l.end(); ++it) {
            reward_layers_to_listen.push_back(it->getInt());
        }
        sim_run_c.fill_structure(v["sim_run_conf"]);
    }


    void print(std::ostream &str) const {
        str << "input_layers_conf: \n";  print_vector<LayerConf>(input_layers_conf, str, ",\n");
        str << "net_layers_conf: \n"; print_vector<LayerConf>(net_layers_conf, str, ",\n");
        str << "conn_map: \n";
        for(auto it=conn_map.begin(); it!=conn_map.end(); ++it) {
            pair<size_t,vector<size_t>> aff = it->first;
            cout << aff.first << "->";
            print_vector<size_t>(aff.second, str, ", ");    
            str << "\t";
            print_vector<ConnectionConf>(it->second, str, "\n");
        }
        str << "reinforce_map: \n";
        for(auto it=reinforce_map.begin(); it!=reinforce_map.end(); ++it) {
            pair<size_t,vector<size_t>> aff = it->first;
            cout << aff.first << "->";
            print_vector<size_t>(aff.second, str, ", ");
            str << "\t" << it->second;
        }
        str << "time_series_map_conf: " << ts_map_conf << "\n";
        str << "sim_run_conf: " << sim_run_c << "\n";
        str << "neurons_to_listen: "; print_vector<size_t>(neurons_to_listen, str, ","); str << "\n";
    }
};




typedef map<string, const ConstObj *> const_map;

class Constants : public Serializable<Protos::Constants> {
    Constants() : Serializable(EConstants) {
        Serializable::init(EConstants);
        if(!Constants::glob_inst) {
            Constants::glob_inst = this;
        }
    }
    friend class Factory;
public:
    Constants(string filename) : Serializable(EConstants) {
        Serializable::init(EConstants);
        json_content = preprocessAndReadConstJson(filename);
        parse();
        if(!Constants::glob_inst) {
            Constants::glob_inst = this;
        }
    }

    void parse();
    static string preprocessAndReadConstJson(string filename);

    bool operator==(const Constants &c) const {
        if(c.json_content == json_content) {
            return true;
        }
        return false;
    }
    bool operator!=(const Constants &c) const {
        if( *this == c) return false;
        return true;
    }
    void deserialize() {
        Protos::Constants *c_serial = getSerializedMessage();
        json_content = c_serial->json_content();
        parse();
    }

    ProtoPack serialize() {
        Protos::Constants *c_serial = getNewMessage();
        c_serial->set_json_content(json_content);
        return ProtoPack({c_serial});
    }


    const_map neurons;
    const_map layers;

    const_map globals;
    const_map tuning_curves;
    const_map synapses;
    const_map act_funcs;
    const_map learning_rules;
    const_map reward_modulations;

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
        if(reward_modulations.count(key)) return reward_modulations.at(key);

        if(key.substr(0, blank_prefix.size()) == blank_prefix) { // starts with Blank -- ignore
            return nullptr;
        }
        cerr << "Couldn't find instance with key in constants: " << key << "\n";
        terminate();
    }

    static void print_constants_map(const const_map &m, std::ostream &str) {
        for(auto it = m.cbegin(); it != m.cend(); ++it ) {
            str << it->first << " == " << *it->second;
        }
    }
    void print(std::ostream& str) const {
        str << "== Sim Constants ==\n";
        print_constants_map(globals, str);
        print_constants_map(neurons, str);
        print_constants_map(layers, str);
        print_constants_map(tuning_curves, str);
        print_constants_map(synapses, str);
        print_constants_map(act_funcs, str);
        print_constants_map(learning_rules, str);
        str << "\n== Sim Configuration ==\n";
        str << sim_conf;
    }
    static const Constants globalInstance() {
        if(!glob_inst) {
            cerr << "Trying to access to non-initialized global constants instance\n";
            terminate();
        }
        return *glob_inst;
    }
    static bool IsGlobalInstanceCreated() {
        if(glob_inst) {
            return true;
        }
        return false;
    }

private:
    string json_content;

    static Constants *glob_inst;
};


