#pragma once

#include <map>
#include <memory>
#include <vector>

#include <snnlib/util/matrix.h>
#include <snnlib/core.h>
#include <snnlib/util/json/json_box.h>



class const_element_t {
protected:
    virtual void print(std::ostream& str) const = 0;
public:
    virtual void fill_structure(JsonBox::Value v) = 0;
    friend std::ostream& operator<<(std::ostream& str, const_element_t const& data) {
        data.print(str);
        return str;
    }
};


class IaFLayerC: public const_element_t {
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

class SigmaTCLayerC: public const_element_t {
public:
    double sigma;

    void fill_structure(JsonBox::Value v) {
        sigma = v["sigma"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "sigma: " << sigma << "\n";
    }
};


class SynapseC : public const_element_t {
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


class DetermC : public const_element_t {
public:
    double u_tr;

    void fill_structure(JsonBox::Value v) {
        u_tr = v["u_tr"].getDouble();
    }
    void print(std::ostream &str) const {
        str << "u_tr: " << u_tr << "\n";
    }

};

class ExpHennequinC : public const_element_t {
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


class OptimalStdpC: public const_element_t {
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

class SimConfiguration: const_element_t {
public:
    vector<size_t> input_sizes;
    vector<size_t> layers_sizes;
    Matrix<double> conn_matrix;
    Matrix<double> inh_frac_matrix;

    vector<string> input_layers;
    vector<string> net_layers;
    vector<string> learning_rules;
    vector<string> prob_funcs;
    
    void fill_structure(JsonBox::Value v) {
        auto a = v["input_sizes"].getArray();
        for(auto it=a.begin(); it!=a.end(); ++it) {
            input_sizes.push_back(it->getInt());
        }
    }
    void print(std::ostream &str) const {
        std::copy(input_sizes.begin(), input_sizes.end(), ostream_iterator<size_t>(str, " "));
//        str << "input_sizes: " << input_sizes;
    }    
//    friend std::ostream& operator<<(std::ostream& str, Constants const& data) {
//        str << "input_sizes: " << data.input_sizes;
//        return str;
//    }
};

typedef map<string, unique_ptr<const_element_t> > constants_map;



class Constants {
public:    
    constants_map net_layers;
    constants_map input_layers;
    constants_map synapses;
    constants_map prob_funcs;
    constants_map learning_rules;
    
    SimConfiguration sim_conf;
    static void print_constants_map(const constants_map &m) {
        for(auto it = m.cbegin(); it != m.cend(); ++it ) { 
            cout << it->first << " == " << *it->second; 
        }
    }
    friend std::ostream& operator<<(std::ostream& str, Constants const& data) {
        print_constants_map(data.net_layers);
        print_constants_map(data.input_layers);
        print_constants_map(data.synapses);
        print_constants_map(data.prob_funcs);
        print_constants_map(data.learning_rules);
//        str << sim_conf;
        return str;
    }
};

template<typename T> const_element_t * createInstance() { return new T; }

typedef map<string, const_element_t*(*)()> map_type;

map_type generateMapType() {
    map_type map;
    
    map["IaFLayer"]     =   &createInstance<IaFLayerC>;
    map["Synapse"]      =   &createInstance<SynapseC>;
    map["Determ"]       =   &createInstance<DetermC>;
    map["ExpHennequin"] =   &createInstance<ExpHennequinC>;
    map["OptimalStdp"]  =   &createInstance<OptimalStdpC>;
    map["SigmaTCLayer"]  =  &createInstance<SigmaTCLayerC>;

    return map;
}

Constants parseConstants(string filename);

