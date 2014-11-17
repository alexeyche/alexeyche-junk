

#include "constants.h"

#include <snnlib/config/factory.h>



#define PARSE_CONST_STRUCTURE(name) {                                               \
    const JsonBox::Object &inner_o = it->second.getObject();                        \
    for(auto inner_it = inner_o.begin(); inner_it != inner_o.end(); inner_it++) {   \
        string struct_name = inner_it->first;                                        \
        const JsonBox::Value &v = inner_it->second;                                 \
        name[struct_name] = shared_ptr<ConstObj>(Factory::inst().createConst(struct_name, v)); \
    }\
}\

Constants::Constants(string filename) {
    JsonBox::Value v;
	v.loadFromFile(filename);
    if(v.isObject()) {
        const JsonBox::Object &o = v.getObject();
        for(auto it = o.begin(); it != o.end(); it++) {
            const string &const_field = it->first;
            if(const_field == "tuning_curves") {
                PARSE_CONST_STRUCTURE(tuning_curves)
            } else 
            if(const_field == "neurons") {
                PARSE_CONST_STRUCTURE(neurons)
            } else 
            if(const_field == "layers") {
                PARSE_CONST_STRUCTURE(layers)
            } else 
            if(const_field == "synapses") {
                PARSE_CONST_STRUCTURE(synapses)
            } else 
            if(const_field == "act_funcs") {
                PARSE_CONST_STRUCTURE(act_funcs)
            } else 
            if(const_field == "learning_rules") {
                PARSE_CONST_STRUCTURE(learning_rules)
            } else 
            if(const_field == "sim_configuration") {
                JsonBox::Value v = it->second;
                sim_conf.fill_structure(v);                
            } else 
            {
                cerr << "Unknown constants field " << const_field << "\n";
                exit(1);
            }
                  
        }
    } else {
        cerr << "Failed to find main object in constants json file " << filename << "\n";
    }
}

pair<double,double> readLowAndHigh(JsonBox::Array a) {
    if(a.size() != 2) {
        cerr << "Bad values for intercept. Need two sized array for low and high values\n";
        terminate();
    }
    pair<double,double> ans;
    ans.first = a[0].getDouble();
    ans.second = a[1].getDouble();
    return ans;
}
void printDoublePair(pair<double,double> p, std::ostream &str) {
    str << "[" << p.first << "," << p.second << "]";
}