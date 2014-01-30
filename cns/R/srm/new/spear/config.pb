language: PYTHON
name:     "snnSRM"

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
 name: "lr"
 type: FLOAT
 size: 1
 min:  0.1
 max:  2
}

variable {
 name: "llh_depr"
 type: FLOAT
 size: 1
 min:  0.02666667
 max:  0.1
}

variable {
 name: "refr"
 type: ENUM
 size: 3
 options: "low"
 options: "middle"
 options: "big"
}
