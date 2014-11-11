
#include <snnlib/util/json/json_box.h>


#include "constants.h"

Constants parseConstants(string filename) {
	Constants c;
    map_type map;
    
    map["DerivedA"] = &createInstance<DerivedA>;
    map["DerivedB"] = &createInstance<DerivedB>;

    JsonBox::Value v;
	v.loadFromFile(filename);
    if(v.isObject()) {
        const JsonBox::Object &o = v.getObject();
        for(auto it = o.begin(); it != o.end(); it++) {
            const string &const_field = it->first;
            if(const_field == "input_layers") {

            } else 
            if(const_field == "net_layers") {
                const JsonBox::Object &inner_o = it->second.getObject();
                for(auto inner_it = inner_o.begin(); inner_it != inner_o.end(); inner_it++) {
                    cout << inner_it->first << "\n";
                }
            } else 
            if(const_field == "synapses") {
                
            } else 
            if(const_field == "prob_funcs") {
                
            } else 
            if(const_field == "learning_rules") {
                
            } else 
            if(const_field == "sim_configuration") {
                
            } else 
            {
                cerr << "Unknown constants field " << const_field << "\n";
                exit(1);
            }
                  
        }
    } else {
        cerr << "Failed to find main object in constants json file " << filename << "\n";
    }
    return c;
}

