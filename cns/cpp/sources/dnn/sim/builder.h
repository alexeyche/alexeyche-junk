#pragma once


namespace dnn {


class Builder {
public:
	struct Layer {
		vector<InterfacedPtr<SpikeNeuronBase>> neurons;
	};

	

	Builder(const Constants &_c) : c(_c), input_stream(nullptr) {

	}
	
	void registerFile(const string &fname) {

	}

	vector<InterfacedPtr<SpikeNeuronBase>> buildNeurons() {
		vector<Layer> layers;
		for(const string &lc: c.sim_conf.layers) {
			Layer layer;
			Document layer_conf = Json::parseStringC(lc);
			for(size_t ni=0; ni<Json::getUintVal(layer_conf, "size"); ++ni) {
				InterfacedPtr<SpikeNeuronBase> n;
					
				if(input_stream) {
					n.set(input_stream->readObject<SpikeNeuronBase>());
				} else {
					n.set(buildObjectFromConstants<SpikeNeuronBase>(Json::getStringVal(layer_conf, "neuron"), c.neurons));					
					n.ref().setAxonDelay( Json::getDoubleValDef(layer_conf, "axon_delay", 0.0) );

					const string act_function = Json::getStringValDef(layer_conf, "act_function", "");
					if(!act_function.empty()) {
						n.ref().setActFunction(buildObjectFromConstants<ActFunctionBase>(act_function, c.act_functions));
					}
					const string input = Json::getStringValDef(layer_conf, "input", "");
					if(!input.empty()) {
						n.ref().setInput(buildObjectFromConstants<InputBase>(input, c.inputs));
						if(c.sim_conf.files.end() == c.sim_conf.files.find(input)) {
							cerr << "Can't find external file for " << input << " in config\n";
							terminate();
						}
						Input* res = getExternalSource()
						n.ref().setExternalSource();
					}
				}
				layer.neurons.push_back(n);
			}
			layers.push_back(layer);
		}
		if(!input_stream) {
			for(auto it = c.sim_conf.conn_map.begin(); it != c.sim_conf.conn_map.end(); ++it) {
				size_t l_id_pre = it->first.first;
				size_t l_id_post = it->first.second;
				const string conn_conf = it->second;
				if(l_id_pre>=layers.size()) { 
					cerr << "Can't find layer with id " << l_id_pre << "\n";
					terminate();
 				}
 				if(l_id_post>=layers.size()) { 
 					cerr << "Can't find layer with id " << l_id_post << "\n";
					terminate();
				}
				connectLayers(layers[l_id_pre], layers[l_id_post], conn_conf);
			}
		}

		vector<InterfacedPtr<SpikeNeuronBase>> neurons;
		for(auto &l : layers) {
			neurons.insert(neurons.end(), l.neurons.begin(), l.neurons.end());
		}
		return neurons;
	}
	void connectLayers(Layer &pre, Layer &post, const string &conn_conf_s) {
		Document conn_conf = Json::parseStringC(conn_conf_s);
		for(auto &npre: pre.neurons) {
			for(auto &npost: post.neurons) {
				if(getUnif() < Json::getDoubleValDef(conn_conf, "prob", 1.0)) {
					InterfacedPtr<SynapseBase> syn(
						buildObjectFromConstants<SynapseBase>(Json::getStringVal(conn_conf, "synapse"), c.synapses)
					);
					syn.ref().setIdPre(npre.ref().id());
					syn.ref().setDendriteDelay(Json::getDoubleValDef(conn_conf, "dendrite_delay", 0.0));
					syn.ref().setWeight(Json::getDoubleVal(conn_conf, "start_weight"));
					
					npost.ref().addSynapse(syn);
				}
			}
		}
	}

	template <typename T>
	static T* buildObjectFromConstants(const string &name, const map<string, string> &object_const_map) {
		auto cptr = object_const_map.find(name);
		if( cptr == object_const_map.end() ) {
			cerr << "Trying to build " << name << " from constants and can't find them\n";
			terminate();
		}

		istringstream *ss = new istringstream(cptr->second);
		Stream s(*ss, Stream::Text);
		T* n = s.readObject<T>();
		delete ss;
		return n;
	} 

	void setInputModelStream(Stream *s) {
		input_stream = s;
	}

private:
	Stream *input_stream;
	const Constants &c;

};

}