language: PYTHON
name:     "best_code"

variable {
 name: "sigma"
 type: FLOAT
 size: 1
 min:  0.01
 max:  5
}

variable {
 name: "C"
 type: FLOAT
 size: 1
 min:  50 
 max:  400
}

variable {
 name: "gL"
 type: FLOAT
 size: 1
 min:  1
 max:  100
}

variable {
 name: "Vtr"
 type: FLOAT
 size: 1
 min:  -51
 max:  -25
}

variable {
 name: "slope"
 type: FLOAT
 size: 1
 min:  0.01
 max:  10
}

variable {
 name: "tau_w"
 type: FLOAT
 size: 1
 min:  0
 max:  1000
}

variable {
 name: "a"
 type: FLOAT
 size: 1
 min:  0
 max:  100
}

variable {
 name: "b"
 type: FLOAT
 size: 1
 min:  0.001
 max:  500
}
