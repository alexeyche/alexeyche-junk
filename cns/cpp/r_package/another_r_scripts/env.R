

pe = path.expand
pj = function(l, r) {
    return(sprintf("%s/%s", l, r))
} 
    
HOME = pe("~/dnn")

SPIKES_PLACE = pj(HOME, "spikes")
TS_PLACE = pj(HOME, "ts")
DATASETS_PLACE = pj(HOME, "datasets")
CONST_JSON = pj(HOME, "const.json")