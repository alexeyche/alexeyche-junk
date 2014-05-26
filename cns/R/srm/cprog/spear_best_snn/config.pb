language: PYTHON
name:     "best_snn"

variable {
 name: "input_edge_prob"
 type: FLOAT
 size: 1
 min:  0.1
 max:  1
}

variable {
 name: "net_edge_prob"
 type: FLOAT
 size: 1
 min:  0.01
 max:  0.8
}

variable {
 name: "inhib_frac"
 type: FLOAT
 size: 1
 min:  0.0
 max:  0.8
}

variable {
 name: "tm"
 type: FLOAT
 size: 1
 min:  2.5
 max:  50
}

variable {
 name: "beta"
 type: FLOAT
 size: 1
 min:  0.1
 max:  10
}

variable {
 name: "u_tr"
 type: FLOAT
 size: 1
 min:  10
 max:  30
}

variable {
 name: "qa"
 type: FLOAT
 size: 1
 min:  0.01
 max:  10
}

variable {
 name: "ta"
 type: FLOAT
 size: 1
 min:  20
 max:  300
}

variable {
 name: "qr"
 type: FLOAT
 size: 1
 min:  20
 max:  200
}

variable {
 name: "tr"
 type: FLOAT
 size: 1
 min:  0.1
 max:  20
}


