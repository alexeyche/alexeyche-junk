

#include "constants.h"

#include <snnlib/config/type_maps.h>

string findBaseStructName(map_type &tmap, string deriv_struct_name) {
    for(auto it=tmap.begin(); it != tmap.end(); ++it) {
        string base_struct_name = it->first;
        if(deriv_struct_name.substr(0, base_struct_name.size()) == base_struct_name) {
            return base_struct_name;
        }
    }
    cerr << "Unrecognized typename: " << deriv_struct_name << "\n";
    terminate();
}


#define PARSE_CONST_STRUCTURE(name) {                                               \
    const JsonBox::Object &inner_o = it->second.getObject();                        \
    for(auto inner_it = inner_o.begin(); inner_it != inner_o.end(); inner_it++) {   \
        string struct_name = inner_it->first;                                        \
        string base_struct_name = struct_name; \
        const JsonBox::Value &v = inner_it->second;                                 \
        auto it = tmap.find(struct_name); \
        if(it == tmap.end()) { \
            base_struct_name = findBaseStructName(tmap, struct_name);\
        }\
        name[struct_name] = unique_ptr<const_element_t>(tmap[base_struct_name]()); \
        name[struct_name]->fill_structure(v);                                \
    }                                                                               \
}\

Constants::Constants(string filename) {
    map_type tmap = generateMapType();

    JsonBox::Value v;
	v.loadFromFile(filename);
    if(v.isObject()) {
        const JsonBox::Object &o = v.getObject();
        for(auto it = o.begin(); it != o.end(); it++) {
            const string &const_field = it->first;
            if(const_field == "input_layers") {
                PARSE_CONST_STRUCTURE(input_layers)
            } else 
            if(const_field == "net_layers") {
                PARSE_CONST_STRUCTURE(net_layers)
            } else 
            if(const_field == "synapses") {
                PARSE_CONST_STRUCTURE(synapses)
            } else 
            if(const_field == "prob_funcs") {
                PARSE_CONST_STRUCTURE(prob_funcs)
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

