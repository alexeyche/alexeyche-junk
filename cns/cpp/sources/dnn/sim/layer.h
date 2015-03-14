#pragma once

struct Layer {
	void connect(Layer &post, string conn_conf, const map<string, string>)
	void addNeuron()

	vector<InterfacedPtr<SpikeNeuronBase>> neurons;
};