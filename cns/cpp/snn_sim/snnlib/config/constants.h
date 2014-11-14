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


class SRMLayerC: public ConstObj {
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

class SigmaTCLayerC: public ConstObj {
public:
    double sigma;

    void fill_structure(JsonBox::Value v) {
        sigma = v["sigma"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "sigma: " << sigma << "\n";
    }
};


class SynapseC : public ConstObj {
public:
    double epsp_delay;
    double amp;

    void fill_structure(JsonBox::Value v) {
        epsp_delay = v["epsp_delay"].getDouble();
        amp = v["amp"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "epsp_delay: " << epsp_delay << ", " << "amp: " << amp << "\n"; 
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

class InputLayersConf : public ConstObj {
public:    
    size_t size;
    string type;
    string act_func;
    void fill_structure(JsonBox::Value v) {
        size = v["size"].getInt();
        type = v["type"].getString();
        act_func = v["act_func"].getString();
    }
    void print(std::ostream &str) const {
        str << "InputLayersConf(size: " << size << ", type: " << type <<  ", act_func: " << act_func << ")";
    }
};

class NetLayersConf : public ConstObj {
public:    
    size_t size;
    string type;
    string learning_rule;
    string act_func;
    void fill_structure(JsonBox::Value v) {
        size = v["size"].getInt();
        type = v["type"].getString();
        learning_rule = v["learning_rule"].getString();
        act_func = v["act_func"].getString();
    }
    void print(std::ostream &str) const {
        str << "NetLayersConf(size: " << size << ", type: " << type << ", learning_rule: " << learning_rule << ", act_func: " << act_func << ")";
    }
};

class ConnectionMap: public ConstObj {
    class ConnectionConf: public ConstObj {
    public:
        double prob;
        double weight;
        string type;

        void fill_structure(JsonBox::Value v) {
            prob = v["prob"].getDouble();
            type = v["type"].getString();
            weight = v["weight"].getDouble();
        }
        void print(std::ostream &str) const {
            str << "ConnectionConf(" << "prob: " << prob << ", weight: " << weight << ", type: " << type << ")";
        }
    };
public:
    map< pair<size_t, size_t>, vector<ConnectionConf> > conn_map;

    void fill_structure(JsonBox::Value v) {
        JsonBox::Object o = v.getObject();
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
    }   

    void print(std::ostream &str) const {
        for(auto it=conn_map.begin(); it!=conn_map.end(); ++it) {
            pair<size_t,size_t> aff = it->first;
            str << aff.first << "-" << aff.second << ":\n"; 
            print_vector<ConnectionConf>(it->second, str, "\n");
        }
    }
};



class SimConfiguration: public ConstObj {
public:
    vector<InputLayersConf> input_layers_conf;
    vector<NetLayersConf> net_layers_conf;
    
    ConnectionMap conn_map;
    
    void fill_structure(JsonBox::Value v) {
        auto a_input_sizes = v["input_layers_conf"].getArray();      
        for(auto it=a_input_sizes.begin(); it!=a_input_sizes.end(); ++it) { 
            JsonBox::Value v = *it;
            InputLayersConf conf;
            conf.fill_structure(v);
            input_layers_conf.push_back(conf);
        }
        auto a_net_sizes = v["net_layers_conf"].getArray();      
        for(auto it=a_net_sizes.begin(); it!=a_net_sizes.end(); ++it) { 
            JsonBox::Value v = *it;
            NetLayersConf conf;
            conf.fill_structure(v);
            net_layers_conf.push_back(conf);
        }

        conn_map.fill_structure(v["conn_map"]);
    }
    

    void print(std::ostream &str) const {
        str << "input_layers_conf: \n";  print_vector<InputLayersConf>(input_layers_conf, str, ",\n");
        str << "net_layers_conf: \n"; print_vector<NetLayersConf>(net_layers_conf, str, ",\n");
        str << "conn_map: \n";       str << conn_map;
    }    
};


typedef map<string, shared_ptr<const ConstObj> > const_map;

class Constants {
public:    
    Constants(string filename);

    const_map net_layers;
    const_map input_layers;
    const_map synapses;
    const_map act_funcs;
    const_map learning_rules;
        
    SimConfiguration sim_conf;

    const ConstObj* operator[](const string &key) const {
        if(net_layers.count(key)) return net_layers.at(key).get();
        if(input_layers.count(key)) return input_layers.at(key).get();
        if(synapses.count(key)) return synapses.at(key).get();
        if(act_funcs.count(key)) return act_funcs.at(key).get();
        if(learning_rules.count(key)) return learning_rules.at(key).get();

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
        print_constants_map(data.net_layers);
        print_constants_map(data.input_layers);
        print_constants_map(data.synapses);
        print_constants_map(data.act_funcs);
        print_constants_map(data.learning_rules);
        str << "\n== Sim Configuration ==\n";
        str << data.sim_conf;
        return str;
    }
};


