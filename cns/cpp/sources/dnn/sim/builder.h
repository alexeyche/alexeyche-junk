#pragma once

#include <dnn/connections/connection.h>

namespace dnn {


class Builder {
public:
	struct Layer {
		vector<InterfacedPtr<SpikeNeuronBase>> neurons;
	};



	Builder(const Constants &_c) : c(_c), input_stream(nullptr) {
	}


	vector<InterfacedPtr<SpikeNeuronBase>> buildNeurons() {
		vector<Layer> layers;
		for (const string &lc : c.sim_conf.layers) {
			Layer layer;
			Document layer_conf = Json::parseStringC(lc);
			size_t layer_size = Json::getUintVal(layer_conf, "size");
			size_t col_size = ceil(sqrt(layer_size));
			size_t xi = 0;
			size_t yi = 0;
			for (size_t ni = 0; ni < layer_size; ++ni) {
				InterfacedPtr<SpikeNeuronBase> n;

				if (input_stream) {
					n.set(input_stream->readObject<SpikeNeuronBase>());
				} else {
					n.set(buildObjectFromConstants<SpikeNeuronBase>(Json::getStringVal(layer_conf, "neuron"), c.neurons));
					n.ref().mutAxonDelay() = Json::getDoubleValDef(layer_conf, "axon_delay", 0.0);

					const string act_function = Json::getStringValDef(layer_conf, "act_function", "");
					if (!act_function.empty()) {
						n.ref().setActFunction(buildObjectFromConstants<ActFunctionBase>(act_function, c.act_functions));
					}
					const string learning_rule = Json::getStringValDef(layer_conf, "learning_rule", "");
					if (!learning_rule.empty()) {
						n.ref().setLearningRule(buildObjectFromConstants<LearningRuleBase>(learning_rule, c.learning_rules));
					}
					const string input = Json::getStringValDef(layer_conf, "input", "");
					if (!input.empty()) {
						n.ref().setInput(buildObjectFromConstants<InputBase>(input, c.inputs));
					}
					n.ref().setCoordinates(xi, yi, col_size);
					xi++;
					if(xi % col_size == 0) {
						yi++;
						xi = 0;
					}
				}
				layer.neurons.push_back(n);
			}
			layers.push_back(layer);
		}

		for (auto it = c.sim_conf.files.begin(); it != c.sim_conf.files.end(); ++it) {
			const string &obj_name = it->first;			
			Document file_conf = Json::parseStringC(it->second);
			
			string fname = Json::getStringVal(file_conf, "filename");
			if(fname.find("@") == 0) {
				continue;
			}
			auto slice = Factory::inst().getObjectsSlice(obj_name);
			for(auto it=slice.first; it != slice.second; ++it) {
				Factory::inst().getObject(it)->setAsInput(
					Factory::inst().getCachedObject(
	            	    fname
	            	)
				);
			}
		}
		
		
		if (!input_stream) {
			for (auto it = c.sim_conf.conn_map.begin(); it != c.sim_conf.conn_map.end(); ++it) {
				size_t l_id_pre = it->first.first;
				size_t l_id_post = it->first.second;
				const string conn_conf = it->second;
				if (l_id_pre >= layers.size()) {
					throw dnnException()<< "Can't find layer with id " << l_id_pre << "\n";
				}
				if (l_id_post >= layers.size()) {
					throw dnnException()<< "Can't find layer with id " << l_id_post << "\n";
				}
				connectLayers(layers[l_id_pre], layers[l_id_post], conn_conf);
			}
		}

		vector<InterfacedPtr<SpikeNeuronBase>> neurons;
		for (auto &l : layers) {
			neurons.insert(neurons.end(), l.neurons.begin(), l.neurons.end());
		}
		turnOnStatistics(neurons, c.sim_conf.neurons_to_listen);
		return neurons;
	}

	static void turnOnStatistics(vector<InterfacedPtr<SpikeNeuronBase>> &neurons, const vector<size_t> &ids) {
		for(auto it=ids.cbegin(); it != ids.cend(); ++it) {
			if(*it >= neurons.size()) {
				throw dnnException() << "Can't find neuron " << *it << " to listen\n";
			}
			neurons[*it].ref().stat.turnOn();
			for(auto s: neurons[*it].ref().getSynapses()) {
				s.ref().stat.turnOn();
			}
			if(neurons[*it].ref().lrule.isSet()) {
				neurons[*it].ref().lrule.ref().stat.turnOn();
			}
		}
	}
	void connectLayers(Layer &pre, Layer &post, const string &conn_conf_s) {
		Document conn_conf = Json::parseStringC(conn_conf_s);
		ConnectionBase *conn = buildObjectFromConstants<ConnectionBase>(Json::getStringVal(conn_conf, "type"), c.connections);

		for (auto &npre : pre.neurons) {
			for (auto &npost : post.neurons) {
				if(npre.ref().id() == npost.ref().id()) {
					continue;
				}
				ConnectionRecipe connection_recipe = conn->getConnectionRecipe(npre.ref(), npost.ref());
				if (connection_recipe.exists) {
					SynapseBase *syn(nullptr);					
					if(!connection_recipe.inhibitory) {
					    syn = buildObjectFromConstants<SynapseBase>(
					    	Json::getStringVal(conn_conf, "synapse"), 
					    	c.synapses
					    );
					} else {
						string inh_synapse_type = Json::getStringValDef(conn_conf, "inh_synapse", "");
						if(inh_synapse_type.empty()) {
							throw dnnException() << "Connection " << conn->name() << " demands inhibitory synapse type to be pointed in constants\n";
						}
					    syn = buildObjectFromConstants<SynapseBase>(
					    	inh_synapse_type,
					    	c.synapses
					    );
					}
					assert(syn);
					
					syn->mutIdPre() = npre.ref().id();
					syn->mutDendriteDelay() = Json::getDoubleValDef(conn_conf, "dendrite_delay", 0.0);
					syn->mutWeight() = connection_recipe.amplitude * Json::getDoubleVal(conn_conf, "start_weight");
					npost.ref().addSynapse(InterfacedPtr<SynapseBase>(syn));
				}
			}
		}
	}

	template <typename T>
	static T* buildObjectFromConstants(const string &name, const map<string, string> &object_const_map) {
		auto cptr = object_const_map.find(name);
		if ( cptr == object_const_map.end() ) {
			throw dnnException() << "Trying to build " << name << " from constants and can't find him\n";
		}

		istringstream *ss = new istringstream(cptr->second);
		Stream s(*ss, Stream::Text);
		T* n = s.readObject<T>();
		delete ss;
		if(!n) {
			throw dnnException() << "Null object from constants: " << name << "\n";
		}
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