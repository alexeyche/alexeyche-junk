#pragma once


#include <dnn/contrib/rapidjson/document.h>
#include <dnn/contrib/rapidjson/stringbuffer.h>
#include <dnn/contrib/rapidjson/prettywriter.h>
#include <dnn/contrib/rapidjson/filestream.h>

#include <dnn/util/distributions.h>
#include <dnn/util/json.h>

namespace dnn {

using namespace rapidjson;

struct LayerConfiguration : public Printable {
	LayerConfiguration(size_t _size, string _neuron, string _act_function, uptr<Distribution<double>> _axon_delay_distr) :
		size(_size), neuron(_neuron), act_function(_act_function), axon_delay_distr(std::move(_axon_delay_distr)) {}
	
	LayerConfiguration(LayerConfiguration&& other) : 
		size(other.size), neuron(other.neuron), act_function(other.act_function), axon_delay_distr(std::move(other.axon_delay_distr)) {}

	size_t size;
	string neuron;
	string act_function;
	uptr<Distribution<double>> axon_delay_distr;

	void print(ostream &o) const {
		o << "size: " << size << ", neuron: " << neuron << ", act_function: " << act_function << ", axon_delay_distr: " << *(axon_delay_distr.get());
		//axon_delay_distr->print(o);
	}
private:
	LayerConfiguration(const LayerConfiguration& other) {}	
    LayerConfiguration& operator=(const LayerConfiguration& other) { return *this; }
};

struct ConnConfiguration : public Printable {
	ConnConfiguration(double _prob, string _synapse, uptr<Distribution<double>> _weight_distr, uptr<Distribution<double>> _dendrite_delay_distr) :
			prob(_prob), synapse(_synapse), weight_distr(std::move(_weight_distr)), dendrite_delay_distr(std::move(_dendrite_delay_distr)) {}
	
	ConnConfiguration(ConnConfiguration&& other) : 
		prob(other.prob), synapse(other.synapse), weight_distr(std::move(other.weight_distr)), dendrite_delay_distr(std::move(other.dendrite_delay_distr)) {}

	double prob;
	string synapse;
	uptr<Distribution<double>> weight_distr;
	uptr<Distribution<double>> dendrite_delay_distr;

	void print(ostream &o) const {
		o << "prob: " <<  prob << ", synapse: " << synapse <<  ", weight_distr: " << *(weight_distr.get()) << ", dendrite_delay_distr: " << *(dendrite_delay_distr.get());
	}
private:
    ConnConfiguration(const ConnConfiguration& other) {}
    ConnConfiguration& operator=(const ConnConfiguration& other) { return *this; }
};

struct SimConfiguration : public Printable {
	vector<LayerConfiguration> layers;
	map< pair<size_t, size_t>, ConnConfiguration> conn_map;
	double dt;
	int seed;
	vector<size_t> neurons_to_listen; 

	void print(ostream &o) const {
		o << "layers: \n";
		for(auto &v : layers) {
			o << "\t" <<  v << "\n";
		}
		o << "conn_map: \n";
		for(auto &v : conn_map) {
			o << "\t" <<  v.first.first << "->" << v.first.second << " " << v.second << "\n";
		}
		o << "dt: " << dt << "\n";
		o << "seed: " << seed << "\n";
		o << "neurons_to_listen: ";
		for(auto &v : neurons_to_listen) { o << v << ", "; }
		o << "\n";
	}
};


struct Constants : public Printable {
	Constants(const string& fname) {
		std::ifstream ifs(fname);
		std::string const_json((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
		
		Document document = Json::parseString(const_json);
		

		fill(Json::getVal(document, "neurons"), neurons);
		fill(Json::getVal(document, "act_functions"), act_functions);
		fill(Json::getVal(document, "synapses"), synapses);

		const Value &sim_conf_doc = Json::getVal(document, "sim_configuration");
		const Value &layers_doc = Json::getArray(sim_conf_doc, "layers");
		
		for (SizeType i = 0; i < layers_doc.Size(); i++) {
			const Value &v = layers_doc[i];
			LayerConfiguration lc(
				Json::getUintVal(v, "size"), 
				Json::getStringVal(v, "neuron"), 
				Json::getStringVal(v, "act_function"),
				parseDistribution<double>(Json::getStringValDef(v, "axon_delay_distr", "Exp(0,0)"))
			);
			sim_conf.layers.push_back(std::move(lc));
		}
		const Value &conn_map_doc = Json::getVal(sim_conf_doc, "conn_map");

		for (Value::ConstMemberIterator itr = conn_map_doc.MemberBegin(); itr != conn_map_doc.MemberEnd(); ++itr) {
			const string k = itr->name.GetString();
			vector<string> aff = splitBySubstr(k, "->");
            if(aff.size() != 2) {
                cerr << "conn_map configuration not right: need 2 afferents separated by \"->\"\n";
                terminate();
            }

            const pair<size_t, size_t> aff_p(stoi(aff[0]), stoi(aff[1]));
            const Value &conns = itr->value;
            if(!conns.IsArray()) { cerr << "conn_map must be array\n"; terminate(); }

            for (SizeType i = 0; i < conns.Size(); i++) {
            	const Value &v = conns[i];
            	ConnConfiguration conn_conf(
            		Json::getDoubleVal(v, "prob"), 
            		Json::getStringVal(v, "synapse"),	
            		parseDistribution<double>(Json::getStringVal(v, "weight_distr")),
            		parseDistribution<double>(Json::getStringValDef(v, "dendrite_delay_distr", "Exp(0,0)"))
            	);
            	sim_conf.conn_map.insert( pair<pair<size_t,size_t>, ConnConfiguration>(aff_p, std::move(conn_conf)) );
            }
		}
		sim_conf.dt = Json::getDoubleVal(sim_conf_doc, "dt");
		sim_conf.seed = Json::getIntVal(sim_conf_doc, "seed");
		sim_conf.neurons_to_listen = Json::getUintVector(sim_conf_doc, "neurons_to_listen");
	}
			

	static void fill(const Value &v, map<string, string> &m) {
		for (Value::ConstMemberIterator itr = v.MemberBegin(); itr != v.MemberEnd(); ++itr) {
			Document d;
		    Value cv(kObjectType);
		    Value copy_v;
		    copy_v.CopyFrom(itr->value, d.GetAllocator());
    		cv.AddMember(StringRef(itr->name.GetString()), copy_v, d.GetAllocator());
		    m[itr->name.GetString()] = Json::stringify(cv);
		}
	}
	
	void print(ostream &o) const {
		print_section("neurons: ", neurons, o);
		print_section("act_functions: ", act_functions, o);
		print_section("synapses: ", synapses, o);
		o << "sim_configuration: \n";
		o << sim_conf;
	}
	static void print_section(const string &sect_name, const map<string, string> &m, ostream &o) {
		o << sect_name << "\n";
		for(auto it = m.begin(); it != m.end(); ++it) {
			o << "\t" << it->first << ": ";
			for( auto &s: split(it->second, '\n') ) {
				o << "\t" <<  s << "\n";	
			}
		}
	}
	
	map<string, string> neurons;
	map<string, string> act_functions;
	map<string, string> synapses;
	SimConfiguration sim_conf;
};

}
