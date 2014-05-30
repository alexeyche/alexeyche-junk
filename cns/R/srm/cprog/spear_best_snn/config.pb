language: PYTHON
name:     "best_snn"

variable {
 name: "weight_decay_factor"
 type: FLOAT
 size: 1
 min:  0.0
 max:  0.03
}

variable {
 name: "weight_per_neuron"
 type: FLOAT
 size: 1
 min:  50.0
 max:  200.0
}

variable {
 name: "net_edge_prob"
 type: FLOAT
 size: 1
 min:  0.0
 max:  0.75
}

variable {
 name: "inhib_frac"
 type: FLOAT
 size: 1
 min:  0.0
 max:  0.75
}
