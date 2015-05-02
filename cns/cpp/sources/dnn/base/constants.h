#pragma once


#include <dnn/contrib/rapidjson/document.h>
#include <dnn/contrib/rapidjson/stringbuffer.h>
#include <dnn/contrib/rapidjson/prettywriter.h>
#include <dnn/contrib/rapidjson/filestream.h>

#include <dnn/util/json.h>
#include <dnn/base/exceptions.h>

namespace dnn {

using namespace rapidjson;


struct SimConfiguration : public Printable {
	vector<string> layers;
	multimap< pair<size_t, size_t>, string> conn_map;
	double dt;
	int seed;
	vector<size_t> neurons_to_listen;
	map<string, string> files;
	
	void print(ostream &o) const {
		o << "layers: \n";
		for (auto &v : layers) {
			o << "\t" <<  v << "\n";
		}
		o << "conn_map: \n";
		for (auto &v : conn_map) {
			o << "\t" <<  v.first.first << "->" << v.first.second << " " << v.second << "\n";
		}
		o << "dt: " << dt << "\n";
		o << "seed: " << seed << "\n";
		o << "neurons_to_listen: ";
		for (auto &v : neurons_to_listen) { o << v << ", "; }
		o << "\n";
	}
};


struct Constants : public Printable {
	enum ReadMod {FromString, FromFile};
	Constants(const string& s, OptMods mods = OptMods(), ReadMod mod = FromFile) {
		string const_json;
		if(mod == FromFile) {
			std::ifstream ifs(s);
			const_json = std::string((std::istreambuf_iterator<char>(ifs)),
			                       std::istreambuf_iterator<char>());
		} else
		if(mod == FromString) {
			const_json = s;
		}
		
		for(auto it=mods.begin(); it != mods.end(); ++it) {
			replaceAll(const_json, it->first, it->second);
		}

		Document document = Json::parseString(const_json);		

		fill(Json::getVal(document, "neurons"), neurons);
		fill(Json::getVal(document, "act_functions"), act_functions);
		fill(Json::getVal(document, "synapses"), synapses);
		fill(Json::getVal(document, "inputs"), inputs);
		fill(Json::getVal(document, "learning_rules"), learning_rules);
		fill(Json::getVal(document, "connections"), connections);

		const Value &sim_conf_doc = Json::getVal(document, "sim_configuration");
		const Value &layers_doc = Json::getArray(sim_conf_doc, "layers");

		for (SizeType i = 0; i < layers_doc.Size(); i++) {
			const Value &v = layers_doc[i];
			sim_conf.layers.push_back(Json::stringify(v));
		}
		const Value &conn_map_doc = Json::getVal(sim_conf_doc, "conn_map");

		for (Value::ConstMemberIterator itr = conn_map_doc.MemberBegin(); itr != conn_map_doc.MemberEnd(); ++itr) {
			const string k = itr->name.GetString();
			vector<string> aff = splitBySubstr(k, "->");
			if (aff.size() != 2) {
				throw dnnException() << "conn_map configuration not right: need 2 afferents separated by \"->\"\n";
			}

			const pair<size_t, size_t> aff_p(stoi(aff[0]), stoi(aff[1]));
			const Value &conns = itr->value;

			for (SizeType i = 0; i < conns.Size(); i++) {
				const Value &v = conns[i];
				sim_conf.conn_map.insert( pair<pair<size_t, size_t>, string>(aff_p, Json::stringify(v) ));
			}
		}
		sim_conf.dt = Json::getDoubleVal(sim_conf_doc, "dt");
		sim_conf.seed = Json::getIntVal(sim_conf_doc, "seed");
		if(sim_conf.seed < 0) {
			std::srand ( unsigned ( std::time(0) ) );
		} else {
			std::srand ( sim_conf.seed );
		}
		sim_conf.neurons_to_listen = Json::getUintVector(sim_conf_doc, "neurons_to_listen");
		const Value &files_doc = Json::getVal(sim_conf_doc, "files");
		for (Value::ConstMemberIterator itr = files_doc.MemberBegin(); itr != files_doc.MemberEnd(); ++itr) {
			const string k = itr->name.GetString();
			sim_conf.files[k] = Json::stringify(itr->value);
		}
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
		print_section("inputs: ", inputs, o);
		print_section("neurons: ", neurons, o);
		print_section("act_functions: ", act_functions, o);
		print_section("synapses: ", synapses, o);
		print_section("learning_rules: ", learning_rules, o);
		print_section("connections: ", connections, o);
		o << "sim_configuration: \n";
		o << sim_conf;
	}
	static void print_section(const string &sect_name, const map<string, string> &m, ostream &o) {
		o << sect_name << "\n";
		for (auto it = m.begin(); it != m.end(); ++it) {
			o << "\t" << it->first << ": ";
			for ( auto &s : split(it->second, '\n') ) {
				o << "\t" <<  s << "\n";
			}
		}
	}


	map<string, string> neurons;
	map<string, string> act_functions;
	map<string, string> synapses;
	map<string, string> inputs;
	map<string, string> learning_rules;
	map<string, string> connections;
	SimConfiguration sim_conf;
};

}
