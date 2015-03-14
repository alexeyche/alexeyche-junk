#pragma once


namespace dnn {

class Builder {
public:
	Builder(const Constants &_c) : c(_c), input_stream(nullptr) {

	}

	vector<InterfacedPtr<SpikeNeuronBase>> buildNeurons() {
		vector<InterfacedPtr<SpikeNeuronBase>> neurons;
		for(auto &lc: c.sim_conf.layers) {
			for(size_t ni=0; ni<lc.size; ++ni) {
				InterfacedPtr<SpikeNeuronBase> n;
				if(input_stream) {
					n.set(input_stream->readObject<SpikeNeuronBase>());
				} else {
					n.set(buildObjectFromConstants<SpikeNeuronBase>(lc.neuron, c.neurons));	
				}
				
				neurons.push_back(n);	
			}
		}
		return neurons;
	}
	template <typename T>
	T* buildObjectFromConstants(const string &name, const map<string, string> &object_const_map) {
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