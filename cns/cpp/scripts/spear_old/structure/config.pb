language: PYTHON
name:     "structure"

variable {    
    name: "tau_adapt"
    type: FLOAT
    size: 1
    min:  0
    max: 500 
}
variable {    
    name: "amp_adapt"
    type: FLOAT
    size: 1
    min:  0
    max: 10 
}
variable {    
    name: "tau_refr"
    type: FLOAT
    size: 1
    min: 0 
    max: 10 
}
variable {    
    name: "beta"
    type: FLOAT
    size: 1
    min:  0.1
    max:  4.0
}
variable {    
    name: "epsp_decay_exc"
    type: FLOAT
    size: 1
    min: 1 
    max: 50 
}
variable {    
    name: "epsp_decay_inh"
    type: FLOAT
    size: 1
    min: 1 
    max: 50 
}
variable {    
    name: "prob_feedforward_exc"
    type: FLOAT
    size: 1
    min:  0.1
    max:  1.0
}
variable {    
    name: "prob_feedforward_inh"
    type: FLOAT
    size: 1
    min:  0.0
    max:  1.0
}
variable {    
    name: "prob_reccurent_inh"
    type: FLOAT
    size: 1
    min:  0.0
    max:  1.0
}
variable {    
    name: "prob_reccurent_exc"
    type: FLOAT
    size: 1
    min:  0.0
    max:  1.0
}
variable {    
    name: "weight_distr_mean_ff_exc"
    type: FLOAT
    size: 1
    min:  1
    max: 80 
}
variable {    
    name: "weight_distr_mean_ff_inh"
    type: FLOAT
    size: 1
    min:  1
    max: 80 
}
variable {    
    name: "weight_distr_mean_rec_exc"
    type: FLOAT
    size: 1
    min:  1
    max: 80 
}
variable {    
    name: "weight_distr_mean_rec_inh"
    type: FLOAT
    size: 1
    min:  1
    max: 80 
}
