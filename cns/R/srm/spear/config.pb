language: PYTHON
name:     "snn"

variable {
 name: "alpha"
 type: FLOAT
 size: 1
 min:  0.6
 max:  1.2
}

variable {
 name: "beta"
 type: FLOAT
 size: 1
 min:  0.6
 max:  1.2
}


variable {
 name: "llh_depr_mode"
 type: ENUM
 size: 1
 options: 'no'
 options: 'low'
 options: 'middle'
 options: 'high'
}


variable {
 name: "duration"
 type: FLOAT
 size: 1
 min:  100
 max:  500
}


variable {
 name: "edge_prob"
 type: FLOAT
 size: 1
 min:  0.05
 max:  0.7
}

variable {
 name: "refr_mode"
 type: ENUM
 size: 1
 options: "low"
 options: "middle"
 options: "high"
}

variable {
 name: "net_neurons_for_input"
 type: INT
 size: 1
 min: 5
 max: 30
}

variable {
 name: "afferent_per_neuron"
 type: INT
 size: 1
 min: 5
 max: 30
}

variable {
 name: "weights_norm_type"
 type: ENUM
 size: 1
 options: "mult_glob"
 options: "mult_local"
 options: "add"
 options: "no"
}

