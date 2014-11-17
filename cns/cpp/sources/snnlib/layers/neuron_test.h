
typedef FastDelegate0<> StateModifier;
typedef FastDelegate1<const &double> InputModifier;

class SimNeuron: {
public:
	void SimNeuron(Neuron *n) {
		n->provideInputModifiers(input_mods);
		n->provideStateModifiers(state_mods);
	}
	void calculate() {
		for(size_t i=0; i<input_mods.size(); i++) {
			input_mods[i](I);
		}
		for(size_t i=0; i<mods.size(); i++) {
			mods[i]();
		}
	}
	
	double I;
	vector<vector<InputModifier>> input_mods;
	vector<vector<StateModifier>> state_mods;	
};    